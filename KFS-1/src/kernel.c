#include "kernel.h"

/*
** Packs two 4-bit colors (foreground + background) into one byte.
*/
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

/*
** 1. Reset row and column to 0
** 2. Set default color (white on black)
** 3. Point the buffer to VGA_MEMORY (0xB8000)
** 4. Loop through every cell (80 × 25 = 2000 cells)
**    and fill each with a space character + default color
**    → this clears the screen
*/
void kfs_initialize(vga_terminal *terminal) 
{
	terminal->buffer = (uint16_t*)VGA_MEMORY;
	terminal->row = 0;
	terminal->column = 0;
	terminal->color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal->buffer[index] = vga_entry(' ', terminal->color);
		}
	}
}


void kernel_main(void) 
{
	vga_terminal terminal;

	/* Initialize terminal interface */
	kfs_initialize(&terminal);

	print_header(&terminal);

	printk(&terminal, "ptr: %p ||| hex of a: %x", &terminal, 'A');
	printk(&terminal, "print char: %c\n", 'A');
	printk(&terminal, "print int: %i\n", 'A');
	printk(&terminal, LOG_LEVEL_DEBUG "Kernel initialized successfully!\n");
	printk(&terminal, LOG_LEVEL_INFO "Welcome to KFS-1, a simple %d-bit kernel written in C!\n", 32);
	printk(&terminal, LOG_LEVEL_WARNING "This is a warning message.\n");
	printk(&terminal, LOG_LEVEL_ERROR "This is an error message.\n");

	handle_keyboard_keypress(&terminal);
}
