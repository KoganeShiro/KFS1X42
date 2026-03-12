#include "kernel.h"

void add_char_to_cmdline_buffer(screen *scr, char c)
{
    if (scr->cmdline_index < VGA_BUFFER_SIZE - 1) {
        scr->cmdline_buffer[scr->cmdline_index] = c;
        scr->cmdline_index++;
        scr->cmdline_buffer[scr->cmdline_index] = '\0';
    }
}

void shell_prompt(screen *scr)
{
    const char* prompt = "$> ";
    printk(scr, prompt);
}