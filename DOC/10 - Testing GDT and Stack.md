# Testing & Verification Guide: GDT and Stack

This document explains how to verify that your GDT (Global Descriptor Table) and Stack are correctly configured using the QEMU monitor.

## 1. Verifying the GDT

### What to do
1. Boot your kernel in QEMU.
2. Open the QEMU monitor (usually `Ctrl-Alt-2`, or start QEMU with `-monitor stdio`).
3. Run the command: `info registers`

### What you should see
Look for the `GDT=` and segment register lines:
```text
GDT=     00000800 00000037
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
DS =0010 00000000 ffffffff 00cf9200 DPL=0 DS   [-W-]
ES =0010 00000000 ffffffff 00cf9200 DPL=0 DS   [-W-]
SS =0010 00000000 ffffffff 00cf9200 DPL=0 DS   [-W-]
```

### Why you should see it
* **`GDT= 00000800 00000037`**: 
  * `00000800` is your memory Base. This matches `GDT_ADDR` in your C code.
  * `00000037` is the Limit (size). In hex, `0x37` equals 55 in decimal. Because your GDT size is (`7 entries * 8 bytes = 56 bytes`), the limit is `Size - 1`, which is exactly 55 (`0x37`).
* **`CS =0008` & `DS =0010`**:
  * Segment selectors are offsets into the GDT. 
  * `0x08` (8 in decimal) points to the 1st entry (Kernel Code) -> `1 * 8 bytes = 8`.
  * `0x10` (16 in decimal) points to the 2nd entry (Kernel Data) -> `2 * 8 bytes = 16`.
  * This confirms your `load_gdt.s` assembly file executed the far jump (`ljmp $0x08, $flush`) successfully!

---

## 2. Verifying the Stack

### The Observation
When you run `info registers` in QEMU, you might see an ESP like `00208f98`. 
But when you run the `stack` command in your OS shell, it prints a smaller address like `00208f34`.

**Is this an error? No, this is exactly what is supposed to happen!**

### Why this happens
The x86 architecture stack **grows** in memory (from higher addresses to lower addresses or lower addresses to higher addresses, depending on the implementation). 

When QEMU sits idle waiting for your keyboard input, the stack is relatively "empty" at its baseline (e.g., `0x00208F98`). However, when you type `stack` and hit Enter, the CPU executes a chain of function calls:
1. Keyboard Interrupt handling
2. `handle_commands()`
3. String comparison functions
4. `print_stack()`
5. Your inline assembly `asm volatile` block.

**Every single function call** pushes data onto the stack to remember how to return:
* The Instruction Pointer (EIP)
* The Base Pointer (EBP)
* Function arguments
* Local variables (like `gdt_index`, `stack_memory`, etc.)

By the time the code actually reaches your inline assembly to snapshot the `esp` register, the stack has grown downward by around ~100 bytes (from `00208f98` down to `00208f34`).


