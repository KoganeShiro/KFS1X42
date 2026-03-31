# Core Concepts — Deep Dive
> In-depth explanations of every concept needed to build KFS-1.
> Companion to [00 - Learning Roadmap.md](00%20-%20Learning%20Roadmap.md) (which has the TL;DR versions).

---

## Table of Contents

1. [CPU Architecture & Registers](#1-cpu-architecture--registers)
2. [The Stack](#2-the-stack)
3. [Booting Process](#3-booting-process)
4. [Shutdown & Reboot](#4-shutdown--reboot)
5. [VGA Text Mode / Frame Buffer](#5-vga-text-mode--frame-buffer)
6. [Linker & Linker Scripts](#6-linker--linker-scripts)
7. [The Kernel](#7-the-kernel)
8. [Cross-Compilation](#8-cross-compilation)
9. [ELF Format](#9-elf-format)
10. [Keyboard & I/O Ports](#10-keyboard--io-ports)
11. [Memory Management](#11-memory-management)
12. [Virtual Memory & Paging](#12-virtual-memory--paging)
13. [QEMU & KVM](#13-qemu--kvm)
14. [GDT (Global Descriptor Table)]()

---

## 1. CPU Architecture & Registers

### What is the x86 (i386) architecture?

x86 is the instruction set architecture (ISA) used by Intel and AMD processors since the Intel 8086 (1978). **i386** specifically targets the 32-bit variant introduced with the Intel 80386 (1985). KFS-1 compiles for i386 — 32-bit protected mode.

### Register map

```
  ┌─────────────────────────────────────────────────────────────┐
  │                     32-bit (E__)                             │
  │  ┌──────────────────────────────────────────────────────┐   │
  │  │ EAX  ─────────────────────────────────────────────── │   │
  │  │      │           │      AX       │                   │   │
  │  │      │           │  AH   │  AL   │                   │   │
  │  │      31        16 15    8 7     0                    │   │
  │  └──────────────────────────────────────────────────────┘   │
  │                                                             │
  │  Same layout for EBX (BH/BL), ECX (CH/CL), EDX (DH/DL)    │
  │  ESI, EDI, ESP, EBP — no 8-bit sub-registers               │
  └─────────────────────────────────────────────────────────────┘
```

### General-purpose registers

| Register | Name | Primary use | Also used for |
|----------|------|-------------|---------------|
| **EAX** | Accumulator | Return values, arithmetic | `MUL`/`DIV` implicit operand |
| **EBX** | Base | General purpose | Base address in real-mode segmentation |
| **ECX** | Counter | Loop counter (`LOOP`, `REP`) | Shift/rotate count |
| **EDX** | Data | I/O (`IN`/`OUT`), 64-bit mul/div overflow | Port number for `IN`/`OUT` |
| **ESI** | Source Index | Source for string ops (`MOVSB`, `CMPSB`) | General pointer |
| **EDI** | Destination Index | Destination for string ops | General pointer |
| **ESP** | Stack Pointer | Top of stack | **Never use for anything else** |
| **EBP** | Base Pointer | Stack frame anchor | Can be GP with `-fomit-frame-pointer` |

### Special registers

| Register | Purpose |
|----------|---------|
| **EIP** | Instruction Pointer — next instruction to execute. Not writable via `MOV`. Changed by `JMP`, `CALL`, `RET`, `INT`, `IRET`. |
| **EFLAGS** | Bit flags set by arithmetic/comparison instructions: |
| | - CF (bit 0): Carry flag — unsigned overflow |
| | - ZF (bit 6): Zero flag — result was zero |
| | - SF (bit 7): Sign flag — result was negative (MSB = 1) |
| | - OF (bit 11): Overflow flag — signed overflow |
| | - IF (bit 9): Interrupt flag — are hardware interrupts enabled? |
| **CR0** | Control register 0 — PE bit (bit 0) enables protected mode, PG bit (bit 31) enables paging |
| **CR3** | Page directory base register — physical address of the page directory |

### Segment registers

| Register | Name | Role |
|----------|------|------|
| **CS** | Code Segment | Points to the GDT entry for code — determines privilege level |
| **DS** | Data Segment | Default segment for data access |
| **SS** | Stack Segment | Segment for stack operations (`PUSH`/`POP`) |
| **ES, FS, GS** | Extra segments | General purpose; FS/GS often used for thread-local storage |

In a flat memory model (like KFS-1), CS, DS, SS, and ES all point to segments covering the entire 4 GB with base 0 — effectively making segmentation transparent.

### Instruction cycle

```
  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌───────────┐
  │  FETCH  │───►│ DECODE  │───►│ EXECUTE │───►│ MEMORY  │───►│ WRITEBACK │
  │ Read    │    │ What    │    │ ALU /   │    │ Load /  │    │ Store     │
  │ instr   │    │ instr   │    │ branch  │    │ Store   │    │ result    │
  │ at EIP  │    │ is it?  │    │ compute │    │ from/to │    │ to reg    │
  └─────────┘    └─────────┘    └─────────┘    │ RAM     │    └───────────┘
                                               └─────────┘
       ◄──────── One clock cycle per stage (pipelined) ────────►
```

### Pipeline hazards

| Hazard | Cause | Example | Mitigation |
|--------|-------|---------|------------|
| **Data** | Instruction needs result from previous one | `ADD EAX,1` then `MOV EBX,EAX` | Forwarding / bypassing |
| **Control** | Branch changes EIP unpredictably | `JNZ loop` | Branch prediction |
| **Structural** | Two instructions need same hardware unit | Two divisions at once | Stall |

When a branch prediction fails, the pipeline is **flushed** — all in-flight instructions after the branch are discarded. On a modern 15-stage pipeline, this costs ~15 cycles.

### Cache hierarchy

```
  CPU Core
  ┌──────────────────────────────────────┐
  │  ┌──────┐  ┌──────┐                 │
  │  │ L1i  │  │ L1d  │  32-64 KB each  │  ~1-4 cycles
  │  │(code)│  │(data)│                  │
  │  └──┬───┘  └──┬───┘                 │
  │     └────┬────┘                      │
  │     ┌────▼────┐                      │
  │     │   L2    │  256 KB - 1 MB       │  ~10 cycles
  │     └────┬────┘                      │
  └──────────┼───────────────────────────┘
        ┌────▼────┐
        │   L3    │  4-32 MB (shared)       ~30-40 cycles
        └────┬────┘
        ┌────▼────┐
        │   RAM   │  4-64 GB               ~100-200 cycles
        └─────────┘
```

Data is loaded in **cache lines** (64 bytes). Accessing one byte of a struct will pull the next 63 bytes into cache too — this is why sequential memory access is fast and random access is slow (cache misses).

---

## 2. The Stack

### What is the stack?

The stack is a region of memory used for:
- **Function call management** — return addresses, saved registers
- **Local variables** — allocated on function entry, freed on exit
- **Parameter passing** — caller pushes arguments before `CALL`

It grows **downward** on x86: pushing decreases ESP, popping increases it.

### Stack operations

```
  PUSH EAX:                    POP EBX:
  1. ESP = ESP - 4             1. EBX = [ESP]    (read top)
  2. [ESP] = EAX  (store)      2. ESP = ESP + 4  (shrink)

  CALL my_func:                RET:
  1. PUSH EIP  (return addr)   1. POP EIP  (jump back)
  2. EIP = my_func             
```

### Stack frame layout (cdecl calling convention)

When function `foo(int a, int b)` is called with `foo(10, 20)`:

```
  Caller side:          │   Callee side (foo):
  push 20    (arg b)    │   push ebp           ; save caller's frame
  push 10    (arg a)    │   mov  ebp, esp      ; new frame base
  call foo              │   sub  esp, 8        ; 2 local ints
                        │
  Stack during foo:     │
  ┌────────────────┐    │
  │   20 (arg b)   │ [ebp+12]
  │   10 (arg a)   │ [ebp+8]
  │  return addr   │ [ebp+4]    ← pushed by CALL
  │  saved EBP     │ [ebp]      ← pushed by prologue
  │  local var 1   │ [ebp-4]
  │  local var 2   │ [ebp-8]   ← ESP points here
  └────────────────┘
```

**cdecl rules:**
- Caller pushes arguments **right to left** (so first arg is closest to EBP)
- Caller cleans up the stack after `call` returns (`add esp, 8`)
- Return value in **EAX**
- Callee must preserve **EBX, ESI, EDI, EBP** (callee-saved)
- Callee may clobber **EAX, ECX, EDX** (caller-saved)

### Stack in KFS-1

In `boot.s`, the kernel sets up its own stack:
```asm
.section .bss
.align 16
stack_bottom:
    .skip 16384       # 16 KB stack
stack_top:

.section .text
_start:
    mov $stack_top, %esp   # point ESP to top of our stack
    call kernel_main       # enter C code
```

This is a static 16 KB stack. There is no stack guard, no dynamic growth, no separate stacks per process (KFS-1 is single-tasked).

---

## 3. Booting Process

### Full boot sequence (power button → kernel_main)

```
  ┌─────────────────────────────────────────────────────────────┐
  │ 1. POWER ON                                                 │
  │    PSU → "power good" signal → motherboard                  │
  │                                                             │
  │ 2. CPU RESET                                                │
  │    All registers = 0, EIP = 0xFFFFFFF0 (reset vector)       │
  │    CPU is in REAL MODE (16-bit, 1 MB address space)         │
  │                                                             │
  │ 3. BIOS / UEFI                                              │
  │    ROM chip mapped at reset vector                           │
  │    POST: test RAM, detect CPU, init chipset                  │
  │    Enumerate devices (PCI, USB, PS/2)                        │
  │    Build BIOS data area (BDA) at 0x0400                     │
  │    Select boot device from CMOS/NVRAM config                 │
  │                                                             │
  │ 4. MBR LOAD                                                 │
  │    BIOS reads sector 0 (512 bytes) to 0x7C00                │
  │    Checks signature: bytes 510-511 must be 0x55AA            │
  │    Jumps to 0x7C00 → bootloader stage 1 runs                │
  │                                                             │
  │ 5. GRUB STAGE 1  (in MBR, 446 bytes)                        │
  │    Enough code to find & load GRUB stage 2 from disk         │
  │                                                             │
  │ 6. GRUB STAGE 2                                             │
  │    Full bootloader with filesystem drivers                   │
  │    Reads grub.cfg → shows menu → user picks kernel           │
  │    Parses kernel ELF: finds entry point & PT_LOAD segments   │
  │    Copies kernel to physical addresses from ELF headers      │
  │                                                             │
  │ 7. MULTIBOOT HANDOFF                                        │
  │    GRUB verifies multiboot header (magic 0x1BADB002)         │
  │    Sets up basic GDT (flat model)                            │
  │    Switches CPU: real mode → protected mode (PE bit in CR0)  │
  │    Far jump to 32-bit code segment                           │
  │    EAX = 0x2BADB002 (multiboot magic)                        │
  │    EBX = address of multiboot_info struct                    │
  │    Jumps to kernel entry point (_start)                      │
  │                                                             │
  │ 8. KERNEL ENTRY (boot.s)                                    │
  │    _start:                                                   │
  │        mov $stack_top, %esp     ← set up kernel stack        │
  │        call kernel_main         ← enter C code               │
  │        cli                      ← disable interrupts         │
  │        hlt                      ← halt CPU (if main returns) │
  │                                                             │
  │ 9. KERNEL_MAIN (kernel.c)                                   │
  │    kfs_initialize() → init VGA, screens, keyboard, cursor    │
  │    handle_keyboard_keypress() → infinite polling loop        │
  └─────────────────────────────────────────────────────────────┘
```

### The Multiboot Header

The multiboot header must appear in the first 8192 bytes of the kernel binary. In KFS-1 (`boot.s`):

```asm
.section .multiboot
.align 4
    .long 0x1BADB002              # magic number
    .long 0x00                    # flags (no special requirements)
    .long -(0x1BADB002 + 0x00)    # checksum (magic + flags + checksum = 0)
```

The checksum ensures integrity: `magic + flags + checksum` must equal zero (as a 32-bit unsigned sum).

### Memory map at boot time

```
  Physical address        Content
  ──────────────────────────────────────────────
  0x00000000 - 0x000003FF   Interrupt Vector Table (IVT) — 256 entries × 4 bytes
  0x00000400 - 0x000004FF   BIOS Data Area (BDA) — keyboard state, video mode, etc.
  0x00000500 - 0x00007BFF   Free (conventional memory)
  0x00007C00 - 0x00007DFF   MBR loaded here by BIOS
  0x00007E00 - 0x0009FFFF   Free (conventional memory)
  0x000A0000 - 0x000BFFFF   VGA frame buffer
     0xB8000 - 0xB8F9F        VGA text mode (80×25×2 = 4000 bytes)
  0x000C0000 - 0x000FFFFF   BIOS ROM, option ROMs
  0x00100000 - ...           Free — YOUR KERNEL LOADS HERE (1 MB+)
```

### Real Mode → Protected Mode transition

```
  REAL MODE                        PROTECTED MODE
  ┌─────────────────┐              ┌─────────────────┐
  │ 16-bit          │              │ 32-bit          │
  │ 1 MB max        │   ────────►  │ 4 GB max        │
  │ No protection   │   flip PE    │ Ring 0-3        │
  │ Segment:Offset  │   in CR0     │ GDT-based       │
  └─────────────────┘              └─────────────────┘

  Steps (done by GRUB):
  1. Disable interrupts (cli)
  2. Load GDT (lgdt)
  3. Set PE bit: mov eax, cr0 ; or eax, 1 ; mov cr0, eax
  4. Far jump to 32-bit code segment (flushes prefetch queue)
  5. Reload DS, SS, ES with data segment selector
  6. Re-enable interrupts if needed (sti)
```

---

## 4. Shutdown & Reboot

### Shutdown (QEMU ACPI)

There is no universal x86 hardware instruction for power-off. On real hardware, you'd parse ACPI tables (DSDT/FADT) to find the PM1a control register and write the SLP_TYP value. QEMU provides a shortcut:

```c
void kfs_shutdown(screen *scr) {
    outw(0x604, 0x2000);   // QEMU >= 2.0 ACPI shutdown
}
```

Port `0x604` is the QEMU-specific PM1a control port. Writing `0x2000` triggers an ACPI S5 (soft-off) transition. This **only works in QEMU** — it won't work on real hardware or other emulators (Bochs uses port `0xB004`).

### Reboot (8042 keyboard controller)

The i8042 keyboard controller has a GPIO pin connected to the CPU's RESET line. Sending command `0xFE` to port `0x64` pulses that pin:

```c
void kfs_reboot(screen *scr) {
    uint8_t status;
    do {
        status = inb(0x64);         // read status register
    } while (status & 0x02);        // wait until input buffer empty (bit 1 = 0)

    outb(0x64, 0xFE);              // send RESET command
}
```

```
  ┌──────────────────┐         ┌──────────────┐
  │   CPU            │         │  8042 KBC    │
  │                  │  0xFE   │              │
  │  outb(0x64,0xFE)│────────►│ Port 0x64    │
  │                  │         │  ├── pulse   │
  │  RESET line ◄────────────────── RESET pin │
  │  CPU restarts    │         │              │
  └──────────────────┘         └──────────────┘
```

The polling loop (`while (status & 0x02)`) is necessary because the controller can't accept a new command if its input buffer is full. Bit 1 of the status register indicates whether the input buffer is occupied.

### Other reboot methods

| Method | How | Works on |
|--------|-----|----------|
| 8042 reset (KFS-1) | `outb(0x64, 0xFE)` | Most x86 hardware |
| Triple fault | Load invalid IDT, trigger exception | Universal fallback |
| ACPI reset | Write to ACPI reset register (from FADT) | Modern hardware |
| PCI reset | Write to CF9 port | Some chipsets |

---

## 5. VGA Text Mode / Frame Buffer

### What is VGA text mode?

VGA text mode is a hardware display mode where the video card renders a grid of characters from a memory buffer. No pixel-level control — the card has a built-in character ROM (font) and renders each cell as an 8×16 (or 9×16) pixel glyph automatically.

### Memory layout

The VGA text buffer is memory-mapped at physical address `0xB8000`:

```
  Address 0xB8000                                    Address 0xB8F9F
  ┌────┬────┬────┬────┬─────────────── ─ ─ ─ ────┬────┬────┐
  │char│attr│char│attr│                           │char│attr│
  │ 00 │ 00 │ 01 │ 01 │  ...  80×25 = 2000 cells │1999│1999│
  └────┴────┴────┴────┴─────────────── ─ ─ ─ ────┴────┴────┘
    col0      col1                                  col79
    row0      row0                                  row24

  Each cell = 2 bytes (uint16_t):
  ┌─────────────────────────────────┐
  │  Bits 15-12  │ Bits 11-8  │ Bits 7-0  │
  │  Background  │ Foreground │  ASCII    │
  │  (4 bits)    │ (4 bits)   │  char     │
  └─────────────────────────────────┘
```

### Color attribute byte

```
  Bit:  7    6  5  4    3    2  1  0
       ┌────┬────────┬────┬────────┐
       │Blnk│  BG    │ Hi │  FG    │
       │    │ R G B  │    │ R G B  │
       └────┴────────┴────┴────────┘

  Standard colors (4-bit):
  0x0 = Black        0x8 = Dark Grey
  0x1 = Blue         0x9 = Light Blue
  0x2 = Green        0xA = Light Green
  0x3 = Cyan         0xB = Light Cyan
  0x4 = Red          0xC = Light Red
  0x5 = Magenta      0xD = Light Magenta
  0x6 = Brown        0xE = Light Brown (Yellow)
  0x7 = Light Grey   0xF = White
```

### Writing to the screen in C

```c
// Create a VGA entry (character + color)
uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

// Write character at position
uint16_t *buffer = (uint16_t *)0xB8000;
int index = row * VGA_WIDTH + column;  // VGA_WIDTH = 80
buffer[index] = vga_entry('H', 0x0F);  // white 'H' on black
```

### Hardware cursor

The VGA controller has a hardware cursor (blinking underscore). Its position is controlled via I/O ports, **not** through the frame buffer:

```
  Set cursor position:
  1. Calculate linear position: pos = row * 80 + col
  2. Send high byte:
       outb(0x3D4, 0x0E);         // select cursor high register
       outb(0x3D5, pos >> 8);     // write high byte
  3. Send low byte:
       outb(0x3D4, 0x0F);         // select cursor low register
       outb(0x3D5, pos & 0xFF);   // write low byte
```

The cursor position and the character position are independent — you must update both when the user types.

### KFS-1 screen architecture

KFS-1 supports 9 virtual screens. Only one is displayed at a time (mapped to `0xB8000`). Inactive screens are stored in RAM buffers:

```
  ┌──────────────┐    ┌───────────────┐
  │  Screen 0    │───►│  0xB8000      │  ← active (visible)
  │  (active)    │    │  VGA memory   │
  └──────────────┘    └───────────────┘

  ┌──────────────┐    ┌───────────────┐
  │  Screen 1    │───►│  saved_buffer │  ← in RAM
  │  (inactive)  │    │  (uint16_t[]) │
  └──────────────┘    └───────────────┘

  On switch (Alt+N):
  1. memcpy(active.saved_buffer, 0xB8000)   // save current
  2. memcpy(0xB8000, new.saved_buffer)      // restore new
  3. Update cursor position for new screen
```

---

## 6. Linker & Linker Scripts

### What does the linker do?

The **linker** (`ld`) takes multiple object files (`.o`) produced by the compiler and combines them into a single executable. It:
1. **Resolves symbols** — connects `call printk` in one file to the `printk` function defined in another
2. **Merges sections** — all `.text` sections become one, all `.data` become one, etc.
3. **Assigns addresses** — places each section at the correct memory address
4. **Fixes up relocations** — patches addresses that weren't known at compile time

### Why a linker script?

Without a linker script, `ld` uses defaults designed for Linux user-space programs. For a kernel, you need to control:
- Where the binary is loaded (must be above 1 MB for multiboot)
- That the multiboot header appears in the first 8 KB
- Section ordering and alignment

### KFS-1 linker script explained

```ld
/* linker.ld */
ENTRY(_start)                    /* Tell GRUB/linker where to start */

SECTIONS {
    . = 0x100000;                /* Load address: 1 MB */

    /* Code section — multiboot header MUST come first */
    .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)            /* Multiboot header (first!) */
        *(.text)                 /* All code from all .o files */
    }

    /* Read-only data (string literals, const tables) */
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    /* Initialized global variables */
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    /* Uninitialized globals — zeroed, takes no space in binary */
    .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
    }
}
```

### Address assignment walkthrough

```
  . = 0x100000                    ← location counter starts at 1 MB

  .text (size: 0x2000)            ← 0x100000 to 0x101FFF
      .multiboot at 0x100000      ← GRUB finds header here ✓
      code starts at 0x100010

  .rodata (size: 0x500)           ← 0x102000 (aligned to 4K)
      string constants, keymap tables

  .data (size: 0x100)             ← 0x103000 (aligned to 4K)
      initialized globals

  .bss (size: 0x5000)             ← 0x104000 (aligned to 4K)
      screen buffers, stack space
      NOT stored in the binary (zeroed at load)
```

### Key symbols

```
  _start           → entry point (in boot.s)
  kernel_main      → C entry point (called from _start)
  stack_top        → top of kernel stack (defined in .bss)
```

---

## 7. The Kernel

### What is a kernel?

The kernel is the core of the operating system. It's the first user-defined code that runs after the bootloader and has full control over the hardware (ring 0 privilege). It manages:
- **Hardware abstraction** — drivers for screen, keyboard, disk
- **Memory management** — allocating/freeing RAM, virtual memory
- **Process management** — creating, scheduling, terminating programs
- **System calls** — providing services to user-space programs

### Kernel types

| Type | Description | Example |
|------|-------------|---------|
| **Monolithic** | Everything in one binary, ring 0 | Linux, KFS-1 |
| **Microkernel** | Minimal kernel, drivers in user space | MINIX, QNX, seL4 |
| **Hybrid** | Monolithic core + some user-space services | Windows NT, macOS (XNU) |
| **Exokernel** | Minimal abstraction, apps manage hardware | MIT Exokernel |

### KFS-1 kernel architecture

```
  ┌─────────────────────────────────────────────────────┐
  │                    kernel_main()                     │
  │  ┌─────────────┐  ┌──────────────┐  ┌───────────┐  │
  │  │   VGA/TTY   │  │   Keyboard   │  │ Commands  │  │
  │  │  vga_write  │  │  handle_kb   │  │  parser   │  │
  │  │  handle_scr │  │  keymap      │  │  clear    │  │
  │  │  handle_scrl│  │  port_io     │  │  reboot   │  │
  │  │  cursor     │  │              │  │  shutdown │  │
  │  └─────────────┘  └──────────────┘  └───────────┘  │
  │  ┌─────────────┐  ┌──────────────┐                  │
  │  │   printk    │  │     lib      │                  │
  │  │  format str │  │  string.c    │                  │
  │  │  convert    │  │  print_hdr   │                  │
  │  └─────────────┘  └──────────────┘                  │
  └─────────────────────────────────────────────────────┘
                           │
                    ┌──────▼──────┐
                    │  Hardware   │
                    │  VGA 0xB8000│
                    │  KBC 0x60  │
                    │  Ports     │
                    └─────────────┘
```

### Boot → main flow

```c
// kernel.c (simplified)
void kernel_main(void) {
    kfs k;
    kfs_initialize(&k);         // init all screens, VGA, cursor
    print_header(active_scr);   // display welcome banner
    handle_keyboard_keypress(&k); // infinite polling loop — never returns
}
```

The kernel never reaches `return` — it loops forever in `handle_keyboard_keypress()`. If it did return, `boot.s` executes `cli; hlt` to cleanly stop the CPU.

---

## 8. Cross-Compilation

### What is cross-compilation?

Cross-compilation means building code on one platform (the **host**) to run on a different platform (the **target**). For KFS-1:
- **Host:** your Linux x86_64 machine
- **Target:** bare-metal i386 (32-bit, no OS)

Your system's default GCC targets Linux user-space (`x86_64-linux-gnu`) — it links against glibc, generates ELF binaries expecting a Linux kernel, etc. None of that works for a bare-metal kernel.

### The cross-compiler toolchain

You need an `i686-elf` toolchain:

| Tool | Purpose |
|------|---------|
| `i686-elf-gcc` | C compiler targeting bare-metal i386 |
| `i686-elf-ld` | Linker for ELF i386 binaries |
| `i686-elf-as` | Assembler (GAS syntax) |
| `i686-elf-objdump` | Disassemble / inspect binaries |

### Why not just use `-m32`?

`gcc -m32` generates 32-bit code but still targets Linux — it expects:
- `libc` (printf, malloc, etc.) — you have none
- Linux system calls — you have no OS
- Standard ELF layout for a hosted environment

With `i686-elf-gcc`:
- No standard library linked by default
- No OS assumptions
- `-ffreestanding` flag tells GCC to not assume any standard library exists
- `-nostdlib` prevents automatic linking of startup files and libc

### Key compiler flags for KFS-1

```makefile
CC = i686-elf-gcc
CFLAGS = -ffreestanding     # no hosted environment (no libc)
         -O2                # optimize
         -Wall -Wextra      # all warnings
         -nostdlib           # don't link standard library
         -lgcc               # link libgcc (for compiler builtins like __divdi3)
```

### What `-ffreestanding` does

Tells the compiler:
- Don't assume standard headers exist (except `<stddef.h>`, `<stdint.h>`, `<stdbool.h>`, `<stdarg.h>`, `<float.h>`, `<limits.h>`)
- Don't assume `main()` is the entry point
- Don't optimize calls based on standard library semantics (e.g., don't replace a loop with `memcpy` unless you've provided your own)

This is why KFS-1 implements its own `strlen`, `strcmp`, `memcpy`, `memmove`, `memset` in `string.c`.

---

## 9. ELF Format

### What is ELF?

**ELF** (Executable and Linkable Format) is the standard binary format on Linux/Unix. Your cross-compiler produces ELF binaries. GRUB parses the ELF to load your kernel.

### ELF structure

```
  ┌──────────────────────────────────────────────────────┐
  │  ELF Header (52 bytes for 32-bit)                    │
  │    - Magic: 0x7F 'E' 'L' 'F'                        │
  │    - Class: 32-bit (1) or 64-bit (2)                 │
  │    - Machine: EM_386 (3) for i386                    │
  │    - Entry point: address of _start                  │
  │    - Program header table offset                     │
  │    - Section header table offset                     │
  ├──────────────────────────────────────────────────────┤
  │  Program Headers (for loading)                       │
  │    PT_LOAD segment 1:                                │
  │      - Virtual addr: 0x100000                        │
  │      - File offset:  where data is in file           │
  │      - File size:    how many bytes to copy           │
  │      - Memory size:  how much memory to allocate      │
  │      - Flags:        R (read), W (write), X (execute) │
  │    PT_LOAD segment 2: ...                            │
  ├──────────────────────────────────────────────────────┤
  │  .text    — executable code                          │
  │  .rodata  — read-only data                           │
  │  .data    — initialized globals                      │
  │  .bss     — uninitialized globals (not in file)      │
  ├──────────────────────────────────────────────────────┤
  │  Section Headers (for debugging/linking)             │
  │    - Section names, types, addresses, sizes          │
  │    - .symtab — symbol table                          │
  │    - .strtab — string table                          │
  └──────────────────────────────────────────────────────┘
```

### Segments vs Sections

| | Segments (Program Headers) | Sections (Section Headers) |
|-|---------------------------|---------------------------|
| **Used by** | Loader (GRUB) at boot time | Linker, debugger |
| **Purpose** | What to load into memory | How the binary is organized |
| **Example** | PT_LOAD: load .text+.rodata at 0x100000 | .text, .rodata, .data, .bss |

GRUB only reads **program headers**. It copies each `PT_LOAD` segment to its specified virtual address.

### Inspecting your kernel ELF

```bash
# Show ELF header
i686-elf-readelf -h kernel.bin

# Show program headers (what GRUB loads)
i686-elf-readelf -l kernel.bin

# Show sections
i686-elf-readelf -S kernel.bin

# Disassemble
i686-elf-objdump -d kernel.bin | head -50
```

### Why .bss saves space

The `.bss` section stores uninitialized globals (e.g., `screen screens[9]`). In the ELF file, `.bss` takes **zero bytes** — only its size is recorded. The loader (GRUB) allocates that much memory and zeros it. This is why large buffers should be global/static rather than stack-allocated.

---

## 10. Keyboard & I/O Ports

### I/O Port architecture

x86 has two ways to talk to hardware:

```
  Memory-Mapped I/O (MMIO)          Port-Mapped I/O (PMIO)
  ┌─────────────────────┐          ┌─────────────────────┐
  │ Uses normal memory   │          │ Separate address    │
  │ addresses            │          │ space (65536 ports)  │
  │                     │          │                     │
  │ Access: MOV          │          │ Access: IN / OUT     │
  │                     │          │                     │
  │ Example:             │          │ Example:             │
  │ VGA at 0xB8000      │          │ Keyboard at 0x60    │
  └─────────────────────┘          └─────────────────────┘
```

### Port I/O instructions (from `port_io.c`)

```c
// Read 1 byte from port
uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write 1 byte to port
void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Write 2 bytes to port (used for ACPI shutdown)
void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
```

The `volatile` keyword prevents the compiler from optimizing away or reordering these I/O operations.

### PS/2 Keyboard Controller (Intel 8042)

```
  ┌──────────────────────────────────────────────┐
  │              PS/2 Keyboard Controller         │
  ├──────────────┬───────────────────────────────┤
  │  Port 0x64   │  Status register (read)       │
  │              │  Bit 0: Output buffer full     │
  │              │  Bit 1: Input buffer full      │
  │              │  (command register on write)    │
  ├──────────────┼───────────────────────────────┤
  │  Port 0x60   │  Data register (read/write)   │
  │              │  Read: scancode from keyboard   │
  │              │  Write: data to keyboard       │
  └──────────────┴───────────────────────────────┘
```

### Scancode flow

```
  Key press 'A':
  ┌──────────┐    ┌────────────┐    ┌──────────┐    ┌──────────┐
  │ Physical │    │  Keyboard  │    │  8042    │    │   CPU    │
  │ switch   │───►│  encoder   │───►│ buffer   │───►│ inb(0x60)│
  │ closes   │    │ scancode   │    │ port 0x60│    │ = 0x1E   │
  └──────────┘    │ = 0x1E     │    └──────────┘    └──────────┘
                  └────────────┘

  Key release 'A':
  Same path but scancode = 0x1E | 0x80 = 0x9E (bit 7 set)
```

### Scancode translation in KFS-1

Two lookup tables in `keymap.c`:

```c
static const char scancode_to_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', ...
};

// Shifted version: '1'→'!', 'a'→'A', etc.
static const char scancode_to_ascii_shifted[128] = { ... };
```

### Shift / modifier handling

```
  ┌────────────────────────────────────────────────┐
  │           scan = get_scancode()                 │
  │               │                                 │
  │  scan == 0x2A (LShift press)  → shift = true   │
  │  scan == 0xAA (LShift release)→ shift = false  │
  │  scan == 0x36 (RShift press)  → shift = true   │
  │  scan == 0xB6 (RShift release)→ shift = false  │
  │  scan == 0x3A (CapsLock press)→ toggle shift   │
  │               │                                 │
  │  if shift:    │                                 │
  │    char = scancode_to_ascii_shifted[scan]       │
  │  else:                                          │
  │    char = scancode_to_ascii[scan]               │
  └────────────────────────────────────────────────┘
```

### Polling vs Interrupts

KFS-1 uses **polling** — an infinite loop that constantly checks port 0x64. The interrupt-driven alternative would be:

```
  POLLING (KFS-1)                    INTERRUPTS (not implemented)
  ┌────────────────────┐             ┌────────────────────┐
  │ while(1) {         │             │ // CPU is idle     │
  │   if (inb(0x64)&1) │             │ // ...             │
  │     scancode =     │             │ // KEY PRESSED!    │
  │       inb(0x60);   │             │ // IRQ1 fires      │
  │   // else: wasted  │             │ // CPU jumps to    │
  │   // cycle         │             │ // ISR handler     │
  │ }                  │             │ keyboard_isr() {   │
  │                    │             │   inb(0x60);       │
  │ CPU: 100% busy     │             │   process key;     │
  │ always             │             │   outb(0x20,0x20); │
  │                    │             │ }                  │
  │                    │             │ CPU: 0% when idle   │
  └────────────────────┘             └────────────────────┘
```

---

## 11. Memory Management

### Physical memory

Physical memory is the actual RAM chips in your computer. The CPU sees it as a flat array of bytes addressed from 0 to however much RAM is installed.

### Physical frame management

RAM is divided into **frames** (4 KB each, matching page size). The kernel must track which frames are free and which are in use.

```
  Frame allocator strategies:

  1. BITMAP
     ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
     │1│1│0│0│1│0│0│0│1│1│0│0│  1 = used, 0 = free
     └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘
     + Simple, compact (1 bit per 4 KB frame)
     - Finding free frame: O(n) scan
     - For 4 GB RAM: 1 MB bitmap

  2. FREE LIST
     head → [frame 42] → [frame 17] → [frame 99] → NULL
     + O(1) allocate and free
     - No way to allocate contiguous frames easily
     - Each node stored inside the free frame itself

  3. BUDDY ALLOCATOR
     Split memory into power-of-2 blocks. When allocating,
     split the smallest sufficient block. When freeing,
     merge with adjacent "buddy" if it's also free.

     ┌───────────────────────────────────────┐  Level 3: 32 KB
     ├───────────────────┬───────────────────┤  Level 2: 16 KB
     ├─────────┬─────────┼─────────┬─────────┤  Level 1: 8 KB
     ├────┬────┼────┬────┼────┬────┼────┬────┤  Level 0: 4 KB
     │ U  │ U  │ F  │ F  │ U  │ F  │ F  │ F  │  U=used, F=free
     └────┴────┴────┴────┴────┴────┴────┴────┘

     + O(log n) allocate/free
     + Good for avoiding fragmentation
     - Internal fragmentation (7 KB request → 8 KB block)
```

### What you need for KFS-1

KFS-1 doesn't implement dynamic memory management — all buffers (screens, history, command buffers) are statically allocated as globals in `.bss`. A future KFS might need:
- Physical frame allocator (bitmap or buddy)
- `kmalloc` / `kfree` for kernel heap
- Page tables for virtual memory

---

## 12. Virtual Memory & Paging

### Why virtual memory?

Without virtual memory, every program sees raw physical addresses. Problems:
- Programs can overwrite each other's memory
- Programs must be loaded at fixed addresses
- No way to use more memory than physically available

With virtual memory, each process gets its own **virtual address space** that's independently mapped to physical memory.

### x86 paging (32-bit, no PAE)

```
  Virtual address (32 bits):
  ┌──────────────┬──────────────┬────────────────────┐
  │  PD index    │  PT index    │     Offset         │
  │  bits 31-22  │  bits 21-12  │    bits 11-0       │
  │  (10 bits)   │  (10 bits)   │    (12 bits)       │
  └──────┬───────┴──────┬───────┴────────┬───────────┘
         │              │                │
         ▼              │                │
  ┌──────────────┐      │                │
  │ Page Directory│      │                │
  │ (1024 entries)│      │                │
  │ CR3 → base   │      │                │
  │              │      ▼                │
  │ [PD index]──────► ┌──────────────┐   │
  │              │    │ Page Table    │   │
  └──────────────┘    │ (1024 entries)│   │
                      │              │   │
                      │ [PT index]──────► Physical page base
                      │              │   + Offset
                      └──────────────┘   = Physical address
```

### Page Table Entry format

```
  31                    12 11  9 8 7 6 5 4 3 2 1 0
  ┌────────────────────┬─────┬─┬─┬─┬─┬─┬─┬─┬─┬─┐
  │  Physical page     │Avail│G│0│D│A│C│W│U│R│P│
  │  frame address     │     │ │ │ │ │D│T│/│/│ │
  │  (20 bits)         │     │ │ │ │ │ │ │S│W│ │
  └────────────────────┴─────┴─┴─┴─┴─┴─┴─┴─┴─┴─┘

  P  = Present (1 = page is in memory)
  R/W = Read/Write (0 = read-only, 1 = writable)
  U/S = User/Supervisor (0 = kernel only, 1 = user accessible)
  A  = Accessed (set by CPU when page is read)
  D  = Dirty (set by CPU when page is written)
```

### Enabling paging

```c
// 1. Allocate and fill page directory & page tables
uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_table[1024] __attribute__((aligned(4096)));

// 2. Identity-map first 4 MB (page table covers 4 MB)
for (int i = 0; i < 1024; i++) {
    page_table[i] = (i * 0x1000) | 0x03;  // present + R/W
}
page_directory[0] = ((uint32_t)page_table) | 0x03;

// 3. Load page directory into CR3
asm volatile("mov %0, %%cr3" : : "r"(page_directory));

// 4. Enable paging (set PG bit in CR0)
uint32_t cr0;
asm volatile("mov %%cr0, %0" : "=r"(cr0));
cr0 |= 0x80000000;
asm volatile("mov %0, %%cr0" : : "r"(cr0));
```

### Page fault handling

When the CPU accesses an address whose page table entry has `P=0` (not present), it raises **INT 14** (page fault). The **CR2** register contains the faulting address. The error code on the stack tells you:
- Bit 0: was the page present? (0 = not mapped, 1 = protection violation)
- Bit 1: was it a write? (0 = read, 1 = write)
- Bit 2: was it user mode? (0 = kernel, 1 = user)

---

## 13. QEMU & KVM

### What is QEMU?

QEMU (Quick EMUlator) is a machine emulator that can run your kernel binary on your development machine. It emulates an entire x86 PC: CPU, RAM, VGA, keyboard, disk controller, PIC, PIT, etc.

### Running KFS-1 in QEMU

```bash
# Boot from ISO (how KFS-1 does it)
qemu-system-i386 -cdrom kernel.iso

# Boot kernel directly (multiboot protocol)
qemu-system-i386 -kernel kernel.bin

# With options
qemu-system-i386 -cdrom kernel.iso \
    -m 128M \          # 128 MB RAM
    -no-reboot \       # halt on triple fault instead of rebooting
    -serial stdio      # redirect serial to terminal
```

### What is KVM?

**KVM** (Kernel-based Virtual Machine) is a Linux kernel module that lets QEMU use hardware-assisted virtualization (Intel VT-x / AMD-V). Instead of emulating every instruction, KVM lets the guest code run directly on the real CPU in a sandboxed mode. The hypervisor only intercepts privileged operations (I/O, page table changes, etc.).

```
  Without KVM (pure emulation):
  ┌─────────────────────┐
  │  Guest instruction   │
  │  "mov eax, 42"      │   ──► QEMU translates to host code
  │                     │       (TCG - Tiny Code Generator)
  │  SLOW (~10-100x)    │
  └─────────────────────┘

  With KVM (hardware virtualization):
  ┌─────────────────────┐
  │  Guest instruction   │
  │  "mov eax, 42"      │   ──► Runs DIRECTLY on real CPU
  │                     │       (VMENTER / VMEXIT on I/O)
  │  FAST (~1-2x native)│
  └─────────────────────┘
```

### Enabling KVM

```bash
# Check if hardware virtualization is available
egrep -c '(vmx|svm)' /proc/cpuinfo   # > 0 means supported

# Check if KVM module is loaded
lsmod | grep kvm

# Run with KVM
qemu-system-i386 -enable-kvm -cdrom kernel.iso
```

### QEMU flags cheat sheet

| Flag | What it does |
|------|-------------|
| `-cdrom file.iso` | Attach ISO as CD-ROM (boot from it) |
| `-kernel file.bin` | Load multiboot kernel directly |
| `-m SIZE` | Set guest RAM (default 128M) |
| `-enable-kvm` | Use KVM for hardware acceleration |
| `-no-reboot` | Stop instead of reboot on triple fault |
| `-no-shutdown` | Stop instead of shutdown on ACPI off |
| `-s` | Start GDB server on `localhost:1234` |
| `-S` | Freeze CPU at startup (use with `-s` to debug from first instruction) |
| `-d int` | Log all interrupts to stderr |
| `-d cpu_reset` | Log CPU state on reset |
| `-D logfile` | Write debug output to file instead of stderr |
| `-serial stdio` | Redirect guest serial port to host terminal |
| `-monitor stdio` | Open QEMU monitor in terminal (type `info registers`, `xp /10x 0xB8000`, etc.) |
| `-vga std` | Standard VGA adapter |

### Debugging with GDB

```bash
# Terminal 1: start QEMU paused
qemu-system-i386 -cdrom kernel.iso -s -S -no-reboot

# Terminal 2: attach GDB
gdb kernel.bin
(gdb) target remote :1234    # connect to QEMU
(gdb) break kernel_main      # breakpoint at kernel entry
(gdb) continue                # resume execution
(gdb) info registers          # show all registers
(gdb) x /20x 0xB8000         # examine VGA memory
(gdb) stepi                    # single-step one instruction
```

### QEMU Monitor commands

Press `Ctrl+Alt+2` in QEMU window to open the monitor (or use `-monitor stdio`):

| Command | What it does |
|---------|-------------|
| `info registers` | Dump CPU registers |
| `info mem` | Show memory mappings |
| `xp /10x 0xB8000` | Examine 10 hex words at physical address |
| `gdbserver` | Enable GDB remote debugging |
| `quit` | Exit QEMU |
| `system_reset` | Reset the guest |
| `stop` / `cont` | Pause / resume guest |
