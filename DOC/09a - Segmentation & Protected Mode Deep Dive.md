# 09a — Segmentation & Protected Mode Deep Dive

## What is Memory Segmentation?
Before modern 64-bit processors, CPUs had a much harder time addressing large amounts of memory. In the 16-bit 8086 era, registers were only 16-bits wide (max `0xFFFF` or 64 KB). To address more memory, Intel introduced **Segmentation**. 

Segmentation involves dynamically adding a "Base Offset" to your pointers. In the x86 architecture, memory is accessed using a pair: `Segment:Offset`. 

When your C code asks for pointer `0xB8000`, the CPU does the math:
`Physical Linear Address = Segment_Base_Address + 0xB8000`

### Real Mode vs Protected Mode
When a PC first turns on, it starts in **Real Mode** (16-bit compatibility mode). In Real Mode, segmentation works by simply multiplying the segment register by 16 and adding the offset. It provides absolutely no security. A user program could easily overwrite the kernel or hardware memory.

To get security and 32-bit features, we switch to **Protected Mode**. In Protected Mode, the segment registers no longer hold raw memory addresses. Instead, they hold an "Index" (Selector) pointing to an entry in a lookup table: The **Global Descriptor Table (GDT)**.

## Understanding the GDT

The GDT acts as the rulebook for Memory Segmentation in Protected Mode. It's important to understand that **RAM itself is just "dumb storage"**—it knows nothing about segments or security. The segmentation logic happens entirely inside the **CPU**.

To know which rules to apply, the CPU has dedicated **Segment Registers** (`CS` for Code, `DS` for Data, `SS` for Stack, etc.). In Protected Mode, these registers hold an "Index" (called a Segment Selector) pointing to a specific entry in the GDT. 

The GDT itself is stored in RAM. Every time a memory access occurs, the CPU hardware automatically consults its segment registers, looks up the corresponding GDT entry, and does the math before actually talking to the RAM.

```
  Memory Access Request `mov eax, [0xB8000]`
           │
           ▼
  Which Segment? (Uses Data Segment Register `DS` by default)
           │
           ▼
  CPU looks up the DS index in the GDT (which lives in RAM)
           │
           ▼
  Checks GDT Entry bounds:
   - Does base + offset exceed the Limit? (Segmentation Fault!)
   - Does current Ring level have Privilege? (Protection Fault!)
   - Is it Writable? (Read-Only Fault!)
           │
           ▼
  Access Granted! CPU translates to a Linear Address and fetches from RAM.
```

### The Flat Memory Model Hack
While segmentation allows setting up distinct isolated memory locations for Kernel Code, User Code, and Hardware, it proved clumsy to maintain compared to **Paging** (which divides memory into fixed 4KB chunks). 

However, since x86 natively requires the GDT to be enabled before entering Protected Mode, modern OS developers discovered a loophole: **The Basic Flat Model**. 

If we create our GDT entries such that:
- Every Segment `Base = 0x00000000`
- Every Segment `Limit = 0xFFFFFFFF` (4 GB)

Then `Physical Address = 0x0 + Offset`. The segment offset math perfectly cancels out, making the GDT invisible to the compiler. This turns off segmentation behavior, letting us use standard non-segmented C pointers, while still keeping the CPU happy.

### Ring 0 and Paging (Security Without Segmentation)
If segmentation is turned off by the Flat Model, how do we protect the kernel?
The GDT still enforces **Privilege Rings**. 
- Ring 0 (Kernel) has full permission.
- Ring 3 (User) has restricted permissions.

We create duplicate GDT entries for the same `0x0` Flat Address space—one pair for Ring 0, one pair for Ring 3. Even though they point to the exact same physical addresses, the CPU remembers which Ring a process is running in. Later down the line, we will use **Paging** to map exactly which 4KB chunks of memory Ring 3 is actually allowed to see.

## How Memory Actually Works in a System

