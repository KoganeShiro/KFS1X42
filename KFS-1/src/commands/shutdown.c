#include "kernel.h"

void kfs_shutdown(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Shutting down the system...\n");
    // Here you would add the actual shutdown code, such as sending ACPI commands
    // or halting the CPU. For now, we'll just print a message.
}

