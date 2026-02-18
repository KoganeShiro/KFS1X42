#include "kernel.h"

/*
** Packs two 4-bit colors (foreground + background) into one byte.
*/
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

/*
** 1. Reset row and column to 0
** 2. Set default color (light grey on black)
** 3. Point the buffer to VGA_MEMORY (0xB8000)
** 4. Loop through every cell (80 × 25 = 2000 cells)
**    and fill each with a space character + default color
**    → this clears the screen
*/
void kfs_initialize(vga_terminal *terminal) 
{
	terminal->buffer = (uint16_t*)VGA_MEMORY;
	terminal->row = 0;
	terminal->column = 0;
	terminal->color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal->buffer[index] = vga_entry(' ', terminal->color);
		}
	}
}



void kernel_main(void) 
{
	vga_terminal terminal;

	/* Initialize terminal interface */
	kfs_initialize(&terminal);

	/* Newline support is left as an exercise. */
	ft_printk(&terminal, "Lorem ipsum dolor sit amet Donec orci nisl, placerat sed diam eget, semper hendrerit lacus. Nulla mi turpis, consequat eu felis non, dignissim vestibulum lorem. Proin tincidunt molestie est, at euismod felis molestie vitae. Donec at viverra enim, eget rutrum turpis. Sed molestie sed risus quis tincidunt. Nullam mattis ullamcorper est sit amet facilisis. Maecenas rutrum justo a eros ullamcorper, ut pulvinar velit eleifend. Integer pharetra semper efficitur. Quisque auctor vel tortor varius maximus. Vivamus congue commodo mi. Nullam at neque nec leo dapibus ultrices. Praesent sed lectus eget augue porta mollis. Phasellus pharetra malesuada erat id venenatis. Curabitur at tortor ante. Quisque fermentum vel nulla ac sagittis. Fusce ac bibendum urna, ac volutpat lectus. Quisque sapien nibh, ultricies vitae elementum ac, efficitur eu mi. Vestibulum imperdiet feugiat efficitur. Nam arcu massa, pulvinar in dolor eget, fermentum malesuada est. Donec erat urna, dapibus eu convallis at, mattis sed justo. Donec ultricies, mauris sed elementum aliquet, velit neque sodales metus, at pharetra ex turpis sed orci. Pellentesque tempus dolor eget tempus sodales. Donec varius dolor a risus mattis, at rutrum mi iaculis. Praesent sed sem nec quam mattis hendrerit. Nunc fermentum, nibh ac posuere fermentum, risus augue consectetur magna, quis tincidunt nisl urna egestas orci. Praesent at porttitor dolor. Nunc euismod dui quis efficitur laoreet. Phasellus vestibulum felis mauris, et posuere massa dignissim quis. Aenean in lacinia nisi, quis congue nulla. \n");
}
