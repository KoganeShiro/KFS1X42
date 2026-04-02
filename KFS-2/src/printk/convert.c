#include "kernel.h"

static char* 	_zero_case(char* buffer)
{
	int i = 0;

	buffer[i++] = '0';
	buffer[i] = '\0';
	return (buffer);
}

char* _reverse_str(char* buffer)
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

char* 	itoa(int value, char* buffer)
{
	int i = 0;

	if (value == 0) {
		return (_zero_case(buffer));
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
	return (_reverse_str(buffer));
}

/*
** Convert unsigned int to hexadecimal string
** uppercase: 1 for A-F, 0 for a-f
*/
char* itoh(unsigned int value, char* buffer, char *hexa)
{
	int i = 0;
	
	if (value == 0) {
		return (_zero_case(buffer));
	}
	// Extract hex digits (in reverse)
	while (value > 0) {
		buffer[i++] = hexa[value & 0xF];  // Get last 4 bits
		value >>= 4;                         // Shift right by 4 bits
	}
	buffer[i] = '\0';
	// Reverse the string
	return (_reverse_str(buffer));
}
