#include "kernel.h"

//implement the halt command

/*
** kfs_halt() - Halt the machine.
**   This function will cause the machine to stop executing instructions.
**   It will be like to put the PC to a stop : guarantees that all hardware devices are stopped the same way as a regular shutdown (without shuting down the power)
**   The main difference is that it will not shut off the power supply (meaning the machine will not turn on if we press the power button again, 
**    we will need to reset it manually by pressing the reset button or by unplugging and plugging the battery).
** https://forums.linuxmint.com/viewtopic.php?t=395982
*/
void kfs_halt(screen *scr)
{
    printk(scr, LOG_LEVEL_WARNING "Halting the machine...\n");
    uint8_t temp;

    hide_cursor(scr);
    asm volatile ("cli"); /* disable all interrupts */
    asm volatile ("hlt"); /* halt the CPU */
}
