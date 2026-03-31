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

static bool is_shift_pressed(uint8_t scancode, bool shift_pressed)
{
    if (shift_pressed == true && scancode == KEY_CAPSLOCK) {
        return (false); 
    } else if (shift_pressed == false && scancode == KEY_CAPSLOCK) {
        return (true);
    } else if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
        return (true); 
    } else if (scancode == (KEY_LSHIFT | KEY_RELEASE_MASK) || scancode == (KEY_RSHIFT | KEY_RELEASE_MASK)) {
        return (false);
    }
    return (shift_pressed);
}

static void shift_left_from_cursor(screen *scr)
{
    // Calculate buffer positions
    int line_start = scr->row * VGA_WIDTH + scr->column;
    int line_end   = scr->row * VGA_WIDTH + (VGA_WIDTH - 1);

    // Shift everything from (column+1) to end-of-line LEFT by one slot
    memmove(&scr->buffer[line_start],
            &scr->buffer[line_start + 1],
            (line_end - line_start) * sizeof(uint16_t));

    scr->buffer[line_end] = vga_entry('\0', scr->color);
}

// Handle backspace: move cursor back, shift VGA line, and update cmdline buffer
static void handle_backspace(screen *scr)
{
    if (scr->column <= 3 && scr->shell_mode == true) {
        return;
    }
    if (scr->column > 0) {
        scr->column--;
        shift_left_from_cursor(scr);

        /* Keep cmdline_buffer in sync with what's on screen */
        if (scr->cmdline_index > 0) {
            scr->cmdline_index--;
            scr->cmdline_buffer[scr->cmdline_index] = '\0';
        }
    }
    return;
}

// Handle arrow keys: move cursor only through existing text
static int handle_arrow_keys(screen *scr, uint8_t scancode)
{
    switch (scancode) {
        case KEY_UP:
        case KEY_DOWN:
        // command history like readline ?
            return (1);
        case KEY_LEFT:
            if (scr->column <= 3 && scr->shell_mode == true) {
                return (1);
            }
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
        command_parser(scr);
        return;
    }
    if (scancode == KEY_BACKSPACE) {
        handle_backspace(scr);
        return;
    }
    if (handle_arrow_keys(scr, scancode) == 1) {
        return; 
    }
    
    if (scancode & KEY_RELEASE_MASK)
        return;

    char c = keymap_translate(scancode, shift_pressed);
    if (c) {
        add_char_to_cmdline_buffer(scr, c);
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
        
        if (scancode == 0) {
            /* No key pending — hint to the CPU that this is a spin-wait loop. */
            asm volatile("pause");
            continue;
        }
        else {
            if (handle_screen_shortcut(k, scancode) == true) {
                continue; // If it was a screen shortcut, skip normal processing
            }
            if (handle_scroll_key(k, scancode) == true) {
                continue; // If it was a scroll key, skip normal processing
            }
            screen *scr = kfs_active_screen(k);
            if (scr->cursor_visible == true) {
                process_scancode(scr, scancode);
                update_cursor_pos(scr);
            }
        }
    }
}

