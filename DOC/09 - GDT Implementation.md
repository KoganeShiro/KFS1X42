# 09 — GDT Implementation

## What is it?
The Global Descriptor Table (GDT) is a data structure used by Intel x86-family processors starting with the 80286 in order to define the characteristics of the various memory areas used during program execution, including the base address, the size, and access privileges like executability and writability.

In our kernel, we are forced to use the GDT because the i386 architecture requires memory segmentation. We implement the "Flat Memory Model" to map all segments from address `0x00000000` to `0xFFFFFFFF` (4 GB), which makes segmentation effectively transparent to our C code.

## How it works

### The GDT Structure

The CPU expects the GDT entries to be laid out in a specific 8-byte format. Because the original 80286 processor only had 24-bit addresses and a 16-bit limit, the 80386 designers awkwardly crammed the new 32-bit fields into the remaining available bits to keep backward compatibility. This is why the fields are scattered.

```
                  8-Byte GDT Descriptor Layout
         ┌───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┐
  Byte   │   7   │   6   │   5   │   4   │   3   │   2   │   1   │   0   │
         ├───────┴───────┴───────┴───────┼───────┴───────┴───────┴───────┤
  Content│ Base 31:24    │ Gran. │ Limit │ Access│ Base 23:16            │
         │               │       │ 19:16 │       │                       │
         ├───────────────┼───────┴───────┼───────┴───────────────────────┤
  Content│ Base 15:0                     │ Limit 15:0                    │
         └───────────────────────────────┴───────────────────────────────┘
```

Rather than using magic numbers (like `0x9A` or `0xCF`), we use **C bitfields** to map out exactly what each bit does in the Access and Granularity bytes.

### The Access Byte

The Access byte dictates segment privileges, whether it's code or data, and read/write permissions.

```c
typedef struct gdt_access {
    uint8_t accessed        : 1; // Bit 0: CPU sets this to 1 when accessed
    uint8_t read_write      : 1; // Bit 1: Readable (Code) / Writable (Data)
    uint8_t conforming      : 1; // Bit 2: Can be executed from lower privilege?
    uint8_t executable      : 1; // Bit 3: 1 for Code, 0 for Data
    uint8_t descriptor_type : 1; // Bit 4: 1 for Code/Data, 0 for System
    uint8_t privilege_level : 2; // Bits 5-6: 0 for Kernel (Ring 0), 3 for User (Ring 3)
    uint8_t present         : 1; // Bit 7: Must be 1 for all valid segments
} __attribute__((packed)) gdt_access;
```

**Example: Kernel Code Segment**
- `present` = 1 (Valid segment)
- `privilege_level` = 00 (Ring 0 / Kernel)
- `descriptor_type` = 1 (Code/Data)
- `executable` = 1 (It's a code segment)
- `conforming` = 0 (Can only be executed by Ring 0)
- `read_write` = 1 (We can read constants from the code segment)
- `accessed` = 0 (CPU will flip this)
- *Total Binary: `10011010` -> Hex: `0x9A`*

### The Granularity Byte

The Granularity byte contains the top 4 bits of our 20-bit limit, along with flags defining the segment's size and block scaling.

```c
typedef struct gdt_granularity {
    uint8_t limit_high      : 4; // Bits 0-3: Upper 4 bits of the 20-bit limit
    uint8_t available       : 1; // Bit 4: Unused by hardware (available for OS)
    uint8_t long_mode       : 1; // Bit 5: 1 for 64-bit segments (0 for us)
    uint8_t size_flag       : 1; // Bit 6: 1 for 32-bit protected mode
    uint8_t granularity     : 1; // Bit 7: 1 = multiply limit by 4KB, 0 = in bytes
} __attribute__((packed)) gdt_granularity;
```

**Example: The Flat Model Granularity**
Because our limit is 20 bits (`0xFFFFF`), the maximum addressable memory natively is 1 MB. By setting `granularity = 1`, the CPU shifts our limit by 12 bits (multiplying by 4 KB).
`0xFFFFF * 4096 bytes = 4 GB (0xFFFFFFFF)`.
- `granularity` = 1 (4KB Blocks)
- `size_flag` = 1 (32-bit)
- `long_mode` = 0
- `available` = 0
- `limit_high` = `1111` (The 'F' in 0xFFFFF)
- *Total Binary: `11001111` -> Hex: `0xCF`*

### The GDT Loading Process (lgdt)

To tell the CPU to start using our new table, we must use the `lgdt` (Load GDT) assembly instruction. However, it requires a very specific 6-byte structure:

```c
typedef struct gdt_ptr {
    uint16_t limit; // Size of the GDT minus 1 (Number of bytes)
    uint32_t base;  // Linear address where the GDT is stored
} __attribute__((packed)) gdt_ptr;
```

Once we prepare this `gdt_ptr` (e.g. pointing to `0x800`), we pass it to our assembly routine:
1. `lgdt [ptr]` overrides the CPU's internal GDTR.
2. We reload the Data Segment registers (`DS`, `ES`, `FS`, `GS`, `SS`) with the offset of our Kernel Data selector (`0x10`).
3. We perform a **Far Jump** (`jmp 0x08:flush`) to force the CPU to reload the Code Segment register (`CS`) with our Kernel Code selector (`0x08`) and flush its pipeline.

## Files involved

| File | Role |
|------|------|
| `src/kernel.h` | Definitions of `descriptor`, `gdt_access`, `gdt_granularity`, `gdt_ptr` |
| `src/gdt/gdt.c` | Logic to compile the bitfields and flat mapping into the 0x800 memory |
| `src/gdt/load_gdt.s` | Assembly wrapper to perform the `lgdt` and Far Jump (Coming soon) |
