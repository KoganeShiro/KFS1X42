# 03 — Keyboard Input

## What is it?
The keyboard is a PS/2 device that sends **scancodes** (hardware-specific numbers) when keys are pressed or released. We read these scancodes from I/O ports, translate them to ASCII characters using a keymap table, and handle special keys (Shift, Backspace, Enter, arrows, etc.).

## How it works

### Keyboard I/O ports

```
    ┌──────────────────────────────────────────────┐
    │              PS/2 Keyboard Controller         │
    ├──────────────┬───────────────────────────────┤
    │  Port 0x64   │  Status register (read)       │
    │              │  Bit 0 = 1: data ready        │
    │              │  Bit 0 = 0: buffer empty       │
    ├──────────────┼───────────────────────────────┤
    │  Port 0x60   │  Data register (read)         │
    │              │  Contains the scancode         │
    └──────────────┴───────────────────────────────┘

    Reading flow:
    1. Check port 0x64 bit 0   ──► Is there data?
    2. If yes, read port 0x60  ──► Get scancode
    3. If no, loop back to 1
```

### Scancodes (Set 1)

When you **press** a key, you get its **make code**. When you **release** it, you get its **break code** (make code + 0x80):

```
    Key press:     scancode = 0x1E        (make code for 'A')
    Key release:   scancode = 0x1E | 0x80 = 0x9E  (break code)

    ┌──────────┬──────────┬──────────┐
    │ Scancode │  Key     │ ASCII    │
    ├──────────┼──────────┼──────────┤
    │   0x02   │  '1'     │  '1'     │
    │   0x10   │  'q'     │  'q'     │
    │   0x1E   │  'a'     │  'a'     │
    │   0x2C   │  'z'     │  'z'     │
    │   0x39   │  Space   │  ' '     │
    │   0x1C   │  Enter   │  '\n'    │
    │   0x0E   │  Bksp    │  '\b'    │
    │   0x2A   │  LShift  │  (mod)   │
    └──────────┴──────────┴──────────┘
```

### Keymap translation

We use two static lookup tables — one for normal keys, one for shifted keys:

```
    scancode ──► scancode_to_ascii[128]          ──► 'a'
    scancode ──► scancode_to_ascii_shifted[128]  ──► 'A'

    Example: scancode 0x1E with shift pressed
    ┌──────────────────┐
    │ shift_pressed?   │
    │   YES ──► shifted table ──► 'A'
    │   NO  ──► normal table  ──► 'a'
    └──────────────────┘
```

### Main keyboard loop

```
    handle_keyboard_keypress(kfs *k)
    ┌───────────────────────────────────────────────────────┐
    │  while (1) {                                          │
    │      scancode = get_scancode()                        │
    │      if scancode == 0 → continue (no data)            │
    │      │                                                │
    │      ├── handle_screen_shortcut() → consumed? skip    │
    │      ├── handle_scroll_key()      → consumed? skip    │
    │      │                                                │
    │      └── if cursor_visible:                           │
    │              process_scancode(scr, scancode)           │
    │              update_cursor_pos(scr)                    │
    │  }                                                    │
    └───────────────────────────────────────────────────────┘
```

The infinite loop reads scancodes and dispatches them through a priority chain:
1. **Screen shortcuts** (Alt+1..9) — handled first
2. **Scroll keys** (Page Up/Down) — handled second
3. **Normal input** — only if cursor is visible (not in scroll-back mode)

### Shift / CapsLock handling

```
    ┌─────────────────────────────────────────────┐
    │          Shift state machine                 │
    │                                              │
    │  LShift pressed  (0x2A)    → shift = true    │
    │  LShift released (0xAA)    → shift = false   │
    │  RShift pressed  (0x36)    → shift = true    │
    │  RShift released (0xB6)    → shift = false   │
    │  CapsLock pressed (0x3A)   → toggle shift    │
    └─────────────────────────────────────────────┘
```

### Backspace handling

Backspace moves the cursor left by one column, then shifts all characters after it to the left to fill the gap:

```
    Before backspace (cursor at col 3, marked ◄):
    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    │ H │ e │ l │ ◄ │ l │ o │ _ │ _ │
    └───┴───┴───┴───┴───┴───┴───┴───┘
                  ▲ cursor

    Step 1: Move cursor left (col 2)
    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    │ H │ e │ ◄ │ l │ l │ o │ _ │ _ │
    └───┴───┴───┴───┴───┴───┴───┴───┘
              ▲ cursor

    Step 2: shift_left_from_cursor — memmove everything left
    ┌───┬───┬───┬───┬───┬───┬───┬───┐
    │ H │ e │ l │ l │ o │ _ │ _ │ _ │  ← last cell cleared
    └───┴───┴───┴───┴───┴───┴───┴───┘
              ▲ cursor stays here
```

### Arrow keys

```
    ┌───────────┬─────────────────────────────────────────┐
    │   Key     │   Behavior                              │
    ├───────────┼─────────────────────────────────────────┤
    │   ←       │   Move left IF col > 0 AND char exists  │
    │   →       │   Move right IF col < 79 AND char exists│
    │   ↑ / ↓   │   Reserved (command history - not impl) │
    └───────────┴─────────────────────────────────────────┘

    The cursor can only move through existing characters,
    it cannot move into empty space:

    ┌───┬───┬───┬───┬───┬───┬───┐
    │ H │ i │ ! │ _ │ _ │ _ │ _ │
    └───┴───┴───┴───┴───┴───┴───┘
              ▲
    Pressing → here: cursor moves to col 3
    BUT col 3 is '\0', so has_char_at() returns false
    → cursor stays at col 2
```

## Files involved

| File | Role |
|------|------|
| `src/drivers/handle_keyboard.c` | Main loop, scancode processing, backspace, arrows |
| `src/drivers/keymap.c` | Scancode-to-ASCII translation tables |
| `src/drivers/port_io.c` | `inb()` to read from keyboard ports |
| `src/kernel.h` | Key scancode defines (`KEY_ENTER`, `KEY_BACKSPACE`, etc.) |
