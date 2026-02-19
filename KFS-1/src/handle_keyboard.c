#include "kernel.h"

/*
** inb() - Read byte from I/O port
    The inb instruction will write its result into the AL register,
        and that value should be copied into the ret variable
* inb is an x86 assembly instruction meaning "input byte"
* %1 is the port number (stored in DX register or as immediate value)
* %0 is where the result goes
    (A register (AL for 8-bit, AX for 16-bit, EAX for 32-bit), which is ret)
*/
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}

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
** map_scancode()
keyboard mapping table
Convert the scancode to a character (e.g., 'a', '1', etc.)
*/
char map_scancode(uint8_t scancode, int shift_pressed)
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

/*
** process_scancode()
Call map_scancode() and print it.
*/
void process_scancode(vga_terminal *terminal, uint8_t scancode)
{
    static int shift_pressed = 0;

    // Check for Shift key press/release
    //CAPSLOCK is not working
    if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT || scancode == KEY_CAPSLOCK) {
        shift_pressed = 1;
        return;
    } else if (scancode == KEY_LSHIFT + 0x80 || scancode == KEY_RSHIFT + 0x80 || scancode == KEY_CAPSLOCK + 0x80) { 
        shift_pressed = 0;
        return;
    }

    if (scancode == KEY_ENTER) {
        kfs_putchar(terminal, '\n');
        return;
    }

    if (scancode == KEY_BACKSPACE) {
        // Handle backspace: move cursor back and print space
        if (terminal->column > 0) {
            terminal->column--;
        } else if (terminal->row > 0) {
            terminal->row--;
            terminal->column = VGA_WIDTH - 1;
        }
        kfs_putentry_at(terminal, ' '); // Clear the character
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
    //init cursor position
    while (1) {
        uint8_t scancode = get_scancode();
        
        // Only process if we got a valid scancode (not 0)
        if (scancode != 0) {
            if (handle_screen_shortcut(terminal, scancode)) {
                continue; // If it was a screen shortcut, skip normal processing
            }
            process_scancode(terminal, scancode);
            //update cursor
        }
    }
}




/*
** update_cursor()
Update terminal->row and terminal->column if arrow keys are pressed.
*/


/*
** handle_screen_shortcut()
Switch screens when alt+1, alt+2, etc. are pressed.
*/
int handle_screen_shortcut(vga_terminal *terminal, uint8_t scancode)
{
    static int alt_pressed = 0;
    
    // Track Alt key state
    if (scancode == KEY_LALT) {
        alt_pressed = 1;
        return (1); // Return 1 to indicate a screen shortcut was processed
    } else if (scancode == KEY_LALT + 0x80) {
        alt_pressed = 0;
        return (0); // Return 0 to indicate no screen shortcut was processed
    }
    
    //skip 0 because the scancode would be to munch of a hassle to process
    if (alt_pressed && (scancode >= 0x02 && scancode <= 0x0A)) {
        int screen_index = scancode - 0x01; 
        printk(terminal, "Switching to screen %d\n", screen_index);
        //switch_screen(terminal, screen_index);
        return (1); // Return 1 to indicate a screen shortcut was processed
    }
    return (0); // Return 0 if no screen shortcut was processed
}
