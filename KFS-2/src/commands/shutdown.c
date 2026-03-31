#include "kernel.h"


/*
** kfs_shutdown() - Power off the machine.
**   They are no universal hardware instruction for power off but emulators do provide a way to shut down.
**   QEMU >= 2.0 ACPI shutdown: outw(0x604, 0x2000); 
https://wiki.osdev.org/Shutdown
*/
void kfs_shutdown(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Shutting down the system...\n");

    /* QEMU >= 2.0 ACPI shutdown */
    outw(0x604, 0x2000);
}

