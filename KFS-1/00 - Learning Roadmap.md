# KFS-1 Learning Roadmap
> From Transistors to Kernel — A bottom-up approach

This roadmap is designed to build your understanding progressively. Each level builds upon the previous one.

---

## 🔴 Level 0: The Physical Foundation
*"How does electricity become computation?"*

### Concepts:
1. **[[Transistor]]** — The fundamental building block (electronic switch)
2. **[[Logic Gates]]** — AND, OR, NOT, XOR — how transistors combine to make decisions
3. **Binary Arithmetic** — How numbers and data are represented (0s and 1s)
4. **Clock Cycles** — The heartbeat of a computer (synchronization)

### Key Questions to Answer:
- [ ] How does a transistor act as a switch?
- [ ] How do logic gates combine to perform addition?
- [ ] What is a clock signal and why is it necessary?

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
- [ ] What are the main x86 registers and their purposes?
- [ ] How does the CPU know what instruction to execute next?
- [ ] What is the difference between real mode and protected mode?
- [ ] What is the role of the stack pointer (ESP) and base pointer (EBP)?

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
- [ ] How do you write a basic assembly program?
- [ ] What is the difference between `MOV` and `LEA`?
- [ ] How do interrupts work and why are they important?
- [ ] How does the stack work during function calls?

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
- [ ] What happens when you press the power button?
- [ ] What is the multiboot specification and why use it?
- [ ] How does GRUB load your kernel into memory?
- [ ] What is the difference between real mode and protected mode?

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
- [ ] What does a linker script do and why is it necessary?
- [ ] What is the GDT and how do you set it up?
- [ ] How do you handle CPU exceptions (divide by zero, page fault)?
- [ ] What is the difference between a monolithic and microkernel?

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
- [ ] How do you write a character to VGA text mode?
- [ ] How do you read a key from the keyboard?
- [ ] What is the difference between polling and interrupts?
- [ ] How do you handle the PIC to receive keyboard interrupts?

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

---

## 📚 Recommended Learning Order

```
Level 0 + Level 1
└─→ Understand hardware fundamentals

Level 2
└─→ Get comfortable with x86 assembly

Level 3
└─→ Understand the boot process

Level 4
└─→ Build your basic kernel

Level 5
└─→ Implement VGA output + keyboard (bonuses)

Level 6
└─→ Advanced features
```

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

### After Level 2:
> "Write assembly code that adds two numbers and stores the result."

### After Level 3:
> "Describe the complete journey from power-on to your kernel's main() function."

### After Level 4:
> "Explain what your linker script does and why each section matters."

### After Level 5:
> "Explain how your kernel receives and processes a keypress."

