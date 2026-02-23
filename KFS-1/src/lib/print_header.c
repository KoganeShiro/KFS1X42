#include "kernel.h"


void print_header(screen *scr) 
{
    int color_index = 0;
    int default_color = scr->color;
    uint8_t rainbow[] = {
        VGA_COLOR_RED,
        VGA_COLOR_LIGHT_BROWN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_MAGENTA
    };
    
    printk(scr, "********************************************************************************\n");
    /* Group ASCII art into 6 colored sections */
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "        _  _  ____                               \n");
    printk(scr, "       | || ||___ \\                              \n");
    
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | || |_ __) |                             \n");
    printk(scr, "       |__   _/ __/  _                           \n");
    
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       __ |_||_____|| | ___ ___  _ __ ___   ___  \n");
    printk(scr, "       \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ \n");
    
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "        \\ V  V /  __/ | (_| (_) | | | | | |  __/ \n");
    printk(scr, "        _\\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___| \n");
    
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | |_ ___   | | __/ _|___      / |         \n");
    printk(scr, "       | __/ _ \\  | |/ / |_/ __|_____| |         \n");
    
    kfs_set_color(scr, vga_entry(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | || (_) | |   <|  _\\__ \\_____| |         \n");
    printk(scr, "        \\__\\___/  |_|\\_\\_| |___/     |_|         \n\n");
    
    kfs_set_color(scr, vga_entry(default_color, VGA_COLOR_BLACK));
    printk(scr, "********************************************************************************\n");
    printk(scr, "\n");
    
    return;
}
