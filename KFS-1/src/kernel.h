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

#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
// Bit 7 set indicates key release
#define KEY_RELEASE_MASK 0x80 

#define CURSOR


/* VGA text mode constants */
# define VGA_WIDTH   80
# define VGA_HEIGHT  25
# define VGA_MEMORY  0xB8000

/* printk log level */
# define LOG_LEVEL_INFO "[INFO] "
# define LOG_LEVEL_DEBUG "[DEBUG] "
# define LOG_LEVEL_WARNING "[WARNING] "
# define LOG_LEVEL_ERROR "[ERROR] "

/* Special key code */
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_LCTRL       0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_LALT        0x38
#define KEY_CAPSLOCK    0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_NUMLOCK     0x45

// Arrow keys (extended scancodes, prefixed with 0xE0)
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D


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
	//screens index to write the buffer in
} vga_terminal;


/* Variadic Arguments */
//point to the stack (reverse order)
typedef struct va_list {
	void* ptr;
} va_list;

//init the ptr to the last argument
#define va_start(argptr, last) ((argptr).ptr = (void*)(&(last) + 1))

//cast the arg by the type and move the ptr to the next argument
#define va_arg(argptr, type) \
    (*(type*)((argptr).ptr += sizeof(type) , (char*)(argptr).ptr - sizeof(type)))

//ptr that pointed to the stack to NULL
#define va_end(argptr) ((argptr).ptr = NULL)



/* Kernel functions */
void	kfs_initialize(vga_terminal *terminal);

/* printk.c */
int		printk(vga_terminal *terminal, const char *data, ...);
/* printk_utils.c */
int		kfs_putentry_at(vga_terminal *terminal, char c);
int		kfs_putchar(vga_terminal *terminal, char c);

/* io.c */
uint8_t inb(uint16_t port);
void 	outb(uint16_t port, uint8_t val);
void 	outw(uint16_t port, uint16_t val);


/* handle_keyboard.c */
void	handle_keyboard_keypress(vga_terminal *terminal);

/* handle_screen_shortcut.c */
int		handle_screen_shortcut(vga_terminal *terminal, uint8_t scancode);

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
uint16_t	vga_entry(unsigned char uc, uint8_t color) ;
void		kfs_set_color(vga_terminal *terminal, uint8_t color);
size_t 		strlen(const char* str) ;
void 		print_header(vga_terminal *terminal) ;



#endif
