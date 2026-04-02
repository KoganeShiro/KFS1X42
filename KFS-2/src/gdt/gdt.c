#include "kernel.h"

#define GDT_FLAT_BASE   0x00000000
#define GDT_FLAT_LIMIT  0xFFFFF
#define GDT_LOWER_16    0xFFFF
#define GDT_BYTE_MASK   0xFF
#define GDT_NIBBLE_MASK 0x0F

// Helper function to build a single descriptor entry
static void _fill_gdt(descriptor *entry, uint32_t base, uint32_t limit, gdt_access access, gdt_granularity gran)
{
	entry->base_low = (base & GDT_LOWER_16);
	entry->base_middle = (base >> 16) & GDT_BYTE_MASK;
	entry->base_high = (base >> 24) & GDT_BYTE_MASK;

	entry->limit_low = (limit & GDT_LOWER_16);
	
	entry->access = access;
	
	entry->granularity = gran;
	// Inject the 4 highest bits of the 20-bit limit into the existing granularity properties
	entry->granularity.limit_high = (limit >> 16) & GDT_NIBBLE_MASK;
}

static void _setup_kernel_segments(descriptor *gdt_table, gdt_granularity std_gran)
{
	// Kernel Code Segment (Ring 0)
	gdt_access kernel_code_acc = { 
		.accessed = 0, 
		.read_write = 1, .conforming = 0, .executable = 1, 
		.descriptor_type = 1, 
		.privilege_level = 0, .present = 1 };
	_fill_gdt(&gdt_table[1], GDT_FLAT_BASE, GDT_FLAT_LIMIT, kernel_code_acc, std_gran);

	// Kernel Data Segment (Ring 0)
	gdt_access kernel_data_acc = { .accessed = 0, .read_write = 1, .conforming = 0, .executable = 0, .descriptor_type = 1, .privilege_level = 0, .present = 1 };
	_fill_gdt(&gdt_table[2], GDT_FLAT_BASE, GDT_FLAT_LIMIT, kernel_data_acc, std_gran);

	// Kernel Stack Segment (Ring 0)
	gdt_access kernel_stack_acc = { .accessed = 0, .read_write = 1, .conforming = 0, .executable = 0, .descriptor_type = 1, .privilege_level = 0, .present = 1 };
	_fill_gdt(&gdt_table[3], GDT_FLAT_BASE, GDT_FLAT_LIMIT, kernel_stack_acc, std_gran);
}

static void _setup_user_segments(descriptor *gdt_table, gdt_granularity std_gran)
{
	// User Code Segment (Ring 3)
	gdt_access user_code_acc = { .accessed = 0, .read_write = 1, .conforming = 0, .executable = 1, .descriptor_type = 1, .privilege_level = 3, .present = 1 };
	_fill_gdt(&gdt_table[4], GDT_FLAT_BASE, GDT_FLAT_LIMIT, user_code_acc, std_gran);

	// User Data Segment (Ring 3)
	gdt_access user_data_acc = { .accessed = 0, .read_write = 1, .conforming = 0, .executable = 0, .descriptor_type = 1, .privilege_level = 3, .present = 1 };
	_fill_gdt(&gdt_table[5], GDT_FLAT_BASE, GDT_FLAT_LIMIT, user_data_acc, std_gran);

	// User Stack Segment (Ring 3)
	gdt_access user_stack_acc = { .accessed = 0, .read_write = 1, .conforming = 0, .executable = 0, .descriptor_type = 1, .privilege_level = 3, .present = 1 };
	_fill_gdt(&gdt_table[6], GDT_FLAT_BASE, GDT_FLAT_LIMIT, user_stack_acc, std_gran);
}

void init_gdt(kfs *k)
{
	// We want to place the GDT exactly at 0x800 
	descriptor *gdt_table = (descriptor *)GDT_ADDR;

	// 0: Null Descriptor (Required by CPU)
	gdt_access null_access = {0};
	gdt_granularity null_gran = {0};
	_fill_gdt(&gdt_table[0], 0, 0, null_access, null_gran);

	/* 
	 * Standard Granularity for all our segments:
	 * 32-bit protected mode (size_flag=1)
	 * 4KB page sizing (granularity=1) -> our limit of 0xFFFFF is multiplied by 4KB to reach 4GB
	*/
	gdt_granularity std_gran = {
		.limit_high  = 0, // overwritten in fill_gdt
		.available   = 0,
		.long_mode   = 0, // 0 for 32-bit
		.size_flag   = 1, // 1 for 32-bit 
		.granularity = 1  // Multiply limit by 4KB
	};
	_setup_kernel_segments(gdt_table, std_gran);
	_setup_user_segments(gdt_table, std_gran);

	// Setup the pointer struct for lgdt instruction
	k->gdt->ptr.limit = (sizeof(descriptor) * NB_OF_GDT_ENTRY) - 1;
	k->gdt->ptr.base = GDT_ADDR;
}

