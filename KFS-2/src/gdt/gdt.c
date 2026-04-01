#include "kernel.h"

// void init_gdt(kfs *k);
void init_gdt(kfs *k)
{
    //fill each entries by calling fill_gdt() for each entry
}

// void fill_gdt(kfs *k);
void fill_gdt(descriptor *k)
{
    //fill the GDT entries with appropriate values for code segment, data segment, etc.
}

