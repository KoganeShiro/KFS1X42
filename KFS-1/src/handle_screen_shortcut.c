#include "kernel.h"

static int is_alt_pressed(uint8_t scancode, int alt_pressed)
{
    if (scancode == KEY_LALT) {
        return (1); // Alt key pressed
    } else if (scancode == (KEY_LALT + 0x80)) {
        return (0); // Alt key released
    }
    return (alt_pressed); // No change in alt state
}


/*
** handle_screen_shortcut()
Switch screens when alt+1, alt+2, etc. are pressed.
*/
int handle_screen_shortcut(vga_terminal *terminal, uint8_t scancode)
{
    static int alt_pressed = 0;
    
    alt_pressed = is_alt_pressed(scancode, alt_pressed);

    //skip 0 because the scancode would be to munch of a hassle to process
    if (alt_pressed && (scancode >= 0x02 && scancode <= 0x0A)) {
        int screen_index = scancode - 0x01; 
        printk(terminal, "Switching to screen %d\n", screen_index);
        //switch_screen(terminal, screen_index);
        return (1); // Return 1 to indicate a screen shortcut was processed
    }
    return (0); // Return 0 if no screen shortcut was processed
}
