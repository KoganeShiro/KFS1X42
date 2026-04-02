/*
 * load_gdt.s - Global Descriptor Table Loading Routine
 *
 * This assembly file exposes the load_gdt function to our C code.
 * It takes the pointer to our gdt_ptr structure, loads it into the CPU
 * using the lgdt instruction, and then updates all segment registers
 * and flushes the pipeline with a far jump.
 */

.global load_gdt
.type load_gdt, @function

load_gdt:
	/* The C compiler pushes the pointer argument onto the stack.
	   [esp + 4] = gdt_ptr pointer.
	   [esp]     = Return address to C code.
	*/
	movl 4(%esp), %eax

	/* Load the GDT pointer into the CPU's GDTR register */
	lgdt (%eax)

	/* Now we need to reload the data segment registers.
	   0x00 = Null Descriptor
	   0x08 = Kernel Code Segment (Index 1: 1 * 8 bytes = 0x08)
	   -> 0x10 = Kernel Data Segment (Index 2: 2 * 8 bytes = 0x10)
	*/
	movw $0x10, %ax
	movw %ax, %ds    /* Data Segment */
	movw %ax, %es    /* Extra Segment */
	movw %ax, %fs    /* Extra Segment */
	movw %ax, %gs    /* Extra Segment */
	movw %ax, %ss    /* Stack Segment */

	/* We cannot simply move 0x08 into %cs (Code Segment).
	   The only way to update %cs is to perform a "Far Jump".
	   This far jump also flushes the CPU pipeline of any pre-fetched 
	   instructions that might have been decoded using the old GDT.
	*/
	ljmp $0x08, $flush

flush:
	/* Returning to the C code! */
	ret
