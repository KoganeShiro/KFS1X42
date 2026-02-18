#ifndef KERNEL_H
# define KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Check if the compiler thinks we are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

/* VGA text mode constants */
# define VGA_WIDTH   80
# define VGA_HEIGHT  25
# define VGA_MEMORY  0xB8000

/* printk log level */

# define LOG_LEVEL_INFO "[INFO]"
# define LOG_LEVEL_WARNING "[WARNING]"
# define LOG_LEVEL_ERROR "[ERROR]"

/* Hardware VGA color palette */
enum vga_color
{
	VGA_COLOR_BLACK         = 0,
	VGA_COLOR_BLUE          = 1,
	VGA_COLOR_GREEN         = 2,
	VGA_COLOR_CYAN          = 3,
	VGA_COLOR_RED           = 4,
	VGA_COLOR_MAGENTA       = 5,
	VGA_COLOR_BROWN         = 6,
	VGA_COLOR_LIGHT_GREY    = 7,
	VGA_COLOR_DARK_GREY     = 8,
	VGA_COLOR_LIGHT_BLUE    = 9,
	VGA_COLOR_LIGHT_GREEN   = 10,
	VGA_COLOR_LIGHT_CYAN    = 11,
	VGA_COLOR_LIGHT_RED     = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN   = 14,
	VGA_COLOR_WHITE         = 15,
};

/*
**   - row:    current cursor row      (size_t)
**   - column: current cursor column   (size_t)
**   - color:  current text color byte (uint8_t)
**   - buffer: pointer to VGA memory   (uint16_t *)
*/
typedef struct vga_terminal {
	size_t row;
	size_t column;
	uint8_t color;
	uint16_t* buffer;
} vga_terminal;


/* Kernel functions */
void	kfs_initialize(vga_terminal *terminal);

/* printk.c */
void	kfs_putentry_at(vga_terminal *terminal, char c);
void	kfs_putchar(vga_terminal *terminal, char c);
void	printk(vga_terminal *terminal, const char *data, ...);


/* handle_keyboard.c */
/*
** get_scancode()
Read a byte from the keyboard's data port (0x60).
** map_scancode()
Convert the scancode to a character (e.g., 'a', '1', etc.)
** process_scancode()
Call map_scancode() and print it.

** handle_keyboard_keypress()
call get_scancode() in a loop, and call process_scancode() for each scancode read.

/*
** update_cursor()
Update terminal->row and terminal->column if arrow keys are pressed.
*/
/*
** handle_screen_shortcut()
Switch screens when alt+1, alt+2, etc. are pressed.
*/

/* handle_scroll.c */
/*
**  terminal_scroll() 
** Called when the cursor moves past the last row.
** 1. Copy every row UP by one:
**      for each row y from 1 to VGA_HEIGHT-1:
**        buffer[(y-1) * WIDTH + x] = buffer[y * WIDTH + x]
** 2. Clear the last row:
**      fill row (VGA_HEIGHT-1) with spaces + current color
*/

/* handle_screen.c */
/*
* add_screen()
Add a new screen to the list of screens, and switch to it.
* switch_screen()
Switch to an existing screen by index.
*/

// Helper functions (utils.c)
uint16_t vga_entry(unsigned char uc, uint8_t color) ;
void	kfs_set_color(vga_terminal *terminal, uint8_t color);
size_t 	strlen(const char* str) ;
void 	print_header(vga_terminal *terminal) ;



#endif
