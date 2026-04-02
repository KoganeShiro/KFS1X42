#include "kernel.h"

void print_gdt(screen *scr, gdt *gdt)
{
    printk(scr, LOG_LEVEL_INFO "GDT Base: 0x%08X | Limit: 0x%04X\n", gdt->ptr.base, gdt->ptr.limit);
    printk(scr, "+-------+------------+---------+--------+--------+\n");
    printk(scr, "| Index |    Base    |  Limit  | Access |  Gran  |\n");
    printk(scr, "+-------+------------+---------+--------+--------+\n");

    for (int i = 0; i < NB_OF_GDT_ENTRY; i++) {
        descriptor *desc = &gdt->entries[i];

        // Safely extract the raw bytes for the flags
        uint8_t access_byte = *(uint8_t*)&desc->access;
        uint8_t gran_byte = *(uint8_t*)&desc->granularity;
        
        // Reconstruct the full 32-bit Base and 20-bit Limit from the scattered fields
        uint32_t base = (desc->base_high << 24) | (desc->base_middle << 16) | desc->base_low;
        uint32_t limit = ((gran_byte & 0x0F) << 16) | desc->limit_low;
        
        printk(scr, "|   %d   | 0x%08X | 0x%05X |  0x%02X  |  0x%02X  |\n",
               i, base, limit, access_byte, gran_byte);
    }
    printk(scr, "+-------+------------+---------+--------+--------+\n");
}

// For stack, we could print the current stack pointer value and maybe a few values from the stack memory
void print_stack(screen *scr, gdt *current_gdt)
{
    uint32_t esp;
    uint32_t ebp;
    uint16_t ss;

    // Fetch the CPU Stack Segment (SS), Stack Pointer (ESP) and Base Pointer (EBP)
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));
    asm volatile("mov %%ss,  %0" : "=r"(ss));

    // The SS register holds a "Selector" (an index into the GDT, shifted left by 3)
    uint16_t gdt_index = ss >> 3;
    descriptor *stack_desc = &current_gdt->entries[gdt_index];
    uint32_t base = (stack_desc->base_high << 24) | (stack_desc->base_middle << 16) | stack_desc->base_low;

    printk(scr, LOG_LEVEL_INFO "SS: 0x%04X -> GDT Entry %d (Base 0x%08X)\n", ss, gdt_index, base);

    // Print visually like a stack mapping memory
    printk(scr, "      Address      Value\n");
    printk(scr, "      ----------   ----------\n");

    uint32_t *stack_memory = (uint32_t *)esp;

    // Visual representation: The stack "grows downwards" in physical addresses. 
    // We will start at an older (higher) address, and loop down to ESP (lowest address).
    for (int i = 5; i >= 0; i--) {
        uint32_t current_addr = (uint32_t)(stack_memory + i);
        
        // Output visual pointers beside the memory addresses
        if (current_addr == ebp) {
            printk(scr, " EBP> ");
        } else if (current_addr == esp) {
            printk(scr, " ESP> ");
        } else {
            printk(scr, "      ");
        }

        printk(scr, "[0x%08X] | 0x%08X |\n", current_addr, stack_memory[i]);
    }
    printk(scr, "                   ----------\n");
}