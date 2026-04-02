# CPU Registers, Memory Segmentation & QEMU Output

This document serves as a reference for how memory segmentation works in 32-bit x86 Protected Mode, what the CPU registers actually do, and how to interpret the raw output from QEMU's `info registers` command.

---

## 1. The GDT Descriptor (Memory Segmentation)

In a flat-memory model (which is what standard modern kernels use), we still must define segments via the Global Descriptor Table (GDT). Each entry is 64 bits (8 bytes) and tells the CPU where a segment starts (Base), how big it is (Limit), and who is allowed to access it (Access/Flags).

### GDT Entry Memory Layout
```text
63          56 55  52 51   48 47           40 39        32 31               16 15                 0
+-------------+------+-------+---------------+------------+-------------------+-------------------+
| Base 24:31  | Gran | Limit |  Access Byte  | Base 16:23 |     Base 0:15     |    Limit 0:15     |
+-------------+------+-------+---------------+------------+-------------------+-------------------+
```

### The Access Byte (Bits 40-47)
| Bit | Name | Description |
| :--- | :--- | :--- |
| **7 (47)** | **P** (Present) | Must be `1` for any valid segment. |
| **5-6 (45-46)** | **DPL** (Privilege) | Descriptor Privilege Level. `00` (0) for Kernel, `11` (3) for User space. |
| **4 (44)** | **S** (Type) | `1` for Code/Data segments, `0` for System segments (like TSS). |
| **3 (43)** | **E** (Executable) | `1` if this is a Code Segment (can be executed). `0` if Data Segment. |
| **2 (42)** | **DC** (Direction/Conf)| For Data: Direction (0=grows up, 1=grows down). For Code: Conforming (can lower privilege execute it?). Usually `0`. |
| **1 (41)** | **RW** (Read/Write) | For Code: `1` allows Reading. For Data: `1` allows Writing. |
| **0 (40)** | **A** (Accessed) | CPU sets to `1` when the segment is accessed. Usually starts at `0`. |

### The Granularity Byte (Bits 52-55)
| Bit | Name | Description |
| :--- | :--- | :--- |
| **7 (55)** | **G** (Granularity) | `0` = Limit is in 1 Byte blocks, `1` = Limit is in 4 KiB (4096 byte) blocks. |
| **6 (54)** | **DB** (Size Flag) | `0` = 16-bit protected mode, `1` = 32-bit protected mode. |
| **5 (53)** | **L** (Long Mode) | `1` = 64-bit code segment. `0` = 32-bit code. |
| **4 (52)** | **AVL** (Available) | For system software use. Usually `0`. |

*(Bits 48-51 contain the highest 4 bits of the 20-bit Limit).*

---

## 2. 32-bit x86 CPU Registers

### General Purpose Registers
Used for mathematical operations, holding variables, and passing arguments.
* **EAX** (Accumulator): Used for arithmetic operations and function return values.
* **EBX** (Base): General data holding.
* **ECX** (Counter): Used as a loop counter.
* **EDX** (Data): Used in I/O operations (like our `outb`/`inb` functions).

### Index & Pointer Registers
Used primarily for memory addressing.
* **ESI** (Source Index): Used for string/memory copying.
* **EDI** (Destination Index): Used for string/memory copying.
* **EBP** (Base Pointer): Points to the base of the current stack frame. Used to access local variables.
* **ESP** (Stack Pointer): Points to the very top (lowest memory address) of the current stack.
* **EIP** (Instruction Pointer): Points to the memory address of the next CPU instruction to execute.

### Segment Registers
Hold "Selectors" (offsets) into the GDT, telling the CPU which memory segment rules apply.
* **CS** (Code Segment): Points to the GDT entry containing executable code.
* **DS** (Data Segment): Points to the GDT entry containing readable/writable data.
* **SS** (Stack Segment): Points to the GDT entry used for the Stack.
* **ES, FS, GS** (Extra Segments): Additional data segments.

---

## 3. Demystifying QEMU's `info registers`

When you type `info registers` in the QEMU monitor, you get output describing the absolute current state of the CPU.

### Example Segment Breakdown
```text
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
DS =0010 00000000 ffffffff 00cf9200 DPL=0 DS   [-W-]
```

**Let's decode the `CS` (Code Segment) line:**

1. **`0008` (Selector):**
   * This is the value loaded into the `CS` register (8 in decimal).
   * Since each GDT entry is 8 bytes, `8 / 8 = Entry 1`. This proves the CPU is using GDT Entry 1.
2. **`00000000` (Base):**
   * The starting physical memory address mapping for this segment (Flat memory model).
3. **`ffffffff` (Limit):**
   * The maximum size of the segment. Since Granularity (`G`) is `1` (4KiB blocks), a 20-bit limit of `0xFFFFF` becomes `0xFFFFFFFF` (4 GB) physically.
4. **`00cf9a00` (Raw Descriptor Hex):**
   * This is QEMU reconstructing the raw bytes of the descriptor flags for you. 
   * `9a`: Access byte (`10011010` in binary) -> Present, Ring 0, Code, Executable, Readable.
   * `c_`: Granularity byte (`1100` in binary) -> 4KiB Granularity, 32-bit Size.
5. **`DPL=0`:**
   * Descriptor Privilege Level is 0 (Kernel mode).
6. **`CS32`:**
   * Confirms it is recognized as a 32-bit Code Segment.
7. **`[-R-]`:**
   * Quick-read flags: Read is allowed.

**Decoding the `DS` (Data Segment) line:**
* **`0010` (Selector):** 16 in decimal. `16 / 8 = Entry 2`. CPU is using GDT Entry 2.
* **`00cf9200` (Raw Hex):**
  * `92`: Access byte (`10010010` in binary) -> Present, Ring 0, Data, Writable.
* **`[-W-]`:** Ensures memory region is Writable.

### Example General Register Breakdown
```text
EAX=0000001c EBX=0000009d ECX=000003d5 EDX=00000064
ESI=0000009d EDI=00209000 EBP=00208f98 ESP=00208f98
EIP=0010023a EFL=00000202 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0
```
* **`EBP=00208f98 ESP=00208f98`**: In this specific snapshot, the Stack Pointer (ESP) and Base Pointer (EBP) are identical, meaning the CPU is currently sitting inside a fresh, empty function frame.
* **`EIP=0010023a`**: The exact memory address of the instruction the CPU is currently running. If you disassemble your kernel, you can trace this exact address back to a specific line in your C/Assembly code.
* **`CPL=0`**: Current Privilege Level is 0 (Kernel).
