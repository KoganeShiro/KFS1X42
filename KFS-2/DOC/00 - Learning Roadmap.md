# KFS-1 Learning Roadmap
> From Transistors to Kernel — A bottom-up approach

This roadmap is designed to build your understanding progressively. Each level builds upon the previous one.

> **Deep-dive explanations** of core concepts live in [00a - Core Concepts Deep Dive.md](00a%20-%20Core%20Concepts%20Deep%20Dive.md). Answers below are the TL;DR.

---

## 🔴 Level 0: The Physical Foundation
*"How does electricity become computation?"*

### Concepts:
1. **[[Transistor]]** — The fundamental building block (electronic switch)
2. **[[Logic Gates]]** — AND, OR, NOT, XOR — how transistors combine to make decisions
3. **Binary Arithmetic** — How numbers and data are represented (using only 0s and 1s)
4. **Clock Cycles** — The heartbeat of a computer (synchronization)

### Key Questions to Answer:

<details>
<summary>How does a transistor act as a switch?</summary>

Voltage on the gate → current flows (1). No voltage → no current (0). Billions of switching on/off = computation.
- [ Transistors Explained - How transistors work](https://youtu.be/J4oO7PT_nzQ?si=2Nx1Xf1lAIon_ECW)
- [ HOW TRANSISTORS RUN CODE? ](https://youtu.be/HjneAhCy2N4?si=VWlJtFxck5anitL2)
</details>

<details>
<summary>How do logic gates combine to perform addition?</summary>

XOR gate = sum, AND gate = carry. Chain them into a **full adder** (handles carry-in). Cascade N full adders = N-bit addition.
- [ Exploring How Computers Work ](https://www.youtube.com/watch?v=QZwneRb-zqA&list=PLD0P-oVoDCNbYKzfUj8rlZrWhfKmYjtqQ)
- [ Boolean Logic & Logic Gates: Crash Course Computer Science #3 ](https://youtu.be/gI-qXk7XojA?si=mKaqq72Xsfum1RNZ)

If you want to go deeper on how it work
- [ Introduction to Logic Gates - NOT, AND, OR, NAND, NOR, XOR, XNOR ](https://youtu.be/JQBRzsPhw2w?si=r6vPmMsltOHTEgiQ)
- [ Logic Gates, Truth Tables, Boolean Algebra AND, OR, NOT, NAND & NOR ](https://youtu.be/JQBRzsPhw2w?si=8d964UTYigdFWDyJ)
</details>

<details>
<summary>What is a clock signal and why is it necessary?</summary>

Periodic square wave from a crystal oscillator. Synchronizes all CPU operations — one step (fetch/decode/execute) per rising edge. 3 GHz = 3 billion ticks/sec.
- [ How the Clock Tells the CPU to "Move Forward" ](https://youtu.be/PVNAPWUxZ0g?si=lF3DM-VXa8aMlP3h)
- [ How do Digital and Analog Clocks Work? ](https://youtu.be/oEC5fIw0bL0?si=zfm5gDmXUAoqOI5U)
- [ The Fetch-Execute Cycle: What's Your Computer Actually Doing? ](https://youtu.be/Z5JC9Ve1sfI?si=wojWDD9a4QKhWyVM)
</details>

<details>
<summary>What is the difference between combinational and sequential logic?</summary>

**Combinational** = output depends only on inputs (gates). **Sequential** = has state/memory (flip-flops, registers). ALU is combinational; registers are sequential.
- [ Comparison between Combinational and Sequential Circuits ](https://youtu.be/SzV4l0_1MCQ?si=U5RxA-dGFKRsDqKF)
- [ Ep 026: Introduction to Combinational Logic ](https://youtu.be/lTmAlB1T6Yo?si=aNtQylEyxBjrfC9s)
- [Sequential Logic Circuits - Electronics tutorial (article)](https://www.electronics-tutorials.ws/sequential/seq_1.html)
</details>

<details>
<summary>How are negative numbers represented in binary?</summary>

**Two's complement**: invert all bits + add 1. MSB (most significant bit) = sign bit. Addition works the same for signed/unsigned — no special hardware needed.

- [Bit_numbering](https://en-wikipedia-org.translate.goog/wiki/Bit_numbering?_x_tr_sl=en&_x_tr_tl=fr&_x_tr_hl=fr&_x_tr_pto=rq)

</details>

---

## 🟠 Level 1: CPU Architecture
*"How does the processor execute instructions?"*

### Concepts:
1. **[[CPU register]]** — Small, fast storage inside the CPU
2. **[[X86]] / [[I386]]** — The instruction set architecture you'll target
3. **Instruction Cycle** — Fetch → Decode → Execute
4. **Addressing Modes** — How the CPU accesses memory and data
5. **Stack & Heap** — Memory organization for programs
6. **Pipelining** — How CPUs optimize instruction execution
7. **Caching** — L1, L2, L3 cache hierarchy

### Key Questions to Answer:

> 📖 Deep dive: [CPU Architecture & Registers](00a%20-%20Core%20Concepts%20Deep%20Dive.md#1-cpu-architecture--registers) — [The Stack](00a%20-%20Core%20Concepts%20Deep%20Dive.md#2-the-stack)

<details>
<summary>What are the main x86 registers and their purposes?</summary>

- **EAX** (accumulator/return values)
- **EBX** (base/GP)
- **ECX** (counter/loops)
- **EDX** (data/I/O)
- **ESI/EDI** (source/dest index), **ESP** (stack top)
- **EBP** (stack frame base)
- **EIP** (next instruction — not directly writable)
- **EFLAGS** (status: CF, ZF, SF, OF, IF). Segment registers: CS, DS, SS, ES, FS, GS.
- https://wiki.osdev.org/CPU_Registers_x86
- https://medium.com/@tushrsa/x86-64-registers-naming-explained-1cd958628090
</details>

<details>
<summary>How does the CPU know what instruction to execute next?</summary>

**EIP** holds the next instruction address. Auto-incremented after fetch. Changed explicitly by `JMP`, `CALL`, `RET`, and interrupts. Not writable via `MOV`.
- https://math.hws.edu/javanotes/c1/s1.html
- [ How do CPUs know what instruction to compute next ](https://youtu.be/BahvBKcHkuQ?si=Wcf7M700CHNRfBlZ)
</details>

<details>
<summary>What is the difference between real mode and protected mode?</summary>

Real mode: 16-bit, 1 MB, no protection. Protected mode: 32-bit, 4 GB, ring-based privilege (0-3), GDT-based segments. Switch: set PE bit in CR0 + far jump.
In the boot context: the CPU starts in **real mode** (16-bit, 1 MB address space, no protection). The bootloader must:
1. Set up a **GDT** with at least code and data segments
2. Set the **PE bit** (bit 0) in the **CR0** register: `mov cr0, eax` (with bit 0 set)
3. Perform a **far jump** to a 32-bit code segment to flush the prefetch queue

After this, the CPU operates in 32-bit protected mode with full 4 GB addressing.
- https://stackoverflow.com/questions/19619425/what-is-the-true-difference-between-a-real-mode-program-and-a-protected-mode-pro
- [ Making an OS (x86) Chapter 6 - Entering Protected mode, GDT ](https://www.youtube.com/watch?v=Wh5nPn2U_1w&list=PLm3B56ql_akNcvH8vvJRYOc7TbYhRs19M&index=6)
- [ Protected mode: how your OS can protect itself ](https://youtu.be/EBdzWFyKZ0U?si=cxKKsacyRdQ2im0L)

</details>

<details>
<summary>What is the role of ESP and EBP?</summary>

- **ESP** = top of stack (moves on push/pop/call/ret)
- **EBP** = stable anchor for the current stack frame. Args at `[ebp+8]`, locals at `[ebp-4]`. Set by function prologue: `push ebp; mov ebp, esp`.

- [ The Stack and ESP in Assembly Language - What happens when you call a procedure? ](https://youtu.be/RU5vUIl1vRs?si=e_7vWL-TMrQefeS3)
- [ 08 - The Stack ](https://www.youtube.com/watch?v=gWTuOENv6AA&list=PLHJns8WZXCdvaD7-xR7e5FJNW_6H9w-wC&index=8)
- [ wtf is “the stack” ? ](https://youtu.be/CRTR5ljBjPM?si=M6Fmd9CqTBQe9kXh)

</details>

<details>
<summary>What happens during a CPU pipeline stall?</summary>

Data dependency between in-flight instructions → pipeline must wait (bubble). Mitigated by forwarding and branch prediction. Mispredicted branch = full pipeline flush.

- [ CPU Pipeline - Computerphile ](https://www.youtube.com/watch?v=BVNx3wtJ9vs&list=PLzH6n4zXuckpwdGMHgRH5N9xNHzVGCxwf&index=4)
- [ Ep 085: Introduction to the CPU Pipeline ](https://www.youtube.com/watch?v=E5qacBU1XjQ&list=PLxfrSxK7P38X7XfG4X8Y9cdOURvC7ObMF&index=85)
- [Pipeline stall - Wikipedia](https://en.wikipedia.org/wiki/Pipeline_stall)
</details>

<details>
<summary>How does the CPU cache hierarchy work?</summary>

L1 (~4 cycles, 32-64 KB per core) → L2 (~10 cycles) → L3 (~30-40 cycles, shared) → RAM (~100-200 cycles). Data loaded in 64-byte cache lines. Exploits spatial & temporal locality.

- [ How CPU Memory & Caches Work - Computerphile ](https://youtu.be/SAk-6gVkio0?si=EZTwWucPfRuIETYK)
- [ How Cache Works Inside a CPU ](https://www.youtube.com/watch?v=zF4VMombo7U&list=PL38NNHQLqJqYnNrTenxBvGJSPCkV9EOWk)
</details>

---

## 🟡 Level 2: Assembly Language
*"How do we speak the CPU's language?"*

### Concepts:
1. **[[Assembly language]]** — Low-level programming (NASM/GAS syntax)
2. **x86 Instructions** — MOV, ADD, SUB, JMP, CALL, RET, INT, etc.
3. **Memory Segmentation** — CS, DS, SS, ES segments (real mode)
4. **Interrupts** — Software and hardware interrupts (INT instruction)
5. **Calling Conventions** — How functions pass arguments (cdecl, stdcall)

### Key Questions to Answer:

<details>
<summary>How do you write a basic assembly program?</summary>

A minimal NASM program that exits with code 42:

```asm
section .text
global _start

_start:
    mov eax, 1      ; syscall number: sys_exit
    mov ebx, 42     ; exit code
    int 0x80        ; invoke kernel
```

Key structure: `.text` for code, `.data` for initialized data, `.bss` for uninitialized data. `global _start` exports the entry point for the linker. Assemble and link with:
```
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```
</details>

<details>
<summary>What is the difference between MOV and LEA?</summary>

**MOV** copies a *value* — it reads from the source location.
**LEA** (Load Effective Address) computes an *address* — it calculates but does **not** access memory.

```asm
mov eax, [ebx + 4*ecx]   ; reads the VALUE at address (ebx + 4*ecx)
lea eax, [ebx + 4*ecx]   ; stores the ADDRESS (ebx + 4*ecx) into eax
```

LEA is often used as a fast way to do arithmetic: `lea eax, [eax + eax*2]` computes `eax * 3` in one instruction without touching memory.
</details>

- [ you can learn assembly in 10 minutes (try it RIGHT NOW) ](https://youtu.be/6S5KRJv-7RU?si=MNt9f-fbWPKANKBq)
- [ 4. Assembly Language & Computer Architecture ](https://www.youtube.com/watch?v=L1ung0wil9Y&list=PLUl4u3cNGP63VIBQVWguXxZZi0566y7Wf&index=4)
<details>
<summary>How do interrupts work and why are they important?</summary>

Interrupts are signals that tell the CPU to stop what it's doing and run a specific handler:

1. **Hardware interrupts (IRQs)** — triggered by devices (keyboard, timer, disk). The PIC or APIC routes them to the CPU.
2. **Software interrupts** — triggered by `INT n` instructions (e.g., `INT 0x80` for Linux syscalls).
3. **Exceptions** — triggered by the CPU itself on errors (divide by zero = INT 0, page fault = INT 14).

When an interrupt fires, the CPU:
1. Pushes **EFLAGS**, **CS**, and **EIP** onto the stack
2. Looks up entry `n` in the **IDT** (Interrupt Descriptor Table)
3. Jumps to the handler address stored there
4. Handler runs, then executes **IRET** to restore EFLAGS/CS/EIP and resume

Without interrupts, the CPU would have to **poll** every device in a loop — wasting cycles and missing events.

- [ CPU Interrupts - Computerphile ](https://www.youtube.com/watch?v=tGMSARJk7cA&list=PLzH6n4zXuckpwdGMHgRH5N9xNHzVGCxwf&index=11)
- [ Ep 088: Introduction to Interrupts ](https://www.youtube.com/watch?v=dDA3PUr16As&list=PLxfrSxK7P38X7XfG4X8Y9cdOURvC7ObMF&index=88)
- [ How Interrupts Work in Modern Computers ](https://youtu.be/G7bqvpAw7HE?si=oK7M1_Gj7C0H-JRF)
- [ Microcontroller Interrupts | Embedded System Project Series #17 ](https://www.youtube.com/watch?v=S-daLDfkoyY&list=PLS_iNJJVTtiRV0DZRDcTHnvAuDrKGPN40&index=17)
</details>

<details>
<summary>How does the stack work during function calls?</summary>

```
Caller: call my_func          Callee: my_func
┌─────────────────────┐       ┌──────────────────────────┐
│ push arg2           │       │ push ebp                 │
│ push arg1           │       │ mov  ebp, esp            │
│ call my_func        │──────►│ sub  esp, N  (locals)    │
│   (pushes ret addr) │       │ ... function body ...    │
│                     │       │ mov  esp, ebp            │
│ add esp, 8 (cleanup)│◄──────│ pop  ebp                 │
└─────────────────────┘       │ ret  (pops ret addr→EIP) │
                              └──────────────────────────┘

Stack during my_func:
    ┌──────────────┐  ← high addresses
    │   arg2       │  [ebp + 12]
    │   arg1       │  [ebp + 8]
    │   ret addr   │  [ebp + 4]  (pushed by CALL)
    │   saved ebp  │  [ebp]      (pushed by prologue)
    │   local var1 │  [ebp - 4]
    │   local var2 │  [ebp - 8]
    └──────────────┘  ← ESP (low addresses)
```

This is the **cdecl** calling convention: caller pushes args right-to-left, caller cleans up the stack after return.

- [ How Assembly Functions Work - The Stack Explained ](https://youtu.be/u_-oQx_4jvo?si=3dU77nHfOmYcdSY7)
- [ Buffer Overflow 101: Ep 1 - x86 Memory Fundamentals ](https://www.youtube.com/watch?v=_D8eLCmlrS8&list=PLdVIvW2RPTRxNdJeBZRcdt1JQJlmQlQMU)
</details>

<details>
<summary>What is the difference between NASM and GAS (AT&T) syntax?</summary>

| Feature | NASM (Intel) | GAS (AT&T) |
|---------|-------------|------------|
| **Operand order** | `dest, src` | `src, dest` |
| **Register prefix** | None: `eax` | `%` prefix: `%eax` |
| **Immediate prefix** | None: `42` | `$` prefix: `$42` |
| **Memory access** | `[eax + 4]` | `4(%eax)` |
| **Size suffix** | Implicit / `dword` | `movl`, `movw`, `movb` |

Example — same instruction:
```
NASM:  mov  eax, [ebx + 8]
GAS:   movl 8(%ebx), %eax
```

KFS-1 uses GAS syntax in `boot.s` (because GCC's inline assembler uses AT&T by default).
</details>

---

## 🟢 Level 3: Boot Process
*"How does a computer start from nothing?"*

### Concepts:
1. **[[BIOS]]** — Basic Input/Output System (legacy firmware)
2. **[[UEFI]]** — Modern firmware replacement for BIOS
3. **[[Booting]]** — The boot sequence (POST → Bootloader → Kernel)
4. **Master Boot Record (MBR)** — First 512 bytes of the disk
5. **[[Bootloader (GRUB ex)]]** — GRUB and multiboot specification
6. **Multiboot Header** — How GRUB recognizes your kernel
7. **Real Mode → Protected Mode** — CPU mode transition

### Key Questions to Answer:

<details>
<summary>What happens when you press the power button?</summary>

1. **Power supply** sends a "power good" signal to the motherboard
2. **CPU resets** — all registers zeroed, EIP set to `0xFFFFFFF0` (reset vector)
3. **BIOS/UEFI ROM** is mapped at that address — CPU begins executing firmware
4. **POST** (Power-On Self-Test) — checks RAM, detects devices, initializes hardware
5. **Boot device selection** — BIOS reads the first sector (512 bytes) of the boot device (MBR)
6. **Bootloader stage 1** — MBR code loads stage 2 (e.g., GRUB)
7. **GRUB** — loads the kernel ELF binary into memory, sets up multiboot info struct
8. **Mode switch** — GRUB switches CPU from real mode → protected mode
9. **Kernel entry** — GRUB jumps to the kernel's entry point (`_start` in `boot.s`)
10. **`kernel_main()`** — your C code takes over

- [ Computer Boot Process animation ](https://youtu.be/PSnGuvylWBI?si=dK_LVxzqrHObQvxW)
- [ How Does Linux Boot Process Work? ](https://youtu.be/XpFsMB6FoOs?si=zIT8yZqEha09Eije)
- [ BIOS, CMOS, UEFI - What's the difference? ](https://youtu.be/LGz0Io_dh_I?si=EaZdrF4LxyyWzzT7)
</details>

<details>
<summary>What is the multiboot specification and why use it?</summary>

Multiboot is a standard that defines how a bootloader should load a kernel. It eliminates the need to write your own bootloader by establishing a contract:

**Kernel promises:** Place a multiboot header (magic number `0x1BADB002`, flags, checksum) in the first 8 KB of the binary.

**Bootloader promises:**
- Load the kernel into memory at the address specified in the ELF headers (or header fields)
- Switch to 32-bit protected mode
- Disable paging
- Set up a basic GDT
- Put a pointer to a `multiboot_info` struct in **EBX** and the magic `0x2BADB002` in **EAX**

This lets any multiboot-compliant bootloader (GRUB, GRUB2, Limine with multiboot support) load your kernel without changes.

- https://wiki.osdev.org/Multiboot
- 
</details>

<details>
<summary>How does GRUB load your kernel into memory?</summary>

1. GRUB reads its config (`grub.cfg`) to find the kernel path
2. It locates the kernel file on the filesystem (e.g., ISO9660, ext2)
3. It parses the **ELF header** to find:
   - Entry point address (where to jump)
   - Program headers (which sections to load and where)
4. It reads each `PT_LOAD` segment and copies it to the physical address specified in the ELF
5. It prepares the **multiboot info struct** (memory map, boot device, etc.)
6. It sets `EAX = 0x2BADB002` (multiboot magic) and `EBX = address of multiboot info`
7. It jumps to the kernel entry point — your `_start` label in `boot.s`

Your linker script (`linker.ld`) controls the load addresses — typically starting at `0x100000` (1 MB), above the legacy BIOS area.

- https://wiki.osdev.org/Bootloader
- [ Bootloaders Explained ](https://youtu.be/ShKh-18fK54?si=bjjnKdrrG5-iIQXB)
</details>

<details>

<details>
<summary>What is the MBR and why is it limited to 512 bytes?</summary>

The **Master Boot Record** is the very first sector of a disk (LBA 0). The BIOS loads it to address `0x7C00` and jumps to it. Its layout:

```
  Offset   Size    Content
  0x000    446     Bootloader code (stage 1)
  0x1BE    64      Partition table (4 entries × 16 bytes)
  0x1FE    2       Boot signature: 0x55, 0xAA
  ─────────────
  Total:   512 bytes = 1 sector
```

The BIOS only reads **one sector** — that's a hardware/firmware convention from the original IBM PC (1981). The 446 bytes of code must be enough to locate and load a larger stage-2 bootloader. This is why bootloaders like GRUB have multiple stages.

- https://en.wikipedia.org/wiki/Master_boot_record
</details>

---

## 🔵 Level 4: Kernel Fundamentals
*"How do we build the brain of the OS?"*

### Concepts:
1. **[[Kernel (operating system)]]** — The core of the OS
2. **[[Linker (ld)]]** — Combining object files into an executable
3. **Linker Scripts** — Controlling memory layout
4. **Cross-Compilation** — Building for a different target (i386-elf)
5. **ELF Format** — Executable and Linkable Format
6. **GDT (Global Descriptor Table)** — Memory segmentation in protected mode
7. **IDT (Interrupt Descriptor Table)** — Handling interrupts

### Key Questions to Answer:

<details>
<summary>What does a linker script do and why is it necessary?</summary>

The linker script (`linker.ld`) tells the linker **where** to place each section of the kernel in memory. Without it, the linker uses defaults meant for user-space programs (starting at `0x08048000` on Linux) — but your kernel runs bare-metal and needs specific addresses.

Key directives in a typical kernel linker script:
```
ENTRY(_start)              /* Entry point symbol */

SECTIONS {
    . = 0x100000;          /* Start at 1 MB (above BIOS area) */

    .text : { *(.multiboot) *(.text) }   /* Code */
    .rodata : { *(.rodata) }             /* Read-only data */
    .data : { *(.data) }                 /* Initialized data */
    .bss : { *(.bss) }                   /* Uninitialized data (zeroed) */
}
```

The `. = 0x100000` is critical: GRUB loads the kernel exactly where the linker script and ELF headers say. If the addresses are wrong, the kernel will crash immediately.

- [ Compiling, assembling, and linking ](https://youtu.be/N2y6csonII4?si=s9t_rX9jhfqEDN6y)
- [ How do computers read code? ](https://youtu.be/QXjU9qTsYCc?si=3q7Q8dzFva__gJU2)
- [ why do header files even exist? ](https://youtu.be/tOQZlD-0Scc?si=gsUnBgVT3n-gEI4X)
</details>

<details>
<summary>What is the GDT and how do you set it up?</summary>

The **Global Descriptor Table** defines memory segments in protected mode. Each entry (8 bytes) describes a segment's base address, limit (size), and access rights (privilege level, read/write/execute).

A minimal GDT for a flat-memory kernel:

| Entry | Base | Limit | Type | DPL |
|-------|------|-------|------|-----|
| 0 (null) | 0 | 0 | — | — | *(required by CPU)* |
| 1 (code) | 0x0 | 0xFFFFF | Execute/Read | Ring 0 |
| 2 (data) | 0x0 | 0xFFFFF | Read/Write | Ring 0 |

Both code and data segments span the entire 4 GB (base=0, limit=0xFFFFF with 4 KB granularity). This is a **flat memory model** — effectively disabling segmentation while satisfying the CPU's requirement to have a GDT.

Setup: fill the GDT entries in memory, then load it with `lgdt [gdt_descriptor]` where `gdt_descriptor` contains the size and base address of the table. Then reload segment registers (CS via far jump, DS/SS/ES via MOV).

- https://wiki.osdev.org/Global_Descriptor_Table
- [ Building an OS - 8 - Interrupts, IDT, GDT ](https://www.youtube.com/watch?v=5LbXClJhxcs&list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN&index=12)
- [ Making an OS (x86) Chapter 6 - Entering Protected mode, GDT ](https://www.youtube.com/watch?v=Wh5nPn2U_1w&list=PLm3B56ql_akNcvH8vvJRYOc7TbYhRs19M&index=6)
- [ x86 Operating Systems - Implementing a GDT ](https://www.youtube.com/watch?v=jwulDRMQ53I&list=PL2EF13wm-hWAglI8rRbdsCPq_wRpYvQQy&index=3)
</details>

<details>
<summary>How do you handle CPU exceptions (divide by zero, page fault)?</summary>

CPU exceptions are interrupts 0-31, triggered automatically by the CPU:

| INT # | Exception | Type |
|-------|-----------|------|
| 0 | Divide by zero | Fault |
| 6 | Invalid opcode | Fault |
| 8 | Double fault | Abort |
| 13 | General protection fault | Fault |
| 14 | Page fault | Fault |

To handle them:
1. Create handler functions (in assembly: save registers, call C handler, restore, `iret`)
2. Fill the **IDT** (Interrupt Descriptor Table) — 256 entries, each pointing to a handler
3. Load the IDT with `lidt [idt_descriptor]`

A **fault** re-executes the faulting instruction after the handler returns (useful for page faults — map the page, then retry). An **abort** is unrecoverable (double fault = an exception occurred while handling an exception).

- https://www.delorie.com/djgpp/doc/dpmi/ch4.5.html
- https://os.phil-opp.com/cpu-exceptions/
- https://wiki.osdev.org/Exceptions
</details>

<details>
<summary>What is the difference between a monolithic and microkernel?</summary>

| Aspect | Monolithic | Microkernel |
|--------|-----------|-------------|
| **In kernel space** | Everything: drivers, FS, networking, IPC | Only: scheduling, IPC, basic memory management |
| **Drivers** | Run in ring 0 (kernel) | Run in ring 3 (user space) as separate processes |
| **Performance** | Fast (no context switches for driver calls) | Slower (heavy IPC between user-space services) |
| **Reliability** | One buggy driver can crash the whole kernel | A crashed driver can be restarted independently |
| **Examples** | Linux, KFS-1 | MINIX, QNX, seL4 |

KFS-1 is monolithic — everything (VGA, keyboard, commands) runs in a single kernel binary with no user-space separation.

- https://www.geeksforgeeks.org/operating-systems/difference-between-microkernel-and-monolithic-kernel/
- [ Structures of Operating System ](https://youtu.be/XXPBl20J22w?si=hIhrt-c3HbC0tZ3v)
</details>

<details>
<summary>What is the ELF format and why does it matter for kernel development?</summary>

**ELF** (Executable and Linkable Format) is the binary format produced by GCC/LD. It contains:

```
  ┌──────────────────┐
  │  ELF Header      │  magic, architecture, entry point
  │  Program Headers │  segments to load (address, size, permissions)
  │  .text           │  executable code
  │  .rodata         │  read-only data (strings, constants)
  │  .data           │  initialized global variables
  │  .bss            │  uninitialized globals (zeroed at load)
  │  Section Headers │  debug info, symbol tables
  └──────────────────┘
```

GRUB parses the ELF program headers to know where to load each segment in memory. The entry point field tells GRUB where to jump after loading. Without understanding ELF, you can't debug linker issues or understand why your kernel loads at a specific address.

- https://wiki.osdev.org/ELF
- https://medium.com/@abissazaki133/understanding-the-elf-format-a-comprehensive-guide-bab04997001e
- [ How Linux Kernel Runs Executables ](https://youtu.be/ZlZDWeVL2LI?si=hSzW8XMqHKtqfqx7)
- [ In-depth: ELF - The Extensible & Linkable Format ](https://youtu.be/nC1U1LJQL8o?si=d0hx3g6MeLgkbieM)
</details>

---

## 🟣 Level 5: Hardware Interaction (For Bonuses)
*"How does the kernel talk to devices?"*

### Concepts:
1. **[[VGA connector|VGA Text Mode]]** — Writing to the screen (0xB8000)
2. **I/O Ports** — `in` and `out` instructions
3. **[[Keyboard technology]]** — PS/2 keyboard controller (port 0x60, 0x64)
4. **[[Keystroke logging|Scan Codes]]** — How keystrokes are encoded
5. **[[Device driver]]** — Software that controls hardware
6. **PIC (Programmable Interrupt Controller)** — Managing hardware interrupts
7. **IRQs** — Interrupt Request lines (keyboard = IRQ1)

### Key Questions to Answer:

<details>
<summary>How do you write a character to VGA text mode?</summary>

VGA text mode maps a 80×25 character grid to physical memory at `0xB8000`. Each cell is 2 bytes (16 bits):

```
  Byte 0 (low)  = ASCII character
  Byte 1 (high) = Color attribute (4-bit BG | 4-bit FG)
```

To write 'A' in white on black at row 0, column 0:
```c
uint16_t *vga = (uint16_t *)0xB8000;
int index = row * 80 + column;       // = 0
vga[index] = 'A' | (0x0F << 8);     // 0x0F = white FG, black BG
```

That's it — no syscall, no driver API. You're writing directly to hardware-mapped memory. The video card continuously reads this buffer and renders it to the screen.

- https://os.phil-opp.com/vga-text-mode/
- https://wiki.osdev.org/Printing_To_Screen
- [ [030] VGA Text Mode! (Part 2) - Building a GPU From Scratch ](https://www.youtube.com/watch?v=PFazC5LR2eI&list=PLilenfQGj6CEG6iZ4TQJ10PI7pCWsy1AO&index=30)
</details>

<details>
<summary>How do you read a key from the keyboard?</summary>

The PS/2 keyboard controller uses two I/O ports:
- **0x64** (status) — bit 0 = 1 means data is available
- **0x60** (data) — the scancode byte

Polling approach (as in KFS-1):
```c
uint8_t get_scancode(void) {
    if (inb(0x64) & 0x01)    // is data ready?
        return inb(0x60);    // read the scancode
    return 0;                // nothing available
}
```

The scancode is a hardware number (e.g., `0x1E` = 'A' key pressed, `0x9E` = 'A' key released). You translate it to ASCII using a lookup table (`keymap.c`). Key release scancodes have bit 7 set (scancode | 0x80).

- [ How do Computer Keyboards Work? 🤔⌨⌨🛠 ](https://youtu.be/h-NM1xSSzHQ?si=jDw3_abbBwKEmXGr)
- [ How CPUs Interact with So Many Different Devices ](https://youtu.be/tadUeiNe5-g?si=1STwJyNeYbuFN4X5)
- [ How Your Code Really Controls Hardware ](https://youtu.be/sp3mMwo3PO0?si=uIjh-r-21UxDqe14)
</details>

<details>
<summary>What is the difference between polling and interrupts?</summary>

| | Polling | Interrupts |
|-|---------|------------|
| **Mechanism** | CPU spins in a loop checking device status | Device signals CPU via IRQ line |
| **CPU usage** | 100% — stuck in the loop even when idle | 0% when idle — CPU does other work |
| **Latency** | Depends on polling frequency | Near-instant (hardware signal) |
| **Complexity** | Simple to implement | Requires IDT + PIC setup + ISR handlers |
| **KFS-1 uses** | ✅ Polling (`get_scancode()` in a loop) | ❌ Not implemented (no IDT/PIC setup) |

Polling is simpler but wastes CPU cycles. Interrupts are efficient but require setting up the IDT, programming the PIC, and writing ISR (Interrupt Service Routine) handlers in assembly.

- [ #14 Interrupts Explained for Microcontrollers ](https://www.youtube.com/watch?v=cKBqA7IDNpA&list=PL6PplMTH29SEJhWqxhxkDKYPtw0C4mnNY&index=15)
</details>

<details>
<summary>How do you handle the PIC to receive keyboard interrupts?</summary>

The **PIC** (Programmable Interrupt Controller, Intel 8259A) routes hardware IRQs to CPU interrupt numbers. There are two PICs (master + slave), handling IRQ 0-7 and IRQ 8-15. The keyboard is **IRQ 1** on the master PIC.

Setup steps:
1. **Remap the PIC** — by default IRQ 0-7 map to INT 8-15, which conflicts with CPU exceptions. Send ICW (Initialization Command Words) to remap them to INT 32-47:
   ```c
   outb(0x20, 0x11);  // ICW1: begin init, expect ICW4
   outb(0xA0, 0x11);
   outb(0x21, 0x20);  // ICW2: master IRQs → INT 32-39
   outb(0xA1, 0x28);  // ICW2: slave IRQs → INT 40-47
   outb(0x21, 0x04);  // ICW3: slave on IRQ2
   outb(0xA1, 0x02);
   outb(0x21, 0x01);  // ICW4: 8086 mode
   outb(0xA1, 0x01);
   ```
2. **Unmask IRQ 1** — set the interrupt mask register to allow keyboard interrupts:
   ```c
   outb(0x21, 0xFD);  // 0xFD = 11111101 — only IRQ1 enabled
   ```
3. **Set IDT entry 33** (32 + IRQ1) to point to your keyboard ISR
4. **Enable interrupts** — `sti` instruction
5. In the ISR: read scancode from port 0x60, process it, send **EOI** (End of Interrupt) to the PIC:
   ```c
   outb(0x20, 0x20);  // EOI to master PIC
   ```

- [ PIC MCU TUTORIALS #14 - Interrupts & How do they work? (Absolute Beginner) ](https://www.youtube.com/watch?v=hqf9PtsTKmA&list=PLRF1jJIZn8UrmuGBVvzN-c6S45CjyrFmt&index=14)
- [ Building an OS - 10 - Hardware interrupts, PIC ](https://www.youtube.com/watch?v=FD48Lx7Rnx0&list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN&index=14)
</details>

<details>
<summary>What is memory-mapped I/O vs port-mapped I/O?</summary>

**Memory-mapped I/O (MMIO):** Device registers appear as normal memory addresses. You read/write them with regular `MOV` instructions. Example: VGA text buffer at `0xB8000` — writing to that address writes to the video card.

**Port-mapped I/O (PMIO):** Devices have a separate address space accessed via special `IN` and `OUT` instructions. Example: keyboard controller at ports `0x60`/`0x64`, VGA cursor registers at `0x3D4`/`0x3D5`.

x86 supports both. KFS-1 uses MMIO for VGA text and PMIO for everything else (keyboard, cursor, shutdown, reboot).

- [ Memory Mapped IO vs Port Mapped IO (Animation) ](https://youtu.be/bY6NQb10AaI?si=8KM1gVDFfc8DApJk)
</details>

---

## ⚫ Level 6: Advanced Topics (Deep Bonuses)
*"Going further..."*

### Concepts:
1. **[[Memory management]]** — Physical and virtual memory
2. **[[Virtual memory]]** — Paging and page tables
3. **[[System call]]** — User space to kernel space communication
4. **[[Shell (computing)]]** — Command interpreter
5. **Multitasking** — Context switching, scheduling
6. **[[Kernel Virtual Manager (KVM)]]** — Virtualization for testing

### Key Questions to Answer:

<details>
<summary>How does paging work and what is a page table?</summary>

Paging divides memory into fixed-size blocks called **pages** (typically 4 KB). The CPU's **MMU** (Memory Management Unit) translates virtual addresses to physical addresses using page tables.

On x86 (32-bit, no PAE), translation is a two-level lookup:

```
  Virtual address (32 bits):
  ┌──────────┬──────────┬──────────────┐
  │ PD index │ PT index │   Offset     │
  │ (10 bits)│ (10 bits)│  (12 bits)   │
  └────┬─────┴────┬─────┴──────┬───────┘
       │          │            │
       ▼          ▼            │
  Page Directory  Page Table   │
  [PD index] ──► [PT index] ──► Physical page + Offset
```

- **CR3** register holds the physical address of the Page Directory
- Each PD entry points to a Page Table (1024 entries)
- Each PT entry maps one 4 KB page and contains: physical address, present bit, read/write, user/supervisor
- Total: 1024 × 1024 × 4 KB = 4 GB addressable

To enable paging: fill page tables, load CR3, set the PG bit (bit 31) in CR0.

- [ Page Tables and MMU: How Virtual Memory Actually Works Behind the Scenes (Animation) ](https://www.youtube.com/watch?v=B6tJxvYBNrU&list=PL38NNHQLqJqZoDp4CrAueD1aBin7OebEL&index=2)
- [ Segmented, Paged and Virtual Memory ](https://youtu.be/p9yZNLeOj4s?si=rsj0KV9-Trwf5kTo)
</details>

<details>
<summary>What is a system call and how is it implemented?</summary>

A **system call** is how user-space programs request services from the kernel (file I/O, memory allocation, process creation). It's a controlled privilege escalation from ring 3 to ring 0.

On x86 (Linux-style):
1. User program places syscall number in **EAX**, arguments in **EBX, ECX, EDX, ESI, EDI**
2. Executes `INT 0x80` (or `SYSENTER` on modern CPUs)
3. CPU switches to ring 0, jumps to IDT entry 0x80
4. Kernel handler dispatches based on EAX to the correct function
5. Result returned in EAX, `IRET` switches back to ring 3

The kernel must validate all pointers passed from user space — a malicious program could pass kernel-space addresses to trick the kernel into corrupting itself.

- [ Linux System Calls Explained ](https://youtu.be/JdVn3RMLU_Q?si=6tsq-gbBz_vaqSHt)
</details>

<details>
<summary>What is context switching and how does multitasking work?</summary>

**Context switching** is saving the state of one process and restoring another, so multiple programs appear to run simultaneously on a single CPU.

What must be saved per process:
- All general-purpose registers (EAX-EDI)
- EIP (instruction pointer) and EFLAGS
- ESP (stack pointer) — each process has its own kernel stack
- CR3 (page directory) — each process has its own address space
- FPU/SSE state

The **scheduler** decides which process runs next (round-robin, priority-based, CFS). A **timer interrupt** (IRQ 0, typically 100-1000 Hz) fires periodically, invoking the scheduler. This is **preemptive** multitasking — processes don't need to cooperate.

Minimal context switch (simplified):
```asm
; Save current process
push eax, ebx, ecx, ...    ; save registers on current stack
mov [current_task->esp], esp

; Load next process
mov esp, [next_task->esp]
mov cr3, [next_task->cr3]  ; switch address space
pop ecx, ebx, eax, ...    ; restore registers from new stack
ret                        ; pops EIP → resumes next process
```

- [ How Hardware Assist Software When Multitasking ](https://youtu.be/1HHeyUVz43k?si=YnIESl80BTUUCsFQ)
- [ OS Context Switching - Computerphile ](https://youtu.be/DKmBRl8j3Ak?si=tur7zMDlvNLMxXYO)
</details>

<details>
<summary>What is a shell and how does it interact with the kernel?</summary>

A **shell** is a command interpreter — it reads user input, parses commands, and executes them. In a full OS, the shell is a user-space program that uses system calls:

```
while (1) {
    print_prompt();
    read_line(input);           // syscall: read()
    pid = fork();               // syscall: fork()
    if (pid == 0)
        exec(input);            // syscall: execve()
    else
        wait(pid);              // syscall: waitpid()
}
```

In KFS-1, the shell is simpler — it runs **inside** the kernel (no user space, no fork/exec). Commands are dispatched via a static lookup table in `command_parser.c`. Typing `shell` activates prompt mode, and commands like `clear`, `help`, `reboot` are matched by string comparison and call kernel functions directly.

- [ Terminal vs. Bash vs. Command line vs. Prompt ](https://youtu.be/hMSByvFHOro?si=bf1xOzWv8gXq-_Xw)
- [ what is a tty? (beginner - intermediate) anthony explains #022 ](https://youtu.be/SYwbEcNrcjI?si=-O29xDakNKPnbbbg)
</details>

<details>
<summary>What is physical vs virtual memory management?</summary>

**Physical memory management** tracks which physical RAM frames (4 KB pages) are free or in use. Common approaches:
- **Bitmap** — 1 bit per frame (0 = free, 1 = used). Simple but slow to find free frames.
- **Free list** — linked list of free frames. Fast allocation, but no contiguity guarantee.
- **Buddy allocator** — splits memory into power-of-2 blocks, merges adjacent free blocks. Balances speed and fragmentation.

**Virtual memory management** gives each process its own address space via paging. Benefits:
- **Isolation** — process A can't access process B's memory
- **Overcommit** — allocate more virtual memory than physical RAM (swap to disk)
- **Shared memory** — map the same physical page into multiple address spaces
- **Memory-mapped files** — map file contents directly into the address space

- [ What's Virtual Memory? - Computerphile ](https://youtu.be/5lFnKYCZT5o?si=mKhn7LcOMUg1JRAI)
- [ What is Virtual Memory? MMU, Page Tables, and more! ](https://youtu.be/4e18yybPo1E?si=9GwTi1IWGv6qLEJU)
- [ Your Computer is Lying To You (Virtual Memory) ](https://youtu.be/Aw0YAUdQp1c?si=OtYO8CO5-ApPJjmZ)
</details>

<details>
<summary>How do you use QEMU/KVM for kernel development?</summary>

**QEMU** is an emulator that can run your kernel binary without real hardware. Basic usage:
```bash
qemu-system-i386 -cdrom mykernel.iso    # boot from ISO
qemu-system-i386 -kernel kernel.bin      # boot kernel directly (multiboot)
```

**KVM** (Kernel Virtual Machine) is a Linux kernel module that lets QEMU use hardware virtualization (Intel VT-x / AMD-V) for near-native speed. Add `-enable-kvm` to use it.

Useful QEMU flags for kernel development:
| Flag | Purpose |
|------|---------|
| `-s` | Start GDB server on port 1234 |
| `-S` | Pause CPU at startup (wait for GDB) |
| `-d int` | Log interrupts |
| `-no-reboot` | Halt instead of rebooting on triple fault |
| `-serial stdio` | Redirect serial port to terminal (for debug output) |
| `-m 128M` | Set RAM size |

- [ QEMU/KVM for absolute beginners ](https://youtu.be/BgZHbCDFODk?si=E2K6eCQ4xAowCg7l)
</details>

---

## 🎯 Minimum Concepts for Mandatory Part

To complete the **mandatory** requirements, you MUST understand:

1. ✅ Transistors & Logic Gates (conceptually)
2. ✅ x86 Architecture & Registers
3. ✅ Assembly Language (NASM)
4. ✅ BIOS/UEFI Boot Process
5. ✅ GRUB & Multiboot Specification
6. ✅ Linker Scripts
7. ✅ VGA Text Mode (0xB8000)
8. ✅ Cross-compilation (i386-elf-gcc)

---

## 🎁 Additional Concepts for Bonuses

| Bonus Feature | Required Concepts |
|---------------|-------------------|
| Scroll & Cursor | VGA cursor registers, screen buffer manipulation |
| Colors | VGA attribute bytes (foreground/background) |
| printf/printk | Variadic functions, format string parsing |
| Keyboard input | PS/2 controller, scan codes, IRQ1, PIC |
| Multiple screens | Screen buffer management, keyboard shortcuts |

---

## 📺 Your Resources Mapped to Levels

| Resource | Level |
|----------|-------|
| "How Transistors Run Code" | Level 0 |
| "How computers understand programs" | Level 0-1 |
| "Architecture du processeur" | Level 1 |
| "How Computers Work" | Level 0-2 |
| "How Keyboards Work" | Level 5 |
| "OS dev wiki" | Level 3-6 |

---

## ❓ Self-Assessment Checkpoints

Before moving to the next level, you should be able to answer:

### After Level 0-1:
> "Explain how pressing a key on your keyboard eventually becomes a '1' stored in a CPU register."

<details>
<summary>Show answer</summary>

1. You press the '1' key → a mechanical switch closes, completing a circuit
2. The keyboard's microcontroller detects the key matrix change and generates a **scancode** (`0x02` for '1')
3. The keyboard sends this scancode serially over the PS/2 (or USB) cable to the motherboard
4. The **keyboard controller** (Intel 8042) receives the byte and places it in its output buffer (port `0x60`)
5. The controller either raises **IRQ1** (interrupt-driven) or the CPU polls port `0x64` and sees data is ready
6. The CPU executes `IN AL, 0x60` — this triggers the I/O bus to read the byte from the controller
7. The byte travels over the data bus into the CPU and lands in register **AL** (lower 8 bits of EAX)
8. Software (the keymap table) translates scancode `0x02` to ASCII `'1'` (value `0x31`) and stores it in a register or memory

At every stage — keyboard circuits, bus transfers, CPU fetch — the data is just transistors switching on/off, representing the binary value `00110001` (0x31 = '1').
</details>

### After Level 2:
> "Write assembly code that adds two numbers and stores the result."

<details>
<summary>Show answer</summary>

```asm
section .data
    result dd 0           ; reserve 4 bytes for the result

section .text
global _start

_start:
    mov eax, 25           ; first number
    mov ebx, 17           ; second number
    add eax, ebx          ; eax = 25 + 17 = 42
    mov [result], eax     ; store result in memory

    ; Exit cleanly
    mov eax, 1            ; sys_exit
    mov ebx, 0            ; exit code 0
    int 0x80
```

What happens step by step:
1. `MOV EAX, 25` — load immediate value 25 into EAX
2. `MOV EBX, 17` — load immediate value 17 into EBX
3. `ADD EAX, EBX` — ALU performs binary addition, result (42) stored in EAX, flags updated (CF=0, ZF=0)
4. `MOV [result], EAX` — write the 4-byte value from EAX to the memory address of `result`
</details>

### After Level 3:
> "Describe the complete journey from power-on to your kernel's main() function."

<details>
<summary>Show answer</summary>

1. **Power-on** → PSU sends "power good" signal
2. **CPU reset** → EIP = `0xFFFFFFF0`, executes BIOS ROM
3. **POST** → BIOS tests RAM, initializes hardware, enumerates devices
4. **Boot device** → BIOS reads MBR (sector 0, 512 bytes) to `0x7C00`
5. **GRUB stage 1** (in MBR) → loads GRUB stage 2 from disk
6. **GRUB stage 2** → reads `grub.cfg`, displays menu, user selects kernel
7. **Kernel loading** → GRUB parses the kernel's ELF headers, loads segments to addresses specified by `linker.ld` (typically starting at `0x100000`)
8. **Multiboot magic** → GRUB checks the multiboot header (`0x1BADB002`), prepares multiboot info struct
9. **Mode switch** → GRUB transitions CPU: real mode → protected mode (sets up GDT, flips PE bit in CR0)
10. **Jump to kernel** → `EAX = 0x2BADB002`, `EBX = &multiboot_info`, GRUB jumps to `_start`
11. **`boot.s` (_start)** → sets up the kernel stack (`mov esp, stack_top`), calls `kernel_main`
12. **`kernel_main()`** → your C code initializes VGA, keyboard, screens, and enters the main loop
</details>

### After Level 4:
> "Explain what your linker script does and why each section matters."

<details>
<summary>Show answer</summary>

The linker script (`linker.ld`) controls the memory layout of the final kernel binary:

```
ENTRY(_start)          → tells the linker (and GRUB) where execution begins

. = 0x100000           → place the kernel at 1 MB (above BIOS/legacy area: IVT, BDA, VGA)

.text                  → executable code (functions, loops, all instructions)
   .multiboot first    → multiboot header must be in the first 8 KB for GRUB to find it

.rodata                → read-only data (string literals, const arrays, keymap tables)

.data                  → initialized global variables (e.g., int x = 42)

.bss                   → uninitialized globals (e.g., int y;) — zeroed at load, takes no space in binary
```

Why it matters: without the linker script, the multiboot header might end up past 8 KB (GRUB won't find it), or the kernel might overlap with VGA memory (`0xB8000`) or BIOS data areas. The script ensures a deterministic, correct memory layout.
</details>

### After Level 5:
> "Explain how your kernel receives and processes a keypress."

<details>
<summary>Show answer</summary>

In KFS-1 (polling-based, no IRQ):

1. `handle_keyboard_keypress()` runs in an infinite loop
2. `get_scancode()` checks port `0x64` bit 0 — if set, reads the scancode from port `0x60`
3. The scancode is passed through a priority chain:
   - **Screen shortcuts** (`Alt+1..9`) → `handle_screen_shortcut()` switches screens
   - **Scroll keys** (`PgUp/PgDn`) → `handle_scroll_key()` scrolls history
   - **Normal input** → `process_scancode()`
4. `process_scancode()` handles special keys:
   - `KEY_ENTER` → newline + `command_parser()` dispatches the command
   - `KEY_BACKSPACE` → shifts buffer left, erases character
   - Arrow keys → move cursor within the line
5. For printable characters: `keymap_translate()` converts scancode to ASCII (using normal or shifted table based on shift/capslock state)
6. The character is added to `cmdline_buffer` and written to VGA memory via `kfs_putchar()`
7. `update_cursor_pos()` moves the hardware cursor to match
</details>

### After Level 6:
> "Explain how paging provides memory isolation between processes."

<details>
<summary>Show answer</summary>

Each process gets its own **page directory** (pointed to by CR3). When the OS switches between processes, it loads a different CR3 value → the MMU now translates virtual addresses using a completely different set of page tables.

Process A's virtual address `0x08048000` maps to physical frame `0x00500000`.
Process B's virtual address `0x08048000` maps to physical frame `0x00800000`.
Same virtual address, different physical memory — complete isolation.

The kernel maps itself into every process's address space (upper region, e.g., `0xC0000000+`) with supervisor-only permissions. User-space code (ring 3) cannot access pages marked as supervisor (ring 0) — the CPU raises a page fault (INT 14) if it tries. This is how the kernel protects itself from user programs.
</details>

