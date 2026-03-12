#include "kernel.h"


/* implement clear command */
// void clear_terminal(void)
// {
//     int index = 0;
    
//     while (index < 80 * 25 * 2) {
//             terminal_buffer[index] = '\0';
//             index += 2;
//     }
// }

void clear_terminal(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Clearing the terminal...\n");
}