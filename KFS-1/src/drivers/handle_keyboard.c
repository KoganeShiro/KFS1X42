#include "kernel.h"

// Check if there's a character at the given position (not '\0')
static int has_char_at(screen *scr, int cursor_pos)
{
    uint16_t entry = scr->buffer[cursor_pos];
    char c = (char)(entry & GET_LOWER_8B_MASK);  // Extract character from VGA entry
    return (c != '\0');
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
//TODO: move the remain char to the left
static void handle_backspace(screen *scr)
{
    if (scr->column > 0) {
        scr->column--;
    }
    kfs_putentry(scr, '\0');
    return;
}

// Handle arrow keys: move cursor only through existing text
static int handle_arrow_keys(screen *scr, uint8_t scancode)
{
    switch (scancode) {
        case KEY_UP:
        case KEY_DOWN:
            return (1);
        case KEY_LEFT:
            if (scr->column > 0 && has_char_at(scr, scr->cursor_pos - 1)) {
                scr->column--;
            }
            return (1);
        case KEY_RIGHT:
            if (scr->column < VGA_WIDTH - 1 && has_char_at(scr, scr->cursor_pos)) {
                scr->column++;
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
static void process_scancode(screen *scr, uint8_t scancode)
{
    static int shift_pressed = 0;

    shift_pressed = is_shift_pressed(scancode, shift_pressed);
    if (scancode == KEY_ENTER) {
        kfs_putchar(scr, '\n');
        return;
    }
    if (scancode == KEY_BACKSPACE) {
        handle_backspace(scr);
        return;
    }
    if (handle_arrow_keys(scr, scancode) == 1) {
        return; 
    }

    char c = keymap_translate(scancode, shift_pressed);
    if (c) {
        kfs_putchar(scr, c);
    }
}

/*
** handle_keyboard_keypress()
call get_scancode() in a loop, and call process_scancode() for each scancode read.
*/
void handle_keyboard_keypress(kfs *k)
{
    while (1) {
        uint8_t scancode = get_scancode();
        
        // Only process if we got a valid scancode (not 0)
        if (scancode != 0) {
            if (handle_screen_shortcut(k, scancode)) {
                continue; // If it was a screen shortcut, skip normal processing
            }
            screen *scr = kfs_active_screen(k);
            process_scancode(scr, scancode);
            update_cursor_pos(scr);
        }
    }
}

