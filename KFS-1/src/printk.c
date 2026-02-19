#include "kernel.h"

static char* reverse_str(char* buffer)
{
	int i = 0;
	while (buffer[i] != '\0') {
		i++;
	}
	for (int j = 0, k = i - 1; j < k; j++, k--) {
		char temp = buffer[j];
		buffer[j] = buffer[k];
		buffer[k] = temp;
	}
	return (buffer);
}

static char* 	zero_case(char* buffer)
{
	int i = 0;

	buffer[i++] = '0';
	buffer[i] = '\0';
	return (buffer);
}

static char* 	itoa(int value, char* buffer)
{
	int i = 0;

	if (value == 0) {
		return (zero_case(buffer));
	}
	if (value < 0) {
		value = -value;
		buffer[i++] = '-';
	}
	
	while (value > 0) {
		buffer[i++] = (value % 10) + '0';
		value /= 10;
	}
	buffer[i] = '\0';
	return (reverse_str(buffer));
}

/*
** Convert unsigned int to hexadecimal string
** uppercase: 1 for A-F, 0 for a-f
*/
static char* itoh(unsigned int value, char* buffer, char *hexa)
{
	int i = 0;
	
	if (value == 0) {
		return zero_case(buffer);
	}
	// Extract hex digits (in reverse)
	while (value > 0) {
		buffer[i++] = hexa[value & 0xF];  // Get last 4 bits
		value >>= 4;                         // Shift right by 4 bits
	}
	buffer[i] = '\0';
	// Reverse the string
	return (reverse_str(buffer));
}


static void kfs_putstr(vga_terminal *terminal, const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		kfs_putchar(terminal, data[i]);

    return;
}

/*
** Check if string starts with given prefix
*/
static bool starts_with(const char* data, const char* log_level)
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
static size_t parse_log_level(vga_terminal *terminal, const char* data)
{
	int length = 0;
	if (starts_with(data, LOG_LEVEL_INFO)) {
		kfs_set_color(terminal, vga_entry(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
		length = strlen(LOG_LEVEL_INFO);
		kfs_putstr(terminal, LOG_LEVEL_INFO, length);
	}
	else if (starts_with(data, LOG_LEVEL_DEBUG)) {
		kfs_set_color(terminal, vga_entry(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
		length = strlen(LOG_LEVEL_DEBUG);
		kfs_putstr(terminal, LOG_LEVEL_DEBUG, length);
	}
	else if (starts_with(data, LOG_LEVEL_WARNING)) {
		kfs_set_color(terminal, vga_entry(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
		length = strlen(LOG_LEVEL_WARNING);
		kfs_putstr(terminal, LOG_LEVEL_WARNING, length);
	}
	else if (starts_with(data, LOG_LEVEL_ERROR)) {
		kfs_set_color(terminal, vga_entry(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
		length = strlen(LOG_LEVEL_ERROR);
		kfs_putstr(terminal, LOG_LEVEL_ERROR, length);
	}
	return (length);
}

/*
* Will use the same simple format as printf in C: only %i, %d, %s, and %c.
* No float because we don't use it in the kernel
*/
int printk(vga_terminal *terminal, const char *data, ...) 
{
	va_list args;
	uint8_t saved_color = terminal->color;
	int length = 0;

	length += parse_log_level(terminal, data);
	va_start(args, data);
	for(size_t i = length; data[i] != '\0'; i++) {
		if (data[i] == '%') {
			i++;
			switch (data[i]) {
				case 'd':
				case 'i': {
					int num = va_arg(args, int);
					char buffer[32];
					itoa(num, buffer);
					kfs_putstr(terminal, buffer, strlen(buffer));
					length += strlen(buffer);
					break;
				}
				case 's': {
					const char* str = va_arg(args, const char*);
					kfs_putstr(terminal, str, strlen(str));
					length += strlen(str);
					break;
				}
				case 'c': {
					char c = (char)va_arg(args, int);
					kfs_putchar(terminal, c);
					length++;
					break;
				}
				case 'x': {
					unsigned int num = va_arg(args, unsigned int);
					char buffer[32];
					itoh(num, buffer, "0123456789abcdef"); 
					kfs_putstr(terminal, buffer, strlen(buffer));
					length += strlen(buffer);
					break;
				}
				case 'X': {
					unsigned int num = va_arg(args, unsigned int);
					char buffer[32];
					itoh(num, buffer, "0123456789ABCDEF");
					kfs_putstr(terminal, buffer, strlen(buffer));
					length += strlen(buffer);
					break;
				}
				case 'p': {
					void* ptr = va_arg(args, void*);
					unsigned int addr = (unsigned int)ptr;
					char buffer[32];
					kfs_putstr(terminal, "0x", 2);
					itoh(addr, buffer, "0123456789abcdef");
					kfs_putstr(terminal, buffer, strlen(buffer));
					length += strlen(buffer) + 2;
					break;
				}
				default:
					kfs_putchar(terminal, '%');
					kfs_putchar(terminal, data[i]);
					length += 2;
			}
		} else {
			kfs_putchar(terminal, data[i]);
			length++;
		}
	}
	va_end(args);
	terminal->color = saved_color;
    return (length);
}

