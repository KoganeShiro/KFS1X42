#include "kernel.h"

/*
** Compute the linear index: index = y * VGA_WIDTH + x
** Write vga_entry(char, color) into buffer[index].
*/
int kfs_putentry_at(vga_terminal *terminal, char c) 
{
	const size_t index = terminal->row * VGA_WIDTH + terminal->column;
	terminal->buffer[index] = vga_entry(c, terminal->color);
    return (1);
}

/*
** writes a character at the current cursor position, then advances the "cursor".
** Handles special characters: \n (newline).
** If the cursor reaches the end of the line, move to the beginning of the next line.
** If the cursor reaches the end of the screen, move back to the top-left corner.
*/
int kfs_putchar(vga_terminal *terminal, char c) 
{
	/* Handle newline: move to start of next line */
	if (c == '\n') {
		terminal->column = 0;
		if (++terminal->row == VGA_HEIGHT)
			terminal->row = 0;
		return (1);
	}
	
	/* Regular character: print and advance */
	kfs_putentry_at(terminal, c);
	if (++terminal->column == VGA_WIDTH) {
		terminal->column = 0;
		if (++terminal->row == VGA_HEIGHT)
			terminal->row = 0;
	}
    return (1);
}
