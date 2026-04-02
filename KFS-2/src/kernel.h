#ifndef KERNEL_H
# define KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// #include <stdarg.h>

/* Variadic Arguments */
// point to the stack (reverse order)
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
# define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)
# define HISTORY_MAX_LINES 42
# define MAX_SCREENS 9

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
#define KEY_PG_UP       0x49
#define KEY_PG_DOWN     0x51

// Arrow keys (extended scancodes, prefixed with 0xE0)
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4B
#define KEY_RIGHT       0x4D


#define HELP_MESSAGE "Available commands:\n" \
					 "  clear:       	Clear the terminal screen\n" \
					 "  gdt:    		Print the GDT table\n" \
					 "  stack:  		Print the stack\n" \
					 "  header:      	Print the kernel header\n" \
					 "  42header:    	Print the 42 header\n" \
					 "  help:        	Show this help message\n" \
					 "  halt:        	Halt the system\n" \
					 "  shutdown:    	Shutdown the system\n" \
					 "  reboot:      	Reboot the system\n" \
					 "  exit:        	Exit shell mode\n" \
					 "  COLOR:       	Will show available colors\n"

#define COLOR_MESSAGE "Available colors:\n" \
					 "  BLUE:        	Change text color to blue\n" \
					 "  GREEN:       	Change text color to green\n" \
					 "  CYAN:        	Change text color to cyan\n" \
					 "  RED:         	Change text color to red\n" \
					 "  MAGENTA:     	Change text color to magenta\n" \
					 "  BROWN:       	Change text color to brown\n" \
					 "  LIGHT_GREY:  	Change text color to light grey\n" \
					 "  DARK_GREY:   	Change text color to dark grey\n" \
					 "  LIGHT_BLUE:  	Change text color to light blue\n" \
					 "  LIGHT_GREEN: 	Change text color to light green\n" \
					 "  LIGHT_CYAN:  	Change text color to light cyan\n" \
					 "  LIGHT_RED:   	Change text color to light red\n" \
					 "  LIGHT_MAGENTA: 	Change text color to light magenta\n" \
					 "  LIGHT_BROWN: 	Change text color to light brown\n" \
					 "  WHITE:    		Change text color to white\n" \
					 "  RESET:    		Reset text color to white\n"


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


/* Scroll-back ring buffer (stores full VGA entries: char + color) */
typedef struct ring_buffer {
    uint16_t  buffer[VGA_WIDTH * HISTORY_MAX_LINES];
    uint16_t  live_snapshot[VGA_BUFFER_SIZE]; /* VGA snapshot taken when entering scroll mode */
    size_t    current_line;  /* next line to write into (ring head)     */
    size_t    total_lines;   /* how many lines stored (capped at max)   */
    size_t    scroll_offset; /* 0 = live view, >0 = scrolled back N lines */
} ring_buffer;

/*
** Per-screen state:
**   - row:    current cursor row      (size_t)
**   - column: current cursor column   (size_t)
**   - color:  current text color byte (uint8_t)
**   - buffer: pointer to VGA memory   (uint16_t *)
*/
/*
** Per-screen state:
**   - buffer:       active write target (VGA_MEMORY for active screen, saved_buffer for inactive)
**   - saved_buffer: always points to this screen's private RAM copy (static allocation)
**                   on switch_screen: memcpy(VGA <-> saved_buffer)
*/
typedef struct screen {
    size_t          row;
    size_t          column;
    uint8_t         color;
	char			cmdline_buffer[VGA_BUFFER_SIZE]; /* buffer to store current command line input */
    size_t 		   	cmdline_index;  /* current index in the cmdline_buffer */
	bool			shell_mode;      /* flag to indicate if we are in shell mode */
	uint16_t*       buffer;        /* active write target: VGA_MEMORY or saved_buffer */
    uint16_t*       saved_buffer;  /* always points to this screen's private RAM copy */
    uint16_t        cursor_pos;
	bool			cursor_visible;
    int             screen_id;
    ring_buffer*    history;       /* scroll-back history for this screen */
} screen;

//GDT
#define NB_OF_GDT_ENTRY 7
#define GDT_ADDR    0x00000800

