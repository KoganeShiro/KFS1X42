#include "kernel.h"

void clear_terminal(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Clearing the terminal...\n");
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        scr->buffer[i] = '\0' | (scr->color << 8);
    }
    scr->column = 0;
    scr->row = 0;
    scr->cmdline_index = 0;
    scr->cmdline_buffer[0] = '\0';
    scr->buffer[scr->cursor_pos] = vga_entry('\0', scr->color);
    scr->saved_buffer[scr->cursor_pos] = vga_entry('\0', scr->color);
    scr->history->scroll_offset = 0;
    scr->history->total_lines = 0;
    scr->history->current_line = 0;
    update_cursor_pos(scr);
}