# 01 — Boot Process

## What is it?
The boot process is the chain of events that brings the kernel from a file on disk to running code in memory. We use **GRUB** as the bootloader, a **Multiboot-compliant** assembly stub as the entry point, and a **linker script** to lay out the binary in memory.

## How it works

### The chain of events

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   BIOS /     │     │    GRUB      │     │   boot.s     │     │  kernel.c    │
│   UEFI       │────▶│  bootloader  │────▶│  (_start)    │────▶│ kernel_main()│
│   firmware   │     │              │     │              │     │              │
└──────────────┘     └──────────────┘     └──────────────┘     └──────────────┘
  Power-on            Finds kernel.bin     Sets up stack        Our C code
  self-test           via grub.cfg         Zeros .bss           runs here
                      Loads it at 2MB      Calls kernel_main
```

### Step 1 — GRUB configuration (`grub.cfg`)

GRUB reads this config to know **what** to boot and **where** the kernel binary is:

```
set timeout=0        ← boot immediately, no menu delay
set default=0        ← select first (and only) entry

menuentry "KFS-1" {
    multiboot /boot/kernel.bin   ← load kernel using Multiboot protocol
    boot
}
```

### Step 2 — Multiboot header (`boot.s`)

GRUB scans the **first 8 KB** of the binary looking for a magic signature. We place it in a `.multiboot` section:

```
              Multiboot Header (12 bytes)
         ┌─────────────┬───────────────────┐
  Byte 0 │ MAGIC       │ 0x1BADB002        │  ← GRUB looks for this
         ├─────────────┼───────────────────┤
  Byte 4 │ FLAGS       │ ALIGN | MEMINFO   │  ← Request alignment + memory info
         ├─────────────┼───────────────────┤
  Byte 8 │ CHECKSUM    │ -(MAGIC + FLAGS)  │  ← Must sum to 0 with MAGIC+FLAGS
         └─────────────┴───────────────────┘
```

FLAGS bits:
- **Bit 0 (ALIGN)**: Ask GRUB to page-align all kernel sections (4 KB boundaries)
- **Bit 1 (MEMINFO)**: Ask GRUB to pass memory map info to us

### Step 3 — Stack setup and BSS zeroing (`boot.s`)

Once GRUB jumps to `_start`, we're in **32-bit protected mode** with no stack:

```
         Memory after GRUB loads kernel
         ┌─────────────────────────┐  High address
         │                         │
         │  stack_top ──────────── │ ◄── ESP points here
         │  ▲ grows downward       │
         │  │ 16 KB reserved       │
         │  stack_bottom ───────── │
         ├─────────────────────────┤
         │  .bss (uninitialized)   │ ◄── zeroed by rep stosb
         │  __bss_start .. end     │
         ├─────────────────────────┤
         │  .data (initialized)    │
         ├─────────────────────────┤
         │  .rodata (read-only)    │
         ├─────────────────────────┤
         │  .text (code)           │
         │  .multiboot header      │
         ├─────────────────────────┤
         │                         │
         └─────────────────────────┘  0x200000 (2 MB)
```

What `_start` does:
1. `mov $stack_top, %esp` — point the stack register to our 16 KB stack
2. Zero the `.bss` section — C expects uninitialized globals to be 0
3. `call kernel_main` — jump into our C code
4. `cli; hlt; jmp` — if `kernel_main` ever returns, disable interrupts and halt forever

### Step 4 — Linker script (`linker.ld`)

The linker script tells the linker **where** to place each section in the final binary:

```
    ENTRY(_start)            ← first instruction to execute

    . = 2M                   ← load everything starting at 2 MB
                               (avoids conflicts with legacy BIOS area below 1 MB)

    Section layout:
    ┌───────────────────┐  2 MB
    │ .text             │  Code + multiboot header (4K aligned)
    │   *(.multiboot)   │  ◄── must be first (GRUB requirement)
    │   *(.text)        │
    ├───────────────────┤
    │ .rodata           │  String constants, const data (4K aligned)
    ├───────────────────┤
    │ .data             │  Initialized globals (4K aligned)
    ├───────────────────┤
    │ .bss              │  Uninitialized data + stack (4K aligned)
    │  __bss_start      │  ◄── used by boot.s to zero this section
    │  *(COMMON)        │
    │  *(.bss)          │
    │  __bss_end        │
    └───────────────────┘
```

Each section is aligned to **4 KB (BLOCK(4K))** — this is important for future paging support.

## Files involved

| File | Role |
|------|------|
| `boot/grub.cfg` | GRUB menu configuration |
| `boot/boot.s` | Assembly entry point, multiboot header, stack setup |
| `boot/linker.ld` | Memory layout of the kernel binary |
| `src/kernel.c` | `kernel_main()` — the C entry point |
