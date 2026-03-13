/*
 * boot.s - Kernel Bootstrap Assembly
 * 
 * This file is the first code that executes when the kernel starts.
 * It bridges the gap between the bootloader (GRUB) and our C kernel.
 * 
 * What it does:
 *   1. Declares a Multiboot header (notice where the kernel is) so GRUB recognizes this as a kernel
 *   2. Sets up a 16KB stack (required before C code can run)
 *   3. Calls kernel_main() - our C entry point
 *   4. Halts the CPU if kernel_main ever returns
 * 
 * Entry state (set by GRUB):
 *   - 32-bit protected mode
 *   - Interrupts disabled
 *   - No paging
 *   - EAX = 0x2BADB002 (multiboot magic)
 *   - EBX = pointer to multiboot info
 */

/* Multiboot header constants */
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

/* Multiboot header - must be in first 8KB of kernel binary (grub require) */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* setup 16KB stack in .bss section (grows downward) */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

/* Kernel entry point */
.section .text
.global _start
.type _start, @function
_start:
	/* Setup stack pointer (required for C code) */
	mov $stack_top, %esp

	/* Zero BSS section (static variables must start at 0) */
	cld                       /* Clear direction flag: rep stosb must go FORWARD */
	mov $__bss_start, %edi
	mov $__bss_end, %ecx
	sub %edi, %ecx
	xor %eax, %eax
	rep stosb

	/* Jump to C kernel */
	call kernel_main

	/* Halt the CPU (should never reach here) */
	cli
1:	hlt
	jmp 1b
.size _start, . - _start