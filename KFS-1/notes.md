### Concepts
### 1️⃣ **Boot & Multiboot (Foundation)**
|Knowledge Area|What You Need|
|---|---|
|Multiboot header|32-bit magic number (0x1BADB002), flags, checksum in ASM|
|GRUB|Install on virtual image, load your kernel ELF|
|Linker script|Place kernel at 1MB (0x100000), define entry point|
|Boot ASM|Set up stack, call `kernel_main`, halt on return|

---

### 2️⃣ **VGA Text Mode (Display)**
|Feature|Implementation|
|---|---|
|Basic output|Write to 0xB8000 (char + attribute per cell)|
|Colors|Attribute byte: FG (bits 0-3), BG (bits 4-6), blink (bit 7)|
|Cursor|I/O ports 0x3D4/0x3D5 to move hardware cursor|
|Scroll|Copy lines up with `memcpy`, clear last line|

---

### 3️⃣ **printk / printf (Debug Output)**
|Component|Notes|
|---|---|
|Variable arguments|Use `__builtin_va_start`, `__builtin_va_arg` (no libc!)|
|Format specifiers|At minimum: `%s`, `%d`, `%x`, `%c`, `%%`|
|Integer to string|Implement `itoa()` for decimal/hex|
|Kernel library|You'll need `strlen`, `memcpy`, `memset`|

---

### 4️⃣ **Keyboard Input (Interrupts)**
|Component|Notes|
|---|---|
|IDT|Interrupt Descriptor Table (256 entries)|
|PIC|Programmable Interrupt Controller (remap IRQs to 0x20+)|
|IRQ1 handler|Read scan code from port 0x60, convert to ASCII|
|Keyboard buffer|Circular buffer for typed characters|
|Modifier keys|Track Shift, Ctrl, Alt state|

---

### 5️⃣ **Multiple Screens + Shortcuts**
|Feature|Implementation|
|---|---|
|Virtual terminals|Array of screen buffers (each 80×25×2 = 4000 bytes)|
|Active screen|Pointer to current buffer; copy to VGA on switch|
|Keyboard shortcuts|Detect Ctrl+1, Ctrl+2, etc. or Alt+F1, Alt+F2|
|Per-screen state|Each terminal tracks its own cursor position, color|


## Implementation Steps & Pseudo-code

### Overview: What Happens When the Kernel Runs
```
Power On
  → BIOS/UEFI initializes hardware          (not our code)
    → GRUB reads multiboot header            (not our code, but we write the header so GRUB recognizes our binary)
      → GRUB loads kernel ELF into memory    (not our code)
        → CPU jumps to our ASM entry point   (boot.s)
          → ASM sets up stack, calls C       (boot.s → kernel_main)
            → kernel_main() runs forever     (kernel.c)
```

https://wiki.osdev.org/Beginner_Mistakes
> Common examples include being too lazy to use a Cross-Compiler, not understanding Real Mode, Unreal Mode, Protected Mode, and Long Mode and jumping far too fast from one to the other without first understanding how to gather all of the vital configuration and use all of its capabilities fully (especially detecting basic capabilities with the BIOS during boot and initialization), relying on BIOS calls that are not standard, not learning to write hardware drivers in your own OS and under Windows and Linux for greatest convenience, exposing test capabilities globally, using flat binaries instead of ELF, not learning about executable, archive, graphics, document and other file formats and compression algorithms, and so on. Experienced developers use the superior alternatives for a reason, which you might not understand yet. Experienced developers choose to use the inferior method in some cases, but that's because they can carefully analyze whether it is appropriate, and they know when not to use it. As a beginner or intermediate developer you will likely not know these methods and technologies well enough to reason whether the inferior solution is good enough for you. Remember that if you oppose a method, you should know it well enough to know everything that is wrong (and right) with it. Either way, laziness and ignorance only leads to trouble down the road. When in doubt, pick what appears to be the conservative choice rather than the simpler. 

