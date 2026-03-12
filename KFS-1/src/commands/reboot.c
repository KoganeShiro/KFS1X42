#include "kernel.h"


void kfs_reboot(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Rebooting the system...\n");
    // Here you would add the actual reboot code, such as sending a reset command to the keyboard controller
    // or using ACPI. For now, we'll just print a message.
}
