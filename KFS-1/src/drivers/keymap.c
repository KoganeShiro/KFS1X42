#include "kernel.h"

/* 
Keyboard mapping:
*/
// Scancode Set 1 mapping table (unshifted keys)
static const char scancode_to_ascii[128] = {
    0,    0,   '1',  '2',  '3',  '4',  '5',  '6',  // 0x00-0x07
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t', // 0x08-0x0F
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10-0x17
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',  // 0x18-0x1F
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20-0x27
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',  // 0x28-0x2F
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  // 0x30-0x37
    0,    ' ',  0,    0,    0,    0,    0,    0,    // 0x38-0x3F (F1-F5)
    0,    0,    0,    0,    0,    0,    0,    '7',  // 0x40-0x47 (F6-F10, NumPad 7)
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  // 0x48-0x4F (NumPad)
    '2',  '3',  '0',  '.',  0,    0,    0,    0,    // 0x50-0x57 (NumPad, F11-F12)
    0,    0,    0,    0,    0,    0,    0,    0     // 0x58-0x5F
    // Rest are zeros (0x60-0x7F)
};

// Shifted characters table
static const char scancode_to_ascii_shifted[128] = {
    0,    0,   '!',  '@',  '#',  '$',  '%',  '^',  // 0x00-0x07
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t', // 0x08-0x0F
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10-0x17
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',  // 0x18-0x1F
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20-0x27
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',  // 0x28-0x2F
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',  // 0x30-0x37
    0,    ' ',  0,    0,    0,    0,    0,    0,    // 0x38-0x3F
    0,    0,    0,    0,    0,    0,    0,    '7',  // 0x40-0x47
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  // 0x48-0x4F
    '2',  '3',  '0',  '.',  0,    0,    0,    0,    // 0x50-0x57
    0,    0,    0,    0,    0,    0,    0,    0     // 0x58-0x5F
};


/*
** keymap_translate()
keyboard mapping table
Convert the scancode to a character (e.g., 'a', '1', etc.)
*/
char keymap_translate(uint8_t scancode, int shift_pressed)
{
    if (scancode > 127) {
        return (0); // Invalid scancode
    }
    if (shift_pressed) {
        return (scancode_to_ascii_shifted[scancode]);
    } else {
        return (scancode_to_ascii[scancode]);
    }
    return (0);
}

