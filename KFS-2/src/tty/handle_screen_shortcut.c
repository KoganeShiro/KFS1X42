#include "kernel.h"

static bool is_alt_pressed(uint8_t scancode, bool alt_pressed)
{
    if (scancode == KEY_LALT) {
        return (true); // Alt key pressed
    } else if (scancode == (KEY_LALT + 0x80)) {
        return (false); // Alt key released
    }
    return (alt_pressed); // No change in alt state
}


/*
** handle_screen_shortcut()
Switch screens when alt+1, alt+2, etc. are pressed.
*/
bool handle_screen_shortcut(kfs *k, uint8_t scancode)
{
    static bool alt_pressed = false;
    
    alt_pressed = is_alt_pressed(scancode, alt_pressed);

    // Alt+1 (0x02) → screen 0, Alt+9 (0x0A) → screen 8
    if (alt_pressed && (scancode >= 0x02 && scancode <= 0x0A)) {
        int screen_index = scancode - 0x02; 
        switch_screen(k, screen_index);
        return (true); 
    }
    return (false);
}
