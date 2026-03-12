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
3. **Binary Arithmetic** — How numbers and data are represented (0s and 1s)
4. **Clock Cycles** — The heartbeat of a computer (synchronization)

### Key Questions to Answer:

<details>
<summary>How does a transistor act as a switch?</summary>

Voltage on the gate → current flows (1). No voltage → no current (0). Billions of MOSFETs switching on/off = computation.
</details>

<details>
<summary>How do logic gates combine to perform addition?</summary>

XOR gate = sum, AND gate = carry. Chain them into a **full adder** (handles carry-in). Cascade N full adders = N-bit addition.
</details>

<details>
<summary>What is a clock signal and why is it necessary?</summary>

Periodic square wave from a crystal oscillator. Synchronizes all CPU operations — one step (fetch/decode/execute) per rising edge. 3 GHz = 3 billion ticks/sec.
</details>

<details>
<summary>What is the difference between combinational and sequential logic?</summary>

**Combinational** = output depends only on inputs (gates). **Sequential** = has state/memory (flip-flops, registers). ALU is combinational; registers are sequential.
</details>

<details>
<summary>How are negative numbers represented in binary?</summary>

**Two's complement**: invert all bits + add 1. MSB = sign bit. Addition works the same for signed/unsigned — no special hardware needed.
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

**EAX** (accumulator/return values), **EBX** (base/GP), **ECX** (counter/loops), **EDX** (data/I/O), **ESI/EDI** (source/dest index), **ESP** (stack top), **EBP** (stack frame base), **EIP** (next instruction — not directly writable), **EFLAGS** (status: CF, ZF, SF, OF, IF). Segment registers: CS, DS, SS, ES, FS, GS.
</details>

<details>
<summary>How does the CPU know what instruction to execute next?</summary>

**EIP** holds the next instruction address. Auto-incremented after fetch. Changed explicitly by `JMP`, `CALL`, `RET`, and interrupts. Not writable via `MOV`.
</details>

<details>
<summary>What is the difference between real mode and protected mode?</summary>

Real mode: 16-bit, 1 MB, no protection. Protected mode: 32-bit, 4 GB, ring-based privilege (0-3), GDT-based segments. Switch: set PE bit in CR0 + far jump.
</details>

<details>
<summary>What is the role of ESP and EBP?</summary>

**ESP** = top of stack (moves on push/pop/call/ret). **EBP** = stable anchor for the current stack frame. Args at `[ebp+8]`, locals at `[ebp-4]`. Set by function prologue: `push ebp; mov ebp, esp`.
</details>

<details>
<summary>What happens during a CPU pipeline stall?</summary>

Data dependency between in-flight instructions → pipeline must wait (bubble). Mitigated by forwarding and branch prediction. Mispredicted branch = full pipeline flush.
</details>

<details>
<summary>How does the CPU cache hierarchy work?</summary>

L1 (~4 cycles, 32-64 KB per core) → L2 (~10 cycles) → L3 (~30-40 cycles, shared) → RAM (~100-200 cycles). Data loaded in 64-byte cache lines. Exploits spatial & temporal locality.
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
</details>

<details>
<summary>What is the difference between real mode and protected mode?</summary>

(Same question as Level 1 — see above for detailed table.)

In the boot context: the CPU starts in **real mode** (16-bit, 1 MB address space, no protection). The bootloader must:
1. Set up a **GDT** with at least code and data segments
2. Set the **PE bit** (bit 0) in the **CR0** register: `mov cr0, eax` (with bit 0 set)
3. Perform a **far jump** to a 32-bit code segment to flush the prefetch queue

After this, the CPU operates in 32-bit protected mode with full 4 GB addressing.
</details>

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
</details>

<details>
<summary>What is memory-mapped I/O vs port-mapped I/O?</summary>

**Memory-mapped I/O (MMIO):** Device registers appear as normal memory addresses. You read/write them with regular `MOV` instructions. Example: VGA text buffer at `0xB8000` — writing to that address writes to the video card.

**Port-mapped I/O (PMIO):** Devices have a separate address space accessed via special `IN` and `OUT` instructions. Example: keyboard controller at ports `0x60`/`0x64`, VGA cursor registers at `0x3D4`/`0x3D5`.

x86 supports both. KFS-1 uses MMIO for VGA text and PMIO for everything else (keyboard, cursor, shutdown, reboot).
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

