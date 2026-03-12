#include "kernel.h"


/*
** kfs_reboot() - Reboot the machine via the 8042 keyboard controller.
**   Sending 0xFE to port 0x64 pulses the CPU RESET line.
**   because the keyboard controller has a pin connected to the CPU's reset line,
**   and 0xFE is the command to pulse that pin.
https://wiki.osdev.org/Reboot
*/
void kfs_reboot(screen *scr)
{
    (void)scr;
    /* Wait for the keyboard controller input buffer to be empty */
    uint8_t status;
    do {
        status = inb(KEYBOARD_STATUS_PORT);
    } while (status & 0x02);

    /* Send reset command to the 8042 keyboard controller */
    outb(KEYBOARD_STATUS_PORT, 0xFE);
}
