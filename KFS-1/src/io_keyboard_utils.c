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
uint8_t inb(uint16_t port)
{
    uint8_t ret;

    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}

void outb(uint16_t port, uint8_t val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void outw(uint16_t port, uint16_t val)
{
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}
