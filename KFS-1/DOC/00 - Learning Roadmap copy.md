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

`.text` section with `global _start` entry point. Use `mov`, `add`, `int 0x80` (syscall). Assemble: `nasm -f elf32`, link: `ld -m elf_i386`.
</details>

<details>
<summary>What is the difference between MOV and LEA?</summary>

`MOV eax, [addr]` reads the **value** at addr. `LEA eax, [addr]` computes the **address** itself without accessing memory. LEA is often used for fast arithmetic (e.g., `lea eax, [eax+eax*2]` = eax×3).
</details>

<details>
<summary>How do interrupts work and why are they important?</summary>

Signals that make the CPU jump to a handler. Three types: **hardware IRQs** (devices), **software** (`INT n`), **exceptions** (CPU errors). CPU pushes EFLAGS/CS/EIP, looks up handler in IDT, runs it, `IRET` returns. Without interrupts, you must poll.
</details>

<details>
<summary>How does the stack work during function calls?</summary>

> 📖 Deep dive: [The Stack](00a%20-%20Core%20Concepts%20Deep%20Dive.md#2-the-stack)

Caller pushes args right-to-left, `CALL` pushes return address. Callee prologue: `push ebp; mov ebp, esp; sub esp, N`. Args at `[ebp+8]`, locals at `[ebp-4]`. `RET` pops return address → EIP. (**cdecl** convention.)
</details>

<details>
<summary>What is the difference between NASM and GAS (AT&T) syntax?</summary>

NASM (Intel): `mov eax, [ebx+8]` (dest, src). GAS (AT&T): `movl 8(%ebx), %eax` (src, dest, `%`/`$` prefixes, size suffixes). KFS-1 uses GAS in `boot.s`.
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

> 📖 Deep dive: [Booting Process](00a%20-%20Core%20Concepts%20Deep%20Dive.md#3-booting-process)

<details>
<summary>What happens when you press the power button?</summary>

PSU → "power good" → CPU reset (EIP=`0xFFFFFFF0`) → BIOS ROM → POST → load MBR (512b) to `0x7C00` → GRUB stage 1 → GRUB stage 2 → parse ELF → load kernel at `0x100000` → switch to protected mode → jump to `_start` → `kernel_main()`.
</details>

<details>
<summary>What is the multiboot specification and why use it?</summary>

A contract: kernel places a header (magic `0x1BADB002`, flags, checksum) in first 8 KB. Bootloader loads the ELF, switches to protected mode, sets `EAX=0x2BADB002` + `EBX=multiboot_info`, jumps to entry. No need to write your own bootloader.
</details>

<details>
<summary>How does GRUB load your kernel into memory?</summary>

Reads `grub.cfg` → finds kernel on filesystem → parses ELF program headers → copies each `PT_LOAD` segment to specified physical address → sets EAX/EBX → jumps to entry point. Your `linker.ld` controls the load addresses.
</details>

<details>
<summary>What is the difference between real mode and protected mode?</summary>

Real: 16-bit, 1 MB, no protection. Protected: 32-bit, 4 GB, ring 0-3. Switch: load GDT → set PE bit in CR0 → far jump to 32-bit code.
</details>

<details>
<summary>What is the MBR and why is it limited to 512 bytes?</summary>

First sector of disk. BIOS loads exactly 1 sector (512 bytes) to `0x7C00`. Layout: 446 bytes code + 64 bytes partition table + 2 bytes signature (`0x55AA`). Convention from IBM PC (1981).
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

> 📖 Deep dive: [Linker & Linker Scripts](00a%20-%20Core%20Concepts%20Deep%20Dive.md#6-linker--linker-scripts) — [The Kernel](00a%20-%20Core%20Concepts%20Deep%20Dive.md#7-the-kernel) — [Cross-Compilation](00a%20-%20Core%20Concepts%20Deep%20Dive.md#8-cross-compilation) — [ELF Format](00a%20-%20Core%20Concepts%20Deep%20Dive.md#9-elf-format)

<details>
<summary>What does a linker script do and why is it necessary?</summary>

Tells the linker where to place each section in memory. Sets entry point (`ENTRY(_start)`), load address (`. = 0x100000` = 1 MB), and section ordering (`.multiboot` first so GRUB finds it in first 8 KB). Without it, the linker uses defaults meant for user-space programs.
</details>

<details>
<summary>What is the GDT and how do you set it up?</summary>

Global Descriptor Table: defines memory segments in protected mode. Minimum: null entry + code segment (base=0, limit=4GB, exec/read, ring 0) + data segment (base=0, limit=4GB, read/write, ring 0). This is a flat memory model. Load with `lgdt`, then reload CS (far jump) and DS/SS/ES (mov).
</details>

<details>
<summary>How do you handle CPU exceptions?</summary>

Exceptions = interrupts 0-31 (div by zero=0, GPF=13, page fault=14). Create handler functions, fill the IDT (256 entries), load with `lidt`. Faults re-execute the faulting instruction; aborts (double fault) are unrecoverable.
</details>

<details>
<summary>What is the difference between a monolithic and microkernel?</summary>

Monolithic: everything (drivers, FS, networking) in ring 0 — fast, but one bug crashes all (Linux, KFS-1). Microkernel: only IPC/scheduling in kernel, drivers in user space — safer but slower (MINIX, seL4).
</details>

<details>
<summary>What is the ELF format?</summary>

Binary format produced by GCC/LD. Contains: ELF header (magic, entry point), program headers (`PT_LOAD` segments for the loader), sections (`.text`, `.rodata`, `.data`, `.bss`), section headers (debug/symbols). GRUB reads program headers to know what/where to load.
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

> 📖 Deep dive: [VGA Text Mode](00a%20-%20Core%20Concepts%20Deep%20Dive.md#5-vga-text-mode--frame-buffer) — [Keyboard & I/O Ports](00a%20-%20Core%20Concepts%20Deep%20Dive.md#10-keyboard--io-ports) — [Shutdown & Reboot](00a%20-%20Core%20Concepts%20Deep%20Dive.md#4-shutdown--reboot)

<details>
<summary>How do you write a character to VGA text mode?</summary>

Write directly to `0xB8000`. Each cell = 2 bytes: low byte = ASCII, high byte = color (4-bit BG + 4-bit FG). Index = `row * 80 + col`. No syscalls, no driver — pure memory-mapped I/O.
</details>

<details>
<summary>How do you read a key from the keyboard?</summary>

Poll port `0x64` bit 0 (data ready?), then read scancode from port `0x60`. Translate scancode to ASCII via lookup table. Key release = scancode | 0x80.
</details>

<details>
<summary>What is the difference between polling and interrupts?</summary>

Polling: CPU loops checking device status — simple but 100% busy. Interrupts: device raises IRQ, CPU jumps to handler — 0% idle CPU but needs IDT+PIC setup. KFS-1 uses polling.
</details>

<details>
<summary>How do you handle the PIC to receive keyboard interrupts?</summary>

Remap PIC (IRQ 0-7 → INT 32-39) via ICW commands to ports `0x20`/`0x21`/`0xA0`/`0xA1`. Unmask IRQ1 (`outb(0x21, 0xFD)`). Set IDT entry 33 to your ISR. Enable interrupts (`sti`). In ISR: read port 0x60, process, send EOI (`outb(0x20, 0x20)`).
</details>

<details>
<summary>What is memory-mapped I/O vs port-mapped I/O?</summary>

**MMIO**: devices at memory addresses, accessed via `MOV` (VGA at `0xB8000`). **PMIO**: separate port address space, accessed via `IN`/`OUT` (keyboard at `0x60`/`0x64`). x86 supports both. KFS-1 uses MMIO for VGA, PMIO for everything else.
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

> 📖 Deep dive: [Memory Management](00a%20-%20Core%20Concepts%20Deep%20Dive.md#11-memory-management) — [Virtual Memory & Paging](00a%20-%20Core%20Concepts%20Deep%20Dive.md#12-virtual-memory--paging) — [QEMU & KVM](00a%20-%20Core%20Concepts%20Deep%20Dive.md#13-qemu--kvm)

<details>
<summary>How does paging work and what is a page table?</summary>

Memory divided into 4 KB pages. 2-level lookup: virtual address → Page Directory index (10 bits) → Page Table index (10 bits) → physical frame + offset (12 bits). CR3 = page directory base. Enable: fill tables, load CR3, set PG bit in CR0.
</details>

<details>
<summary>What is a system call and how is it implemented?</summary>

Controlled privilege escalation: ring 3 → ring 0. User puts syscall # in EAX, args in EBX-EDI, triggers `INT 0x80`. CPU jumps to kernel handler via IDT, dispatches based on EAX, returns result in EAX, `IRET` back to ring 3.
</details>

<details>
<summary>What is context switching and how does multitasking work?</summary>

Save current process state (all registers, ESP, CR3), load next process state. Timer interrupt (IRQ0) triggers scheduler periodically → preemptive multitasking. Each process has its own stack and page directory.
</details>

<details>
<summary>What is a shell and how does it interact with the kernel?</summary>

Command interpreter. Full OS: user-space program using fork/exec/wait syscalls. KFS-1: runs inside the kernel, commands dispatched via static lookup table in `command_parser.c` — no user space, no process creation.
</details>

<details>
<summary>What is physical vs virtual memory management?</summary>

**Physical**: track free/used 4 KB frames (bitmap, free list, or buddy allocator). **Virtual**: per-process address space via paging → isolation, overcommit, shared memory. KFS-1 uses static allocation (no dynamic memory management).
</details>

<details>
<summary>How do you use QEMU/KVM for kernel development?</summary>

`qemu-system-i386 -cdrom kernel.iso`. Add `-enable-kvm` for hardware acceleration, `-s -S` for GDB debugging, `-no-reboot` to catch triple faults. KVM = run guest code on real CPU, only intercept I/O.
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

Key press → keyboard encoder generates scancode (`0x02`) → sent to 8042 controller (port `0x60`) → CPU reads with `IN AL, 0x60` → byte lands in register AL → keymap translates scancode to ASCII `'1'` (`0x31`). Every stage is transistors switching on/off representing binary `00110001`.
</details>

### After Level 2:
> "Write assembly code that adds two numbers and stores the result."

<details>
<summary>Show answer</summary>

`mov eax, 25` → `mov ebx, 17` → `add eax, ebx` (result 42 in EAX) → `mov [result], eax` (store to memory). ALU performs binary addition, flags updated (CF=0, ZF=0).
</details>

### After Level 3:
> "Describe the complete journey from power-on to your kernel's main() function."

<details>
<summary>Show answer</summary>

PSU → CPU reset (EIP=`0xFFFFFFF0`) → BIOS ROM → POST → load MBR to `0x7C00` → GRUB stage 1 → GRUB stage 2 (reads `grub.cfg`, parses ELF, loads kernel at `0x100000`) → checks multiboot header → switches to protected mode → EAX=magic, EBX=info, jumps to `_start` → `boot.s` sets up stack → calls `kernel_main()` → inits VGA, screens, keyboard, enters polling loop.
</details>

### After Level 4:
> "Explain what your linker script does and why each section matters."

<details>
<summary>Show answer</summary>

`ENTRY(_start)` = entry point. `. = 0x100000` = load at 1 MB (above BIOS area). `.text` with `.multiboot` first (GRUB needs it in first 8 KB). `.rodata` = strings/const tables. `.data` = initialized globals. `.bss` = uninitialized globals (zeroed, no space in binary). Without it, multiboot header could be misplaced or kernel could overlap VGA/BIOS memory.
</details>

### After Level 5:
> "Explain how your kernel receives and processes a keypress."

<details>
<summary>Show answer</summary>

Polling loop: `get_scancode()` checks port `0x64` → reads port `0x60`. Priority chain: screen shortcuts (Alt+N) → scroll keys (PgUp/PgDn) → normal input. Special keys: Enter → `command_parser()`, Backspace → erase, Arrows → move cursor. Printable: `keymap_translate()` → `add_char_to_cmdline_buffer()` → `kfs_putchar()` writes to VGA → `update_cursor_pos()`.
</details>

### After Level 6:
> "Explain how paging provides memory isolation between processes."

<details>
<summary>Show answer</summary>

Each process has its own page directory (CR3). On context switch, load different CR3 → MMU uses different page tables. Same virtual address maps to different physical frames per process → complete isolation. Kernel mapped in all address spaces with supervisor-only permissions — ring 3 code triggers page fault (INT 14) if it tries to access kernel pages.
</details>

