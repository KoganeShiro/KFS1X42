#include "kernel.h"

// implement the print stack things command

// could print the gdt table too

//could also print other content of the gdt table memory
//print like a table
void print_gdt(screen *scr, gdt *gdt)
{
    printk(scr, LOG_LEVEL_INFO "GDT Base: 0x%X\n", gdt->ptr.base);
    printk(scr, LOG_LEVEL_INFO "GDT Limit: 0x%X\n", gdt->ptr.limit);
    for (int i = 0; i < NB_OF_GDT_ENTRY; i++) {
        descriptor *desc = &gdt->entries[i];
        printk(scr, "Entry %d: Base=0x%X%X%X Limit=0x%X%X Access=0x%X Granularity=0x%X\n",
               i,
               desc->base_high, desc->base_middle, desc->base_low, desc->limit_low,
               desc->access,
               desc->granularity);
    }
}

// For stack, we could print the current stack pointer value and maybe a few values from the stack memory
void print_stack(screen *scr, gdt *gdt)
{
    printk(scr, LOG_LEVEL_INFO "Printing stack... TODO\n");
    (void)gdt; // to avoid unused parameter warning for now
}