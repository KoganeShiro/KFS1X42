#include "kernel.h"

static uint16_t		saved_buffers[MAX_SCREENS][VGA_BUFFER_SIZE];
static ring_buffer	history_pool[MAX_SCREENS];



/*
** kfs_active_screen() - Get pointer to currently active screen
*/
screen* kfs_active_screen(kfs *k)
{
	return (&k->screens[k->current_screen]);
}

/*
** init_screen() - Initialize a single screen's state
**   - saved_buffer always points to this screen's private RAM slot (never changes)
**   - buffer starts pointing to saved_buffer (inactive by default)
**   - Screen 0 will have buffer overridden to VGA_MEMORY after init
*/
static void init_screen(screen *scr, int id)
{
	scr->row = 0;
	scr->column = 0;
	scr->color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	scr->saved_buffer = saved_buffers[id];
	scr->buffer = scr->saved_buffer;
	scr->cursor_pos = 0;
	scr->cursor_visible = true;
	scr->screen_id = id + 1;
	scr->history = &history_pool[id];
	scr->cmdline_index = 0;
	scr->cmdline_buffer[0] = '\0';
	scr->shell_mode = false;

	/* Clear this screen's saved buffer */
	for (size_t i = 0; i < VGA_BUFFER_SIZE; i++)
		scr->saved_buffer[i] = vga_entry('\0', scr->color);

	print_header(scr);
}

/*
** kfs_initialize() - Set up all screens, screen 1-9
** but the array will be indexed from 0 to 8, so screen 1 is at index 0, etc.
**
** All 9 screens are initialized upfront (static array, no malloc).
** Screen 0 gets VGA_MEMORY as its buffer for direct display.
*/
void kfs_initialize(kfs *k) 
{
	k->current_screen = 0;
	k->total_screens = MAX_SCREENS;

	for (int i = 0; i < MAX_SCREENS; i++)
		init_screen(&k->screens[i], i);

	k->screens[0].buffer = (uint16_t*)VGA_MEMORY;

	/* Clear VGA memory (what's actually on the monitor) */
	for (size_t i = 0; i < VGA_BUFFER_SIZE; i++)
		k->screens[0].buffer[i] = vga_entry('\0', k->screens[0].color);

	initialize_cursor_pos(&k->screens[0]);
}


void kernel_main(void) 
{
	static kfs k;

	kfs_initialize(&k);
	screen *scr = kfs_active_screen(&k);

	print_header(scr);
	printk(scr, LOG_LEVEL_INFO "Welcome to KFS-1, a simple %d-bit kernel written in C!\nType 'shell' to enter the command line interface\n", 32);
	printk(scr, LOG_LEVEL_DEBUG"ptr: %p ||| hex of A: %x ||| print char: %c ||| print int: %i\n", &k, 'A', 'A', 'A');
	printk(scr, LOG_LEVEL_WARNING "This is a warning message.\n");
	printk(scr, LOG_LEVEL_ERROR "This is an error message.\n");

	handle_keyboard_keypress(&k);
}
