#include "kernel.h"

/*
** Packs a character and its color byte into one 16-bit VGA cell.
*/
uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void kfs_set_color(screen *scr, uint8_t color) 
{
	scr->color = color;
}
