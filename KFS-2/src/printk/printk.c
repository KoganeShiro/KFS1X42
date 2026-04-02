#include "kernel.h"

static int _kfs_putstr(screen *scr, const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		kfs_putchar(scr, data[i]);

    return (size);
}

static int _kfs_putint(screen *scr, int value) 
{
	char buffer[12];

	itoa(value, buffer);
	return (_kfs_putstr(scr, buffer, strlen(buffer)));
}

static int _kfs_puthex(screen *scr, unsigned int value, char *hexa, int width, char pad_char) 
{
	char buffer[32];
	int printed = 0;

	itoh(value, buffer, hexa);
	int len = strlen(buffer);
	
	while (len < width) {
		printed += kfs_putchar(scr, pad_char);
		width--;
	}
	printed += _kfs_putstr(scr, buffer, len);
	return (printed);
}

static int _kfs_putptr(screen *scr, void *ptr) 
{
	unsigned int addr = (unsigned int)ptr;
	char buffer[32];
	int length = 0;

	length += _kfs_putstr(scr, "0x", 2);
	itoh(addr, buffer, "0123456789abcdef");
	length += _kfs_putstr(scr, buffer, strlen(buffer));
	return (length);
}

/*
** Check if string starts with given prefix
*/
static bool _str_starts_with(const char* data, const char* log_level)
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
	if (_str_starts_with(data, LOG_LEVEL_INFO)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
		length = _kfs_putstr(scr, LOG_LEVEL_INFO, strlen(LOG_LEVEL_INFO));
	}
	else if (_str_starts_with(data, LOG_LEVEL_DEBUG)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		length = _kfs_putstr(scr, LOG_LEVEL_DEBUG, strlen(LOG_LEVEL_DEBUG));
	}
	else if (_str_starts_with(data, LOG_LEVEL_WARNING)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
		length = _kfs_putstr(scr, LOG_LEVEL_WARNING, strlen(LOG_LEVEL_WARNING));
	}
	else if (_str_starts_with(data, LOG_LEVEL_ERROR)) {
		kfs_set_color(scr, vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
		length = _kfs_putstr(scr, LOG_LEVEL_ERROR, strlen(LOG_LEVEL_ERROR));
	}
	return (length);
}

static int format_specifier_handler(screen *scr, char specifier, va_list *args, int width, char pad_char)
{
	switch (specifier) {
		case 'd':
		case 'i': {
			return (_kfs_putint(scr, va_arg(*args, int)));
		}
		case 's': {
			const char* str = va_arg(*args, const char*);
			return (_kfs_putstr(scr, str, strlen(str)));
		}
		case 'c': {
			char c = (char)va_arg(*args, int);
			return (kfs_putchar(scr, c));
		}
		case 'x': {
			return (_kfs_puthex(scr, va_arg(*args, unsigned int), "0123456789abcdef", width, pad_char));
		}
		case 'X': {
			return (_kfs_puthex(scr, va_arg(*args, unsigned int), "0123456789ABCDEF", width, pad_char));
		}
		case 'p': {
			return (_kfs_putptr(scr, va_arg(*args, void*)));
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
			
			char pad_char = ' ';
			int width = 0;
			
			if (data[i] == '0') {
				pad_char = '0';
				i++;
			}
			
			while (data[i] >= '0' && data[i] <= '9') {
				width = (width * 10) + (data[i] - '0');
				i++;
			}
			
			length += format_specifier_handler(scr, data[i], &args, width, pad_char);
		} else {
			length += kfs_putchar(scr, data[i]);
		}
	}
	va_end(args);
	scr->color = saved_color;
	update_cursor_pos(scr);
    return (length);
}