```c
/*
** Boot pipeline:
**
** 1. Makefile compiles our kernel code (.o) using our linker script to form a binary file -> kernel.bin
**    -> https://wiki.osdev.org/Linker_Scripts
**    -> https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
**
** 2. GRUB loads the binary thanks to the Multiboot header embedded in boot.s and creates an iso image
**    -> https://wiki.osdev.org/GRUB
**    -> https://wiki.osdev.org/Multiboot
**
** 3. QEMU/KVM emulates the hardware and runs the iso image containing our kernel
**    -> https://wiki.osdev.org/QEMU
**
** Reference: https://wiki.osdev.org/Bare_Bones
*/

/*
You (developer)                        The machine (at boot time)
─────────────                          ──────────────────────────
1. Write boot.s with multiboot header
2. Compile + link → kernel.bin
3. grub-mkrescue → kfs1.iso
4. Give ISO to QEMU
                                       5. BIOS starts
                                       6. GRUB loads from the ISO
                                       7. GRUB scans kernel.bin for
                                          the magic number 0x1BADB002
                                       8. GRUB finds it → "OK, this
                                          is a valid multiboot kernel"
                                       9. GRUB loads kernel.bin into
                                          memory at the right address
                                       10. GRUB jumps to _start
                                       11. Your boot.s runs → calls
                                           kernel_main()


Multiboot header (in boot.s) = "Dear GRUB, I'm a real kernel, please boot me" → runtime
Linker script (linker.ld) = "Dear linker, arrange my binary like this in memory" → build time
GRUB does NOT compile anything. It's a bootloader that loads your pre-built binary at boot time.
grub-mkrescue just packages the binary + GRUB into a bootable ISO.
*/
```


---

### STEP 0 — Project Structure & Build System
> Concepts: [[Compilers]], [[Linker (ld)]], [[Cross-compiling]]

```
project/
├── Makefile                    ← Compiles ASM + C, links into kernel.bin, builds ISO
├── src/
│   ├── kernel.c                ← kernel_main()
│   ├── vga.c / vga.h           ← Screen output
│   ├── string.c / string.h     ← strlen, memcpy, memset, strcmp
│   ├── printk.c / printk.h     ← printk() with format specifiers
│   ├── idt.c / idt.h           ← Interrupt Descriptor Table
│   ├── pic.c / pic.h           ← PIC remapping
│   ├── keyboard.c / keyboard.h ← Keyboard driver
│   ├── terminal.c / terminal.h ← Multi-screen manager
│   └── io.asm / io.h           ← inb() / outb() port I/O
└── boot/
│   ├── boot.s                  ← Multiboot header + entry point
    ├── grub.cfg                ← GRUB menu configuration
    └── linker.ld               ← Memory layout (kernel at 0x100000)
```

---

### STEP 1 — Multiboot Header & Boot ASM
> Concepts: [[Booting]], [[Bootloader (GRUB ex)]], [[Assembly language]], [[X86]]

**File: `boot.s`**

**What it does**: Provides the multiboot header so GRUB recognizes our binary,
then sets up a stack and calls `kernel_main`.


**What to verify:**
- [ ] GRUB boots and jumps to our code
- [ ] `kernel_main()` is called successfully

---

### STEP 2 — Linker Script
> Concepts: [[Linker (ld)]]

**File: `linker.ld`**

**What it does**: Tells the linker WHERE to place each section in memory.
GRUB loads kernels at 1MB (0x100000) — below that is BIOS/hardware memory.

---

### STEP 3 — Minimal Kernel (Display "42")
> Concepts: [[VGA connector]], [[Kernel (operating system)]]

**File: `kernel.c`**

**What it does**: First C function called. Clears screen, prints "42".

**What to verify:**
- [ ] "42" appears on screen after boot
- [ ] Kernel doesn't crash or reboot

---

### STEP 4 — Port I/O (ASM helpers)
> Concepts: [[Assembly language]], [[CPU register]]

**File: `io.asm`**

**What it does**: Provides `outb(port, value)` and `inb(port)` functions.
These are CPU instructions that talk directly to hardware (VGA cursor, keyboard, PIC).
C cannot do this — only ASM has the `in`/`out` instructions.

```
function outb(port, value):
    ; Send 1 byte to a hardware I/O port
    Load port into DX register
    Load value into AL register
    Execute OUT instruction           ; CPU sends AL to port DX
    Return

function inb(port):
    ; Read 1 byte from a hardware I/O port
    Load port into DX register
    Execute IN instruction            ; CPU reads from port DX into AL
    Return AL
```

---

### STEP 5 — Kernel String Library
> Concepts: [[Kernel (operating system)]]

**File: `string.c`**

**What it does**: Basic utility functions. No libc exists — we write everything.

---

### STEP 6 — VGA Terminal Driver (Colors, Scroll, Cursor)
> Concepts: [[VGA connector]], [[Device driver]]

**File: `vga.c`**

**What it does**: Full terminal output with colors, scrolling, and hardware cursor.

**What to verify:**
- [ ] Text wraps at column 80
- [ ] '\n' moves to next line
- [ ] Screen scrolls when row 25 is reached
- [ ] Hardware cursor blinks at correct position
- [ ] Colors work (try different fg/bg combos)

---

