#include "kernel.h"

// implement the print stack things command

// could print the gdt table too

//could also print other content of the gdt table memory
void print_gdt(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Printing GDT table... TODO\n");
    
}


void print_stack(screen *scr)
{
    printk(scr, LOG_LEVEL_INFO "Printing stack... TODO\n");
}