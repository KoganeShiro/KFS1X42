#include "kernel.h"

/*
** initialize_cursor_pos() - Set cursor position to (0,0) and configure its shape
**
** VGA text mode cursor is drawn as scanlines within a character cell.
** Character cells are typically 16 pixels tall (scanlines 0-15).
**
** Cursor shape options (change the values below):
**   Scanlines 0-15   = Full block cursor (█)
**   Scanlines 13-15  = Thick underline cursor (_)
**   Scanlines 14-15  = Normal underline cursor (_)
**   Scanlines 0-1    = Top line cursor (‾)
**   Scanlines 7-8    = Middle cursor (-)
*/
void initialize_cursor_pos(screen *scr)
{    
    // Set cursor position to top-left (0, 0)
    scr->row = 0;
    scr->column = 0;
    show_cursor(scr);
}

/*
** The cursor position is a 16-bit linear offset in VGA memory:
**   position = row × VGA_WIDTH + column
        -> (is 16bits so need to split because registers are 8bits)
** 
** This value is split into high and low bytes and sent to VGA registers:
**   - High byte (bits 15-8) goes to VGA_CURSOR_HIGH_REGISTER
**   - Low byte  (bits 7-0)  goes to VGA_CURSOR_LOW_REGISTER
*/
void update_cursor_pos(screen *scr)
{
    scr->cursor_pos = scr->row * VGA_WIDTH + scr->column;

    /* update cursor color */
    if ((scr->buffer[scr->cursor_pos] & GET_LOWER_8B_MASK) == '\0')
        scr->buffer[scr->cursor_pos] = vga_entry('\0', scr->color);

    // Tell VGA: "I want to write to VGA_CURSOR_LOW_REGISTER"
    outb(VGA_CTRL_PORT, VGA_CURSOR_LOW_REGISTER);        
    // Write low byte through port, INTO register VGA_CURSOR_LOW_REGISTER
    outb(VGA_DATA_PORT, (uint8_t)(scr->cursor_pos & GET_LOWER_8B_MASK)); 
    
    // Tell VGA: "I want to write to register VGA_CURSOR_HIGH_REGISTER"
    outb(VGA_CTRL_PORT, VGA_CURSOR_HIGH_REGISTER);     
    // Write high byte through port, INTO register VGA_CURSOR_HIGH_REGISTER
    outb(VGA_DATA_PORT, (uint8_t)((scr->cursor_pos >> 8) & GET_LOWER_8B_MASK)); 
}

void hide_cursor(screen *scr)
{
    (void)scr;
    // Setting bit 5 of the cursor start register to 1 disables the cursor.
    outb(VGA_CTRL_PORT, VGA_CURSOR_START);
    outb(VGA_DATA_PORT, 0x20); // Bit 5 = 1 to disable cursor
}

void show_cursor(screen *scr)
{
    (void)scr;
    // Set cursor shape: scanlines 0-15 = full block cursor
    uint8_t cursor_start = 0;   // Top scanline (0-15)
    uint8_t cursor_end = 15;    // Bottom scanline (0-15)
    
    // Enable cursor and set shape
    outb(VGA_CTRL_PORT, VGA_CURSOR_START);
    outb(VGA_DATA_PORT, cursor_start & 0x1F);  // Bit 5=0 enables cursor
    
    outb(VGA_CTRL_PORT, VGA_CURSOR_END);
    outb(VGA_DATA_PORT, cursor_end & 0x1F);
}