typedef struct gdt_access {
	uint8_t accessed        : 1; // Bit 0: CPU sets this to 1 when segment is accessed
	uint8_t read_write      : 1; // Bit 1: Readable for Code, Writable for Data
	uint8_t conforming      : 1; // Bit 2: Conforming bit
	uint8_t executable      : 1; // Bit 3: 1 for Code, 0 for Data
	uint8_t descriptor_type : 1; // Bit 4: 1 for Code/Data, 0 for System
	uint8_t privilege_level : 2; // Bits 5-6: 0 for Kernel, 3 for User
	uint8_t present         : 1; // Bit 7: Must be 1 for valid segments
} __attribute__((packed)) gdt_access;

typedef struct gdt_granularity {
	uint8_t limit_high      : 4; // Bits 0-3: Upper 4 bits of segment limit
	uint8_t available       : 1; // Bit 4: Available for system use
	uint8_t long_mode       : 1; // Bit 5: 1 for 64-bit code segment
	uint8_t size_flag       : 1; // Bit 6: 1 for 32-bit protected mode
	uint8_t granularity     : 1; // Bit 7: 1 for 4KB page granularity
} __attribute__((packed)) gdt_granularity;

typedef struct descriptor {
	uint16_t limit_low;      // Lower 16 bits of the segment limit
	uint16_t base_low;       // Lower 16 bits of the base address
	uint8_t  base_middle;    // Next 8 bits of the base address
	gdt_access access;       // Access flags using bitfields
	gdt_granularity granularity; // Granularity and limit using bitfields
	uint8_t  base_high;      // Last 8 bits of the base address
} __attribute__((packed)) descriptor;


typedef struct gdt_ptr {
	uint16_t limit; // Size of the GDT (total size of all entries - 1)
	uint32_t base; // GDT_ADDR
} __attribute__((packed)) gdt_ptr;

typedef struct gdt {
	descriptor entries[NB_OF_GDT_ENTRY];
	gdt_ptr    ptr;
} gdt;

/* Global kernel state */
typedef struct kfs {
	int current_screen;
	int total_screens;
	screen screens[MAX_SCREENS];
	gdt *gdt;
} kfs;


/* Get the currently active screen */
screen*	kfs_active_screen(kfs *k);

/* Kernel functions */
void	kfs_initialize(kfs *k);

/* printk.c */
int		printk(screen *scr, const char *data, ...);
/* convert.c */
char* 	itoa(int value, char* buffer);
char* 	itoh(unsigned int value, char* buffer, char *hexa);


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
bool	handle_screen_shortcut(kfs *k, uint8_t scancode);

/* keymap.c */
char	keymap_translate(uint8_t scancode, bool shift_pressed);


/* handle_cursor.c */
void	initialize_cursor_pos(screen *scr);
void	update_cursor_pos(screen *scr);
void	hide_cursor(screen *scr);
void	show_cursor(screen *scr);

/* handle_scroll.c */
bool	handle_scroll_key(kfs *k, uint8_t scancode);
void 	terminal_overflow_down(screen *scr) ;	


/* handle_screen.c */
void	switch_screen(kfs *k, int screen_index);

/* GDT */
void init_gdt(kfs *k);
// void fill_gdt(kfs *k);


/* handle_commands.c */
void add_char_to_cmdline_buffer(screen *scr, char c);
void shell_prompt(screen *scr);

/* parsing_command.c */
void command_parser(gdt *gdt, screen *scr);


void kfs_shutdown(screen *scr);
void kfs_reboot(screen *scr);
void clear_terminal(screen *scr);
void kfs_halt(screen *scr);
void print_gdt(screen *scr, gdt *gdt);
void print_stack(screen *scr, gdt *gdt);


// Helper functions
/* vga.c */
uint16_t	vga_entry(unsigned char uc, uint8_t color) ;
uint8_t		vga_entry_color(enum vga_color fg, enum vga_color bg);
void		kfs_set_color(screen *scr, uint8_t color);

/* string.c */
int strcmp(const char* aptr, const char* bptr);
size_t 		strlen(const char* str) ;
void* 		memmove(void* dstptr, const void* srcptr, size_t size);
int 		memcmp(const void* aptr, const void* bptr, size_t size);
void* 		memset(void* bufptr, int value, size_t size);
void* 		memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size);

/* print_header.c */
void 		print_42_header(screen *scr);
void 		print_header(screen *scr);



#endif
