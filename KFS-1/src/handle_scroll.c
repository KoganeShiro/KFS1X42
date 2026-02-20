#include "kernel.h"

/* handle_scroll.c */
/* how the vga screen work under the hood when we can scroll ?

**  terminal_scroll() 
** Called when the cursor moves past the last row.
** 1. Copy every row UP by one:
**      for each row y from 1 to VGA_HEIGHT-1:
**        buffer[(y-1) * WIDTH + x] = buffer[y * WIDTH + x]
** 2. Clear the last row:
**      fill row (VGA_HEIGHT-1) with spaces + current color
*/

/*
when overflow need to clean the screen and start from the top
(scroll -> store in a buffer, max will be 30 lines, when overflow, remove the first line and add a new line at the end)


void clear_screen(void)
{
    int index = 0;
    
    while (index < 80 * 25 * 2) {
            terminal_buffer[index] = '\0';
            index += 2;
    }
}
*/