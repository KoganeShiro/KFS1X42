#include "kernel.h"

/*
** Compute the linear index: index = y * VGA_WIDTH + x
** Write vga_entry(char, color) into buffer[index].
*/
void kfs_putentry_at(vga_terminal *terminal, char c) 
{
	const size_t index = terminal->row * VGA_WIDTH + terminal->column;
	terminal->buffer[index] = vga_entry(c, terminal->color);
    return;
}

/*
** writes a character at the current cursor position, then advances the "cursor".
** Handles special characters: \n (newline).
** If the cursor reaches the end of the line, move to the beginning of the next line.
** If the cursor reaches the end of the screen, move back to the top-left corner.
*/
void kfs_putchar(vga_terminal *terminal, char c) 
{
	/* Handle newline: move to start of next line */
	if (c == '\n') {
		terminal->column = 0;
		if (++terminal->row == VGA_HEIGHT)
			terminal->row = 0;
		return;
	}
	
	/* Regular character: print and advance */
	kfs_putentry_at(terminal, c);
	if (++terminal->column == VGA_WIDTH) {
		terminal->column = 0;
		if (++terminal->row == VGA_HEIGHT)
			terminal->row = 0;
	}
    return;
}

// static char* 	itoa(int value, char* buffer)
// {
//     return;
// }

static void kfs_putstr(vga_terminal *terminal, const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		kfs_putchar(terminal, data[i]);

    return;
}

/*
* Will use the same simple format as printf in C: only %d, %s, and %c.
* But it has a level logic added to it.
*/
void printk(vga_terminal *terminal, const char* data, ...) 
{
    //va_list args;
    //va_start(args, data);

	kfs_putstr(terminal, data, strlen(data));

    //va_end(args);
    return;
}