To truly grasp what the CPU and GDT are doing, we must understand the fundamental relationship between the CPU, the RAM, and the software.

### 1. RAM is Just a Grid
At the lowest hardware level, RAM (Random Access Memory) is an astronomical array of microscopic capacitors and transistors. It only understands two operations: "Store this byte at index X" and "Give me the byte at index X".

RAM itself has no concept of "variables", "functions", "stack", or "security". It is entirely passive and "dumb". The CPU communicates with the RAM using physical motherboard wires called the **Address Bus** and **Data Bus**. If the CPU sends `0xB8000` down the address bus, the RAM blindly returns whatever byte is stored at that physical transistor location.

### 2. The Logical Layout of Memory
Because RAM is unstructured, the Operating System and Compiler must enforce a logical layout. When a program (or our kernel) is loaded into RAM, memory is conceptually partitioned into distinct regions:

- **.text (Code):** Where the actual executable CPU instructions (machine code) live. This is usually marked as Read-Only.
- **.data & .bss:** Where global and static variables live. `.data` contains initialized variables (e.g., `int x = 5;`), and `.bss` contains zero-initialized variables.
- **The Heap:** A large pool of memory used for dynamic allocation at runtime (`malloc`/`free`). It typically grows *upwards* (towards higher addresses).
- **The Stack:** Temporary memory used for function calls and local variables. It typically grows *downwards* (towards lower addresses).

### 3. Deep Dive: The Stack
The Stack is arguably the most critical region of memory for executing C code. It operates on a **LIFO (Last-In, First-Out)** principle. 

On x86 architecture, the CPU has dedicated registers to interact with the stack:
- **`ESP` (Extended Stack Pointer):** Always stores the raw memory address of the "top" of the stack (the most recently added item).
- **`EBP` (Extended Base Pointer):** Points to the base of the current function's "Stack Frame", acting as an anchor to easily find local variables and passed arguments.

**The x86 Downward Growth Quirk:** 
On x86, the stack grows **downwards** in memory. 
If you allocate a 16 KB stack from physical address `0x100000` up to `0x104000`:
- The initial `ESP` starts at the very highest address: `0x104000`.
- When you execute a `push eax` instruction, the CPU automatically *decrements* the `ESP` by 4 bytes (to `0x103FFC`), and writes the contents of `eax` into that new RAM address.
- When you execute `pop ebx`, the CPU reads the data at `ESP`, puts it into `ebx`, and then *increments* the `ESP` by 4 bytes.

**What is the Stack actually used for?**
1. **Control Flow (Function Calls):** When you `call` a function, the CPU automatically `push`es the *Return Address* (the address of the *next* instruction) onto the stack. When the function finishes, the `ret` instruction `pop`s that address off the stack and jumps back to it.
2. **Local Variables:** Variables declared inside a C function don't exist permanently. The compiler just subtracts an amount from the `ESP` to make temporary room for them in RAM. When the function returns, the compiler adds back to `ESP`, instantly "deleting" or abandoning them.
3. **Passing Arguments:** Before calling a function, arguments are often `push`ed onto the stack so the target function can read them.

### Tying It All Together
This is where the GDT, CPU, RAM, and Stack all intersect. 

Because RAM is just a passive, unprotected grid, what stops a deeply recursive C function from continuously `push`ing to the stack until it grows completely backward and accidentally overwrites the `.text` code segment? 

Nothing, physically. But *logically*, the CPU's MMU (Memory Management Unit) acts as the bouncer. It intercepts every single memory operation (`push`, `mov`, `call`, `pop`). Prior to fetching from RAM, it checks the GDT. 

By utilizing the 4GB Flat Memory Model, we are effectively bypassing the old GDT segmentation bounds checking (which was clunky). Instead, modern operating systems rely on **Paging** to map these specific segments (like the Stack and Heap) into physical RAM and raise "Page Faults" to catch stack overflows before they corrupt the kernel.
