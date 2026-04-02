# 02 — VGA Text Mode Display

## What is it?
VGA text mode is a hardware standard that allows writing characters and colors directly to a memory region (`0xB8000`), and the video hardware automatically displays them on screen. It's essentially a **frame buffer** for text: write a byte pair (character + color) at the right address, and it appears on the monitor.

## How it works

### VGA memory layout

The screen is a grid of **80 columns × 25 rows = 2000 cells**. Each cell is **2 bytes (16 bits)**:

```
    One VGA cell (uint16_t)
    ┌─────────────────────────────────┐
    │  Byte 1 (high)  │  Byte 0 (low) │
    │    Color attr    │   ASCII char   │
    │   ┌───┬───┐     │                │
    │   │BG │FG │     │    e.g. 'A'    │
    │   │4b │4b │     │    = 0x41      │
    │   └───┴───┘     │                │
    └─────────────────────────────────┘

    Example: White 'A' on black background
    FG = 0xF (white), BG = 0x0 (black)
    Color byte = 0x0F
    Cell value  = 0x0F41
```

### Full VGA buffer map

```
    VGA_MEMORY = 0xB8000

    Address         Row  Col   What's on screen
    ┌──────────┐
    │ 0xB8000  │    0    0    Top-left character
    │ 0xB8002  │    0    1    Second character
    │   ...    │    
    │ 0xB809E  │    0   79    End of row 0
    ├──────────┤
    │ 0xB80A0  │    1    0    Start of row 1
    │   ...    │
    ├──────────┤
    │   ...    │   ...  ...
    ├──────────┤
    │ 0xB8F00  │   24    0    Start of last row
    │   ...    │
    │ 0xB8F9E  │   24   79    Bottom-right character
    └──────────┘

    Total: 80 × 25 × 2 bytes = 4000 bytes
```

### Computing a cell's address

```
    index = row × VGA_WIDTH + column

    Example: character at row 3, column 10
    index = 3 × 80 + 10 = 250
    address = 0xB8000 + (250 × 2) = 0xB81F4

    ┌────────────────────────────────────────────────── col 79
    │  row 0:  [ 0 ][ 1 ][ 2 ] ... [ 79 ]
    │  row 1:  [ 80][ 81][ 82] ... [159 ]
    │  row 2:  [160][161][162] ... [239 ]
    │  row 3:  [240][241]...[250]◄─── HERE  ... [319 ]
    │    ...
    │  row 24: [1920][1921] ... [1999]
    └───────────────────────────────────────────────────
```

### Color palette (16 colors)

```
    VGA 4-bit color values:

    ┌─────┬─────────────────┐  ┌─────┬─────────────────┐
    │  0  │ Black           │  │  8  │ Dark Grey        │
    │  1  │ Blue            │  │  9  │ Light Blue       │
    │  2  │ Green           │  │ 10  │ Light Green      │
    │  3  │ Cyan            │  │ 11  │ Light Cyan       │
    │  4  │ Red             │  │ 12  │ Light Red        │
    │  5  │ Magenta         │  │ 13  │ Light Magenta    │
    │  6  │ Brown           │  │ 14  │ Light Brown      │
    │  7  │ Light Grey      │  │ 15  │ White            │
    └─────┴─────────────────┘  └─────┴─────────────────┘

    Color byte = (BG << 4) | FG

    Example: Light Green text on Blue background
    BG = 1 (Blue), FG = 10 (Light Green)
    Color byte = (1 << 4) | 10 = 0x1A
```

## How we implemented it

### `vga_entry()` — Pack char + color into a VGA cell

```c
uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}
```

```
    vga_entry('H', 0x0F)

       color = 0x0F          char = 'H' (0x48)
       ┌────────┐            ┌────────┐
       │00001111│            │01001000│
       └────────┘            └────────┘
            │ << 8                │
            ▼                     │
    ┌────────┬────────┐          │
    │00001111│00000000│          │
    └────────┴────────┘          │
                    │ OR ────────┘
                    ▼
            ┌────────┬────────┐
            │00001111│01001000│  = 0x0F48
            └────────┴────────┘
             color     'H'
```

### `kfs_putentry()` — Write one character at cursor position

Computes the linear index from `(row, column)` and writes the packed VGA entry.

### `kfs_putchar()` — Write character and advance cursor

Handles:
- **Newline (`\n`)**: reset column to 0, increment row. If row overflows (≥ 25), trigger scroll.
- **Regular char**: write via `kfs_putentry`, advance column. If column wraps (≥ 80), move to next line.

```
    Before kfs_putchar(scr, 'X'):        After:
    
    row=2, col=5                          row=2, col=6
    ┌──────────────────────┐              ┌──────────────────────┐
    │H e l l o _ _ _ ...   │ row 0        │H e l l o _ _ _ ...   │ row 0
    │w o r l d _ _ _ ...   │ row 1        │w o r l d _ _ _ ...   │ row 1
    │t e s t _ ◄ _ _ ...   │ row 2        │t e s t X ◄ _ _ ...   │ row 2
    │_ _ _ _ _ _ _ _ ...   │ row 3        │_ _ _ _ _ _ _ _ ...   │ row 3
    └──────────────────────┘              └──────────────────────┘
              ▲ cursor                              ▲ cursor moved right
```

## I/O ports (`port_io.c`)

Hardware communication happens through **I/O ports** using x86 `in`/`out` instructions:

```
    CPU ◄──── inb(port)  ────► Hardware register
    CPU ────► outb(port, val) ► Hardware register

    inb(port):   Read 1 byte FROM a hardware port INTO CPU
    outb(port, val): Write 1 byte FROM CPU TO a hardware port
```

These are implemented with inline assembly (`asm volatile`), letting C code talk directly to hardware (VGA registers, keyboard controller, etc.).

## Files involved

| File | Role |
|------|------|
| `src/drivers/vga.c` | `vga_entry()`, `kfs_set_color()` — VGA cell packing, color management |
| `src/drivers/vga_write.c` | `kfs_putentry()`, `kfs_putchar()` — write chars to VGA buffer |
| `src/drivers/port_io.c` | `inb()`, `outb()`, `outw()` — raw I/O port access |
| `src/kernel.h` | VGA constants, color enum, screen struct |