### STEP 7 — printk (Formatted Debug Output)
> Concepts: [[VGA connector]] (freestanding headers: `<stdarg.h>`)

**File: `printk.c`**

**What it does**: `printk("Hello %s, val=%d hex=%x", name, 42, 0xFF)`


**What to verify:**
- [ ] `printk("Hello %s\n", "World")` → "Hello World"
- [ ] `printk("Dec: %d Hex: %x\n", 255, 255)` → "Dec: 255 Hex: 0xFF"
- [ ] `printk("100%%\n")` → "100%"

---

### STEP 8 — IDT & PIC (Interrupt System)
> Concepts: [[Keyboard technology]], [[I386]], [[Assembly language]]

**File: `idt.c` + `pic.c`**

**What it does**: Sets up the interrupt infrastructure so hardware (keyboard) can
notify the CPU. Without this, pressing keys does nothing.

```
; ══════════════════════════════════════════════
; HOW INTERRUPTS FLOW:
;
; Keyboard press
;   → Keyboard controller sends electrical signal (IRQ1)
;     → PIC receives IRQ1, translates to interrupt number
;       → PIC signals CPU: "interrupt 0x21!"
;         → CPU stops current code
;           → CPU looks up IDT[0x21] to find handler address
;             → CPU jumps to our keyboard_handler function
;               → We read scan code, process it, send EOI
;                 → CPU resumes normal code
; ══════════════════════════════════════════════

; ── PIC Initialization ────────────────────────
; The PIC chips route hardware IRQs to CPU interrupts.
; Problem: Default IRQ 0-7 map to INT 0x00-0x07,
;          which CONFLICT with CPU exception handlers!
; Solution: Remap IRQs to start at INT 0x20

; ── IDT Setup ─────────────────────────────────
; IDT = array of 256 entries, each 8 bytes
; Each entry says: "For interrupt N, jump to address X"

struct idt_entry:
    offset_low   : 16 bits    ; Lower 16 bits of handler address
    selector     : 16 bits    ; Code segment selector (0x08)
    zero         : 8 bits     ; Always 0
    type_attr    : 8 bits     ; Gate type + privilege (0x8E = interrupt gate)
    offset_high  : 16 bits    ; Upper 16 bits of handler address

idt_entries[256]               ; Full IDT table
idt_pointer = { size: 256*8-1, address: &idt_entries }


; ── ASM Interrupt Wrapper ─────────────────────
; CPU doesn't save all registers on interrupt.
; We must save them, call C handler, restore them.

keyboard_handler_wrapper:         ; (in ASM file)
    PUSHA                          ; Save all general registers
    CALL keyboard_handler          ; Call our C function
    POPA                           ; Restore all registers
    IRET                           ; Return from interrupt
                                   ; (different from RET — restores flags + CS:EIP)
```

**What to verify:**
- [ ] CPU doesn't triple-fault after `sti` (interrupts enabled)
- [ ] PIC is properly remapped (no conflict with exceptions)

---

### STEP 9 — Keyboard Driver
> Concepts: [[Keyboard technology]], [[Keystroke logging]]

**File: `keyboard.c`**

**What it does**: Reads scan codes from keyboard, converts to ASCII,
handles modifiers (Shift, Ctrl, Alt), detects shortcuts.

```
; ── State ─────────────────────────────────────
shift_pressed = false
ctrl_pressed  = false
alt_pressed   = false

; ── Scan Code Lookup Tables ───────────────────
; Scan code set 1 (what QEMU emulates)
; Index = scan code, Value = ASCII character

lowercase_map[128]:
    [0x02]='1' [0x03]='2' [0x04]='3' [0x05]='4' ...
    [0x10]='q' [0x11]='w' [0x12]='e' [0x13]='r' ...
    [0x1E]='a' [0x1F]='s' [0x20]='d' [0x21]='f' ...
    [0x2C]='z' [0x2D]='x' [0x2E]='c' [0x2F]='v' ...
    [0x1C]='\n' [0x39]=' ' [0x0E]='\b' [0x0F]='\t'

uppercase_map[128]:
    same positions but 'Q','W','E','R' and '!','@','#','$' ...

; ── Interrupt Handler (called by ASM wrapper) ─
function keyboard_handler():
    scancode = inb(0x60)           ; Read scan code from keyboard port

    ; ── Key RELEASE (bit 7 set) ───
    if scancode & 0x80:
        released_key = scancode & 0x7F     ; Strip release bit
        if released_key == 0x2A or 0x36:   shift_pressed = false
        if released_key == 0x1D:           ctrl_pressed  = false
        if released_key == 0x38:           alt_pressed   = false
        outb(0x20, 0x20)                   ; Send EOI to PIC
        return

    ; ── Modifier Key PRESS ────────
    if scancode == 0x2A or 0x36:   shift_pressed = true;  goto eoi
    if scancode == 0x1D:           ctrl_pressed  = true;  goto eoi
    if scancode == 0x38:           alt_pressed   = true;  goto eoi

    ; ── Screen Switch: Alt+1 through Alt+4 ────
    if alt_pressed:
        if scancode == 0x02:  switch_terminal(0);  goto eoi   ; Alt+1
        if scancode == 0x03:  switch_terminal(1);  goto eoi   ; Alt+2
        if scancode == 0x04:  switch_terminal(2);  goto eoi   ; Alt+3
        if scancode == 0x05:  switch_terminal(3);  goto eoi   ; Alt+4

    ; ── Normal Key PRESS ──────────
    ascii = shift_pressed ? uppercase_map[scancode] : lowercase_map[scancode]

    if ascii != 0:
        terminal_handle_input(ascii)       ; Send to active terminal

    eoi:
    outb(0x20, 0x20)              ; Send End-Of-Interrupt to PIC
                                   ; Without this, PIC won't send more interrupts!
```

