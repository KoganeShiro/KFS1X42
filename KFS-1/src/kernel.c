#include "kernel.h"

/*
** Packs two 4-bit colors (foreground + background) into one byte.
*/
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static screen default_screen;

/*
** kfs_active_screen() - Get the currently active screen from the linked list
*/
screen* kfs_active_screen(kfs *k)
{
	screen *scr = k->screen_list;
	for (int i = 0; i < k->current_screen && scr->next; i++)
		scr = scr->next;
	return (scr);
}

/*
** 1. Set up the kfs global state and the first screen
** 2. Set default color (white on black)
** 3. Point the buffer to VGA_MEMORY (0xB8000)
** 4. Loop through every cell (80 × 25 = 2000 cells)
**    and fill each with a space character + default color
**    → this clears the screen
*/
void kfs_initialize(kfs *k) 
{
	k->current_screen = 0;
	k->screen_list = &default_screen;

	screen *scr = kfs_active_screen(k);
	scr->buffer = (uint16_t*)VGA_MEMORY;
	scr->screen_id = 0;
	scr->history = NULL;
	scr->next = NULL;

	initialize_cursor_pos(scr);
	scr->color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			scr->buffer[index] = vga_entry('\0', scr->color);
		}
	}
}


void kernel_main(void) 
{
	kfs k;

	/* Initialize terminal interface */
	kfs_initialize(&k);

	screen *scr = kfs_active_screen(&k);

	print_header(scr);

	printk(scr, "ptr: %p ||| hex of a: %x", &k, 'A');
	printk(scr, "print char: %c\n", 'A');
	printk(scr, "print int: %i\n", 'A');
	printk(scr, LOG_LEVEL_DEBUG "Kernel initialized successfully!\n");
	printk(scr, LOG_LEVEL_INFO "Welcome to KFS-1, a simple %d-bit kernel written in C!\n", 32);
	printk(scr, LOG_LEVEL_WARNING "This is a warning message.\n");
	printk(scr, LOG_LEVEL_ERROR "This is an error message.\n");

	handle_keyboard_keypress(&k);
}
