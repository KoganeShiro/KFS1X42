/*
 * linker.ld - Kernel Linker Script
 * 
 * Defines the memory layout of the final kernel binary.
 * Controls where each section (.text, .data, .bss, etc.) is placed in memory.
 * 
 * Key decisions:
 *   - Entry point: _start (defined in boot.s)
 *   - Load address: 2MB (safer than traditional 1MB for UEFI compatibility)
 *   - Section order: .multiboot must be first (GRUB requirement)
 * 
 * Memory layout:
 *   .text    → Executable code (includes .multiboot header)
 *   .rodata  → Read-only data (string constants)
 *   .data    → Initialized global variables
 *   .bss     → Uninitialized data (zeroed by loader, includes stack)
 */

ENTRY(_start)

SECTIONS
{
	/* Load kernel at 2MB */
	. = 2M;

	/* .text: Executable code */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}

	/* .rodata: Read-only data */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* .data: Initialized global/static variables */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* .bss: Uninitialized data (includes stack) */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		__bss_start = .;
		*(COMMON)
		*(.bss)
		__bss_end = .;
	}
}