**What to verify:**
- [ ] Pressing keys produces correct characters
- [ ] Shift changes case
- [ ] Enter produces newline
- [ ] Backspace deletes last character
- [ ] Alt+1/2/3/4 switches screens

---

### STEP 10 — Multiple Virtual Terminals
> Concepts: [[VGA connector]], [[Shell (computing)]]

**File: `terminal.c`**

**What it does**: Manages 4 independent screens, each with its own buffer,
cursor position, and color. Only one is visible at a time.

```
; ── Data Structures ───────────────────────────
NUM_TERMINALS = 4

struct terminal:
    buffer[VGA_WIDTH * VGA_HEIGHT]     ; 2000 uint16_t entries (4000 bytes)
    cursor_x                            ; Column position
    cursor_y                            ; Row position
    color                               ; Current text color

terminals[NUM_TERMINALS]                ; Array of 4 terminals
active_terminal = 0                     ; Index of currently displayed terminal

; ── Initialization ────────────────────────────
function terminals_init():
    for i = 0 to NUM_TERMINALS - 1:
        terminals[i].cursor_x = 0
        terminals[i].cursor_y = 0
        terminals[i].color = 0x0F       ; White on black

        ; Clear buffer
        fill terminals[i].buffer with spaces + default color

        ; Show a header on each terminal
        write "=== Terminal {i+1} ===" into terminals[i].buffer

    ; Display terminal 0
    active_terminal = 0
    copy terminals[0].buffer to VGA memory (0xB8000)
    update_cursor(0, 0)

; ── Switching Terminals ───────────────────────
function switch_terminal(index):
    if index == active_terminal: return          ; Already here
    if index >= NUM_TERMINALS: return            ; Invalid

    ; Save current VGA screen into active terminal's buffer
    memcpy(terminals[active_terminal].buffer, VGA_BUFFER, 4000)
    ; (cursor_x/y are already tracked in the struct)

    ; Switch
    active_terminal = index

    ; Load new terminal's buffer to VGA
    memcpy(VGA_BUFFER, terminals[active_terminal].buffer, 4000)

    ; Restore cursor position
    update_cursor(
        terminals[active_terminal].cursor_x,
        terminals[active_terminal].cursor_y
    )

; ── Input Handling (echo typed chars) ─────────
function terminal_handle_input(ascii_char):
    term = &terminals[active_terminal]

    if ascii_char == '\b':                       ; Backspace
        if term.cursor_x > 0:
            term.cursor_x--
            write ' ' at (cursor_x, cursor_y) in VGA buffer
            write ' ' at (cursor_x, cursor_y) in term.buffer

    else if ascii_char == '\n':                  ; Enter
        term.cursor_x = 0
        term.cursor_y++

    else:                                        ; Regular character
        write char at (cursor_x, cursor_y) in VGA buffer
        write char at (cursor_x, cursor_y) in term.buffer
        term.cursor_x++

    ; Handle wrap
    if term.cursor_x >= VGA_WIDTH:
        term.cursor_x = 0
        term.cursor_y++

    ; Handle scroll
    if term.cursor_y >= VGA_HEIGHT:
        scroll both VGA buffer AND term.buffer
        term.cursor_y = VGA_HEIGHT - 1

    update_cursor(term.cursor_x, term.cursor_y)
```

