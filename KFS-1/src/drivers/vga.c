#include "kernel.h"

/*
** Packs a character and its color byte into one 16-bit VGA cell.
*/
uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

/*
** Packs two 4-bit colors (foreground + background) into one byte.
*/
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

void kfs_set_color(screen *scr, uint8_t color) 
{
	scr->color = color;
}
