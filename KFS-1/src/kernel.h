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

#define GET_LOWER_8B_MASK 0xFF // Mask to extract character from VGA entry (lower 8 bits)

#define KEYBOARD_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
// Bit 7 set indicates key release
#define KEY_RELEASE_MASK 0x80 

/* VGA Cursor Control Ports */
#define VGA_CTRL_PORT		0x3D4  // VGA control port for cursor position
#define VGA_DATA_PORT  		0x3D5  // VGA data port for cursor position
#define VGA_CURSOR_START   		0x0A   // Cursor start scanline register (also enables/disables cursor)
#define VGA_CURSOR_END     		0x0B   // Cursor end scanline register
#define VGA_CURSOR_HIGH_REGISTER   		0x0E   // Cursor location high byte register
#define VGA_CURSOR_LOW_REGISTER    		0x0F   // Cursor location low byte register

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


/* Forward declarations */
typedef struct screen screen;
typedef struct ring_buffer ring_buffer;

/* Scroll-back ring buffer (text storage only) */
typedef struct ring_buffer {
    char    buffer[VGA_WIDTH * VGA_HEIGHT * 30];
    size_t  visible_start;
    size_t  current_line;
    size_t  total_lines;
} ring_buffer;

/*
** Per-screen state:
**   - row:    current cursor row      (size_t)
**   - column: current cursor column   (size_t)
**   - color:  current text color byte (uint8_t)
**   - buffer: pointer to VGA memory   (uint16_t *)
*/
typedef struct screen {
    size_t          row;
    size_t          column;
    uint8_t         color;
    uint16_t*       buffer;        /* points to VGA_MEMORY or saved buffer */
    uint16_t        cursor_pos;
    int             screen_id;
    ring_buffer*    history;       /* scroll-back history for this screen */
    struct screen*  next;          /* linked list of screens */
} screen;

/* Global kernel state */
typedef struct kfs {
	int current_screen;
	screen* screen_list;
} kfs;


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



/* Get the currently active screen */
screen*	kfs_active_screen(kfs *k);

/* Kernel functions */
void	kfs_initialize(kfs *k);

/* printk.c */
int		printk(screen *scr, const char *data, ...);
/* convert.c */
char* 	itoa(int value, char* buffer);
char* itoh(unsigned int value, char* buffer, char *hexa);
char* reverse_str(char* buffer);


/* vga_write.c */
int		kfs_putentry(screen *scr, char c);
int		kfs_putchar(screen *scr, char c);

/* port_io.c */
uint8_t inb(uint16_t port);
void 	outb(uint16_t port, uint8_t val);
void 	outw(uint16_t port, uint16_t val);


/* handle_keyboard.c */
void	handle_keyboard_keypress(kfs *k);

/* handle_screen_shortcut.c */
int		handle_screen_shortcut(kfs *k, uint8_t scancode);

/* keymap.c */
char	keymap_translate(uint8_t scancode, int shift_pressed);


/* handle_cursor.c */
void	initialize_cursor_pos(screen *scr);
void	update_cursor_pos(screen *scr);

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

/*
parsing.c
create a micro posix shell where it could execute "clear" command


Global Descriptor Table (GDT) and Interrupt Descriptor Table (IDT) setup
	(ONLY THINK OF WHERE IT SHOULD BE CALLED, NOT THE IMPLEMENTATION)
*/

// Helper functions
/* vga.c */
uint16_t	vga_entry(unsigned char uc, uint8_t color) ;
void		kfs_set_color(screen *scr, uint8_t color);

/* string.c */
size_t 		strlen(const char* str) ;
void* 		memmove(void* dstptr, const void* srcptr, size_t size);
int 		memcmp(const void* aptr, const void* bptr, size_t size);
void* 		memset(void* bufptr, int value, size_t size);
void* 		memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);

/* print_header.c */
void 		print_header(screen *scr) ;



#endif
