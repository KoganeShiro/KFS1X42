#include "kernel.h"

static int kfs_putstr(screen *scr, const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		kfs_putchar(scr, data[i]);

    return (size);
}

static int kfs_putint(screen *scr, int value) 
{
	char buffer[12];

	itoa(value, buffer);
	return (kfs_putstr(scr, buffer, strlen(buffer)));
}

static int kfs_puthex(screen *scr, unsigned int value, char *hexa) 
{
	char buffer[32];

	itoh(value, buffer, hexa);
	return (kfs_putstr(scr, buffer, strlen(buffer)));
}

static int kfs_putptr(screen *scr, void *ptr) 
{
	unsigned int addr = (unsigned int)ptr;
	char buffer[32];
	int length = 0;

	length += kfs_putstr(scr, "0x", 2);
	itoh(addr, buffer, "0123456789abcdef");
	length += kfs_putstr(scr, buffer, strlen(buffer));
	return (length);
}

/*
** Check if string starts with given prefix
*/
static bool str_starts_with(const char* data, const char* log_level)
{
	while (*log_level) {
		if (*data != *log_level)
			return (false);
		data++;
		log_level++;
	}
	return (true);
}

/*
** Parse log level from format string and set terminal color
** Returns the position after the log level tag, or 0 if no tag found
-> when multiple screenes implemented, print to a different screen based on log level
*/
static size_t parse_log_level(screen *scr, const char* data)
{
	int length = 0;
	if (str_starts_with(data, LOG_LEVEL_INFO)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
		length = kfs_putstr(scr, LOG_LEVEL_INFO, strlen(LOG_LEVEL_INFO));
	}
	else if (str_starts_with(data, LOG_LEVEL_DEBUG)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		length = kfs_putstr(scr, LOG_LEVEL_DEBUG, strlen(LOG_LEVEL_DEBUG));
	}
	else if (str_starts_with(data, LOG_LEVEL_WARNING)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
		length = kfs_putstr(scr, LOG_LEVEL_WARNING, strlen(LOG_LEVEL_WARNING));
	}
	else if (str_starts_with(data, LOG_LEVEL_ERROR)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
		length = kfs_putstr(scr, LOG_LEVEL_ERROR, strlen(LOG_LEVEL_ERROR));
	}
	return (length);
}

static int format_specifier_handler(screen *scr, char specifier, va_list *args)
{
	switch (specifier) {
		case 'd':
		case 'i': {
			return (kfs_putint(scr, va_arg(*args, int)));
		}
		case 's': {
			const char* str = va_arg(*args, const char*);
			return (kfs_putstr(scr, str, strlen(str)));
		}
		case 'c': {
			char c = (char)va_arg(*args, int);
			return (kfs_putchar(scr, c));
		}
		case 'x': {
			return (kfs_puthex(scr, va_arg(*args, unsigned int), "0123456789abcdef"));
		}
		case 'X': {
			return (kfs_puthex(scr, va_arg(*args, unsigned int), "0123456789ABCDEF"));
		}
		case 'p': {
			return (kfs_putptr(scr, va_arg(*args, void*)));
		}
		default:
			return (kfs_putchar(scr, '%') + kfs_putchar(scr, specifier));
	}
}

/*
* Will use the same simple format as printf in C: only %i, %d, %s, and %c.
* No float because we don't use it in the kernel
*/
int printk(screen *scr, const char *data, ...) 
{
	va_list args;
	uint8_t saved_color = scr->color;
	int length = 0;

	length += parse_log_level(scr, data);
	va_start(args, data);
	for(size_t i = length; data[i] != '\0'; i++) {
		if (data[i] == '%') {
			i++;
			length += format_specifier_handler(scr, data[i], &args);
		} else {
			length += kfs_putchar(scr, data[i]);
		}
	}
	va_end(args);
	scr->color = saved_color;
	update_cursor_pos(scr);
    return (length);
}

