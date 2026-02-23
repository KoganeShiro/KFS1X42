#include "kernel.h"

/*
** Compute the linear index: index = y * VGA_WIDTH + x
** Write vga_entry(char, color) into buffer[index].
*/
int kfs_putentry(screen *scr, char c) 
{
	const size_t index = scr->row * VGA_WIDTH + scr->column;
	scr->buffer[index] = vga_entry(c, scr->color);
    return (1);
}

/*
** writes a character at the current cursor position, then advances the "cursor".
** Handles special characters: \n (newline).
** If the cursor reaches the end of the line, move to the beginning of the next line.
** If the cursor reaches the end of the screen, move back to the top-left corner.
*/
int kfs_putchar(screen *scr, char c) 
{
	/* Handle newline: move to start of next line */
	if (c == '\n') {
		scr->column = 0;
		if (++scr->row == VGA_HEIGHT)
			scr->row = 0;
		return (1);
	}
	
	/* Regular character: print and advance */
	kfs_putentry(scr, c);
	if (++scr->column == VGA_WIDTH) {
		scr->column = 0;
		if (++scr->row == VGA_HEIGHT)
			scr->row = 0;
			//call scroll function
	}
    return (1);
}
