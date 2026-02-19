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
