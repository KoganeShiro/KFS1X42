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
** keyboard_data_ready() - Check if new scancode is available
** Port 0x64 is the keyboard status register:
**   Bit 0 (0x01) = Output buffer status
**     1 = Buffer full (data ready to read from port 0x60)
**     0 = Buffer empty (no new data)
*/
static inline int keyboard_data_ready(void)
{
    return (inb(KEYBOARD_STATUS_PORT) & 0x01);
}

/*
** get_scancode() - Read scancode only if new data is ready
** Returns scancode if available, 0 if no new data
*/
static inline uint8_t get_scancode(void)
{
    if (keyboard_data_ready()) {
        return (inb(KEYBOARD_PORT));
    }
    return (0);
}

/*
** map_scancode()
keyboard mapping table
Convert the scancode to a character (e.g., 'a', '1', etc.)
*/
static char map_scancode(uint8_t scancode, int shift_pressed)
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

static int is_shift_pressed(uint8_t scancode, int shift_pressed)
{
    if (shift_pressed == 1 && scancode == KEY_CAPSLOCK) {
        return (0); // Caps Lock toggles shift state
    } else if (shift_pressed == 0 && scancode == KEY_CAPSLOCK) {
        return (1); // Caps Lock toggles shift state
    } else if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
        return (1); // Shift key pressed
    } else if (scancode == (KEY_LSHIFT | KEY_RELEASE_MASK) || scancode == (KEY_RSHIFT | KEY_RELEASE_MASK)) {
        return (0); // Shift key released
    }
    return (shift_pressed); // No change in shift state
}

// Handle backspace: move cursor back and print space
//TODO: handle backspace when at the beginning of a line (move to end of previous line)
static void handle_backspace(vga_terminal *terminal)
{
    if (terminal->column > 0) {
        terminal->column--;
    }
    kfs_putentry_at(terminal, '\0');
    return;
}

// Check if there's a character at the given position (not '\0')
static int has_char_at(vga_terminal *terminal, int cursor_pos)
{
    uint16_t entry = terminal->buffer[cursor_pos];
    char c = (char)(entry & GET_LOWER_8B_MASK);  // Extract character from VGA entry
    return (c != '\0');
}

// Handle arrow keys: move cursor only through existing text
static int handle_arrow_keys(vga_terminal *terminal, uint8_t scancode)
{
    switch (scancode) {
        case KEY_UP:
        case KEY_DOWN:
            return (1);
        case KEY_LEFT:
            if (terminal->column > 0 && has_char_at(terminal, terminal->cursor_pos - 1)) {
                terminal->column--;
            }
            return (1);
        case KEY_RIGHT:
            if (terminal->column < VGA_WIDTH - 1 && has_char_at(terminal, terminal->cursor_pos)) {
                terminal->column++;
            }
            return (1);
        default:
            return (0);
    }
}

/*
** process_scancode()
Handle special keys (Enter, Backspace, Arrow keys).
Call map_scancode() and print it.
*/
static void process_scancode(vga_terminal *terminal, uint8_t scancode)
{
    static int shift_pressed = 0;

    shift_pressed = is_shift_pressed(scancode, shift_pressed);
    if (scancode == KEY_ENTER) {
        kfs_putchar(terminal, '\n');
        return;
    }
    if (scancode == KEY_BACKSPACE) {
        handle_backspace(terminal);
        return;
    }
    if (handle_arrow_keys(terminal, scancode) == 1) {
        return; 
    }

    char c = map_scancode(scancode, shift_pressed);
    if (c) {
        kfs_putchar(terminal, c);
    }
}

/*
** handle_keyboard_keypress()
call get_scancode() in a loop, and call process_scancode() for each scancode read.
*/
void handle_keyboard_keypress(vga_terminal *terminal)
{
    while (1) {
        uint8_t scancode = get_scancode();
        
        // Only process if we got a valid scancode (not 0)
        if (scancode != 0) {
            if (handle_screen_shortcut(terminal, scancode)) {
                continue; // If it was a screen shortcut, skip normal processing
            }
            process_scancode(terminal, scancode);
            update_cursor_pos(terminal);
        }
    }
}

