#include "kernel.h"

/* handle_scroll.c */
/*
Steps:
1. Write text to: buffer[current_end]
2. Increment: current_end = (current_end + 1) % BUFFER_SIZE
3. If visible at bottom (showing last 25 lines):
   - Shift VGA up by 1 line
   - Copy buffer[current_end - 1] to VGA row 24
   - Increment visible_start: visible_start = (visible_start + 1) % BUFFER_SIZE
*/
/*
**  terminal_scroll_up() 
** Called when the cursor moves past the last row.
** 1. Copy every row UP by one:
**      for each row y from 1 to VGA_HEIGHT-1:
**        buffer[(y-1) * WIDTH + x] = buffer[y * WIDTH + x]
** 2. Clear the last row:
**      fill row (VGA_HEIGHT-1) with spaces + current color
*/

/*
Steps:
1. Write text to: buffer[current_end]
2. Increment: current_end = (current_end + 1) % BUFFER_SIZE
3. If visible at bottom (showing last 25 lines):
   - Shift VGA up by 1 line
   - Copy buffer[current_end - 1] to VGA row 24
   - Increment visible_start: visible_start = (visible_start + 1) % BUFFER_SIZE
*/
// void terminal_overflow_down() {
//     // Shift VGA up by one line (2080 bytes = 80 chars × 2 bytes × 25 lines - 1)
//     memmove(&vga[0], &vga[80], 24 * 80 * 2);
    
//     // Copy only the new bottom line from buffer
//     memcpy(&vga[24 * 80], &buffer[current_bottom_line], 80 * 2);
// }

/* implement clear command */
// void clear_terminal(void)
// {
//     int index = 0;
    
//     while (index < 80 * 25 * 2) {
//             terminal_buffer[index] = '\0';
//             index += 2;
//     }
// }