**What to verify:**
- [ ] Alt+1 through Alt+4 switches screens
- [ ] Each screen keeps its own content
- [ ] Typing on screen 1, switching to 2, switching back → text still there
- [ ] Cursor is at correct position after switch

---


### Checklist Summary

| Step | Feature | Status |
|------|---------|--------|
| 0 | Project structure + Makefile | ☐ |
| 1 | boot.asm (multiboot + stack + call C) | ☐ |
| 2 | linker.ld (kernel at 1MB) | ☐ |
| 3 | Minimal kernel (display "42") | ☐ |
| 4 | I/O ports (inb/outb in ASM) | ☐ |
| 5 | String library (strlen, memcpy, memset) | ☐ |
| 6 | VGA driver (colors, scroll, cursor) | ☐ |
| 7 | printk (format strings) | ☐ |
| 8 | IDT + PIC (interrupt system) | ☐ |
| 9 | Keyboard driver (scan code → ASCII) | ☐ |
| 10 | Multiple terminals (Alt+1/2/3/4) | ☐ |
| 11 | Final kernel_main integration | ☐ |


____

**Official Documentation**
- [OSDev Wiki](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - **The bible** for OS development
    - [Bare Bones Tutorial](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - Minimal kernel in C
    - [Meaty Skeleton](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - Proper project structure
    - [GCC Cross-Compiler](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - Building i686-elf-gcc
    - [VGA Text Mode](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html)
    - [PS/2 Keyboard](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html)
    - [IDT](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - Interrupt Descriptor Table
    - [PIC](vscode-file://vscode-app/usr/share/code/resources/app/out/vs/code/electron-browser/workbench/workbench.html) - Programmable Interrupt Controller

 **Multiboot Specification**
- [Multiboot 1 Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html) - GRUB boot protocol

 **Linker Scripts**
- [GNU LD Manual](https://sourceware.org/binutils/docs/ld/Scripts.html)

 **x86 Architecture**
- [Intel SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) - Official x86 reference (huge, use as reference)
- [x86 Instruction Reference](https://www.felixcloutier.com/x86/) - Searchable version


___

https://youtu.be/sp3mMwo3PO0?si=9sjSHFgKwgb7boJg -  How Your Code Really Controls Hardware 

https://medium.com/@jackiesingh11/kernel-and-shell-understanding-the-core-concepts-of-operating-systems-8f881925f1b2

https://youtu.be/uTNCVRxuP_c?si=IrEhRqrdSxNVz4I6 -  How computers understand programs: From transistors to a CPU (1/3) 

https://youtu.be/HjneAhCy2N4?si=s81thla4zKhQtSmR -  HOW TRANSISTORS RUN CODE? 

https://youtu.be/GtVDTp826DE?si=WDlgZqANrWh0Pk97 -  Architecture du processeur expliquée 

https://youtu.be/8afhcpiMh24?si=TSw1jUwx2BY9F1Mz -  Noyau dans le système d'exploitation : le pouvoir secret de chaque conception de système informatique

https://youtu.be/QZwneRb-zqA?si=rdflYh0VjsXQ3Say -  Exploring How Computers Work 

https://www.youtube.com/watch?v=16zrEPOsIcI&list=PL6rx9p3tbsMuzMea5pSGb2JvQfe42eYYS - How are microchips made? playlist

https://youtu.be/tadUeiNe5-g?si=mQzXSmlcIDQHf-Vp -  Comment les processeurs interagissent avec de nombreux appareils différents 

https://youtu.be/h-NM1xSSzHQ?si=V3pLUn62Mdy3Z_-z - Comment fonctionnent les claviers d'ordinateur ? 🤔⌨⌨🛠 

https://youtu.be/5f3NJnvnk7k?si=c3_WMCteM96muWZE -  How a Computer Works - from silicon to apps 

https://www.youtube.com/watch?v=X82l37rAnP8&list=PLsVorjAex5gtQkIUrtsWHs_h3OU29-euB - [redacted] - OS playlist

https://www.youtube.com/watch?v=O5nskjZ_GoI&list=PL8dPuuaLjXtNlUrzyH5r6jN9ulIgZBpdo&index=3 - computer science playlist

https://www.youtube.com/watch?v=6zPBNEDKbpk&list=PL2EF13wm-hWAglI8rRbdsCPq_wRpYvQQy - Operating System playlist

https://en.wikipedia.org/wiki/I386


https://youtu.be/xFrMXzKCXIc?si=0IZaze6zVi10WE6O - Making Simple Bootloader using x86 Assembly 

https://youtu.be/tVd3OUodNDw?si=FwFK3_kJLna1WQLm - 80386 DX Build, Ep. 19 - Interrupts, Keyboard, Mouse 


