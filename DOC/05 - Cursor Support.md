# 05 — Cursor Support

## What is it?
The VGA hardware has a built-in **text-mode cursor** — a blinking shape drawn within character cells. We control its **position**, **shape** (full block, underline, etc.), and **visibility** by writing to VGA control registers through I/O ports.

## How it works

### VGA cursor registers

The cursor is controlled through **port 0x3D4** (index register) and **port 0x3D5** (data register). You first tell the VGA *which* register you want to access, then read/write the value:

```
    ┌────────────────────────────────────────────────────────┐
    │            VGA Cursor Control                          │
    ├──────────────┬─────────────────────────────────────────┤
    │  Port 0x3D4  │  Control port — select which register   │
    │  Port 0x3D5  │  Data port — read/write register value  │
    ├──────────────┼─────────────────────────────────────────┤
    │  Registers:  │                                         │
    │    0x0A      │  Cursor Start scanline (+ enable bit)   │
    │    0x0B      │  Cursor End scanline                    │
    │    0x0E      │  Cursor Location High byte              │
    │    0x0F      │  Cursor Location Low byte               │
    └──────────────┴─────────────────────────────────────────┘

    Writing pattern:
    outb(0x3D4, register_index)   ← "I want to access register X"
    outb(0x3D5, value)            ← "Write this value to register X"
```

### Cursor shape (scanlines)

Each character cell is **16 pixels tall** (scanlines 0–15). The cursor is drawn between `cursor_start` and `cursor_end` scanlines:

```
    Character cell (16 scanlines)

    Scanline  Full Block    Underline     Top Line      Middle
     0        ████████      ........      ████████      ........
     1        ████████      ........      ████████      ........
     2        ████████      ........      ........      ........
     3        ████████      ........      ........      ........
     4        ████████      ........      ........      ........
     5        ████████      ........      ........      ........
     6        ████████      ........      ........      ........
     7        ████████      ........      ........      ████████
     8        ████████      ........      ........      ████████
     9        ████████      ........      ........      ........
    10        ████████      ........      ........      ........
    11        ████████      ........      ........      ........
    12        ████████      ........      ........      ........
    13        ████████      ████████      ........      ........
    14        ████████      ████████      ........      ........
    15        ████████      ████████      ........      ........
              start=0       start=13      start=0       start=7
              end=15        end=15        end=1         end=8
```

We use `start=0, end=15` for a **full block cursor** (█).

### Cursor position

The cursor position is a **16-bit linear offset** (same indexing as VGA buffer). Since VGA registers are 8-bit, we split it into high and low bytes:

```
    position = row × 80 + column

    Example: cursor at row 3, column 10
    position = 3 × 80 + 10 = 250 = 0x00FA

    ┌───────────────────────────────┐
    │  position = 0x00FA            │
    │                               │
    │  High byte: 0x00  ──► reg 0x0E│
    │  Low byte:  0xFA  ──► reg 0x0F│
    └───────────────────────────────┘

    Code:
    outb(0x3D4, 0x0F)          ← select low byte register
    outb(0x3D5, pos & 0xFF)    ← write 0xFA

    outb(0x3D4, 0x0E)          ← select high byte register
    outb(0x3D5, pos >> 8)      ← write 0x00
```

### Hiding the cursor

Bit 5 of the Cursor Start register controls visibility:

```
    Register 0x0A (Cursor Start):
    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    │ 7 │ 6 │ 5 │ 4 │ 3 │ 2 │ 1 │ 0 │
    └───┴───┴───┴───┴───┴───┴───┴───┘
                  │
                  └── Bit 5: 1 = cursor DISABLED
                             0 = cursor ENABLED

    hide_cursor():
        outb(0x3D4, 0x0A)
        outb(0x3D5, 0x20)    ← 0x20 = 0b00100000 → bit 5 set

    show_cursor():
        outb(0x3D4, 0x0A)
        outb(0x3D5, 0x00)    ← bit 5 clear, start scanline = 0
        outb(0x3D4, 0x0B)
        outb(0x3D5, 0x0F)    ← end scanline = 15
```

### Cursor visibility state

We also track a software flag `cursor_visible` in the `screen` struct. This is used by the keyboard handler to **block input** when the cursor is hidden (e.g., during scroll-back history browsing):

```
    handle_keyboard_keypress() loop:

    ┌─────────────────────────────────────────┐
    │  scancode received                       │
    │                                          │
    │  if cursor_visible == true:              │
    │      process_scancode()  ← typing works  │
    │      update_cursor_pos()                  │
    │  else:                                   │
    │      (ignore keystroke)  ← input blocked  │
    └─────────────────────────────────────────┘
```

## How we implemented it

### `initialize_cursor_pos()` — Called once per screen at boot

1. Configure cursor shape (scanlines 0–15 = full block)
2. Set initial position to (0, 0)

### `update_cursor_pos()` — Called after every character write

1. Compute `cursor_pos = row × 80 + column`
2. Write high and low bytes to VGA registers

### `hide_cursor()` — Called when entering scroll-back mode

Set bit 5 of register 0x0A to disable hardware cursor.

### `show_cursor()` — Called when returning to live view

Clear bit 5, restore scanline range (0–15), and update position.

## Files involved

| File | Role |
|------|------|
| `src/drivers/handle_cursor.c` | `initialize_cursor_pos()`, `update_cursor_pos()`, `hide_cursor()`, `show_cursor()` |
| `src/drivers/port_io.c` | `outb()` for writing to VGA ports |
| `src/kernel.h` | VGA port defines, cursor register defines, `cursor_visible` flag |
