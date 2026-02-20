#include "kernel.h"

/*
** Packs a character and its color byte into one 16-bit VGA cell.
*/
uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

void kfs_set_color(vga_terminal *terminal, uint8_t color) 
{
	terminal->color = color;
}

size_t strlen(const char* str) 
{
	size_t len = 0;

	while (str[len]) {
		len++;
    }
	return (len);
}

void* memmove(void* dstptr, const void* srcptr, size_t size)
{
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i-1] = src[i-1];
	}
	return dstptr;
}

int memcmp(const void* aptr, const void* bptr, size_t size)
{
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void* memset(void* bufptr, int value, size_t size)
{
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++)
		buf[i] = (unsigned char) value;
	return bufptr;
}

void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size)
{
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}


void print_header(vga_terminal *terminal) 
{
    int color_index = 0;
    int default_color = terminal->color;
    uint8_t rainbow[] = {
        VGA_COLOR_RED,
        VGA_COLOR_LIGHT_BROWN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_MAGENTA
    };
    
    printk(terminal, "********************************************************************************\n");
    /* Group ASCII art into 6 colored sections */
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "        _  _  ____                               \n");
    printk(terminal, "       | || ||___ \\                              \n");
    
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "       | || |_ __) |                             \n");
    printk(terminal, "       |__   _/ __/  _                           \n");
    
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "       __ |_||_____|| | ___ ___  _ __ ___   ___  \n");
    printk(terminal, "       \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ \n");
    
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "        \\ V  V /  __/ | (_| (_) | | | | | |  __/ \n");
    printk(terminal, "        _\\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___| \n");
    
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "       | |_ ___   | | __/ _|___      / |         \n");
    printk(terminal, "       | __/ _ \\  | |/ / |_/ __|_____| |         \n");
    
    kfs_set_color(terminal, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(terminal, "       | || (_) | |   <|  _\\__ \\_____| |         \n");
    printk(terminal, "        \\__\\___/  |_|\\_\\_| |___/     |_|         \n\n");
    
    kfs_set_color(terminal, vga_entry(default_color, VGA_COLOR_BLACK));
    printk(terminal, "********************************************************************************\n");
    printk(terminal, "\n");
    
    return;
}
