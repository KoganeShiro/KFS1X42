# 06 — Screen Switching

## What is it?
We support **9 virtual screens** (Alt+1 through Alt+9) that each have their own independent VGA content, cursor position, color state, and scroll history. Only one screen is "live" (displayed on the monitor) at a time — the others are stored in RAM.

## How it works

### Memory architecture

There is only **one VGA hardware buffer** at `0xB8000` (4000 bytes). To simulate multiple screens, each screen has its own **private RAM buffer** (`saved_buffer`). A pointer swap determines which screen is "live":

```
    Hardware (one physical display)
    ┌──────────────────────────────────┐
    │  VGA_MEMORY (0xB8000)            │
    │  4000 bytes — what's on monitor  │
    └──────────┬───────────────────────┘
               │
               │  active screen's buffer pointer
               │  points here
               │
    ┌──────────┴───────────────────────────────────────────┐
    │                     RAM                               │
    │                                                       │
    │  saved_buffers[0]  ←── screen 0's private copy        │
    │  [4000 bytes]          (backed up when switching away) │
    │                                                       │
    │  saved_buffers[1]  ←── screen 1's private copy        │
    │  [4000 bytes]                                         │
    │                                                       │
    │  saved_buffers[2]  ←── screen 2's private copy        │
    │  [4000 bytes]                                         │
    │                                                       │
    │  ...                                                  │
    │                                                       │
    │  saved_buffers[8]  ←── screen 8's private copy        │
    │  [4000 bytes]                                         │
    └───────────────────────────────────────────────────────┘

    Total RAM for screens: 9 × 4000 = 36,000 bytes
```

### The two-pointer design

Each `screen` struct has **two** buffer pointers:

```
    ┌────────────────────────────────────────────────────────┐
    │  screen struct                                         │
    │                                                        │
    │  saved_buffer ──► always points to saved_buffers[id]   │
    │                   (set once at init, NEVER changes)    │
    │                                                        │
    │  buffer ──► points to EITHER:                          │
    │             • VGA_MEMORY (0xB8000) if ACTIVE screen    │
    │             • saved_buffer         if INACTIVE screen  │
    └────────────────────────────────────────────────────────┘
```

This means `kfs_putchar()`, `printk()`, etc. always write to `scr->buffer` — they don't need to know if the screen is visible or not. The pointer swap makes it transparent.

### Switching screens step by step

```
    switch_screen(k, new_index)

    BEFORE: Screen 0 is active (on monitor), switching to Screen 2

    ┌─────────────────────────────────────────────────────┐
    │  VGA_MEMORY:  "Hello from screen 0"                  │
    │                                                      │
    │  Screen 0:                                           │
    │    buffer ────────► VGA_MEMORY (active)               │
    │    saved_buffer ──► saved_buffers[0] (stale)         │
    │                                                      │
    │  Screen 2:                                           │
    │    buffer ────────► saved_buffers[2] (inactive)       │
    │    saved_buffer ──► saved_buffers[2] ("Welcome...")   │
    └─────────────────────────────────────────────────────┘

    ═══════════════════════════════════════════════════════

    Step 1: SAVE — memcpy(VGA → saved_buffers[0])
    "Hello from screen 0" is backed up to RAM

    Step 2: DEACTIVATE — screen[0].buffer = saved_buffer
    Screen 0 now writes to RAM, not the monitor

    Step 3: RESTORE — memcpy(saved_buffers[2] → VGA)
    "Welcome..." appears on the monitor

    Step 4: ACTIVATE — screen[2].buffer = VGA_MEMORY
    Screen 2 now writes directly to the monitor

    Step 5: UPDATE CURSOR — update_cursor_pos(new_scr)
    Hardware cursor jumps to screen 2's saved position

    ═══════════════════════════════════════════════════════

    AFTER:

    ┌─────────────────────────────────────────────────────┐
    │  VGA_MEMORY:  "Welcome..."  (screen 2 content)      │
    │                                                      │
    │  Screen 0:                                           │
    │    buffer ────────► saved_buffers[0] (inactive)      │
    │    saved_buffer ──► saved_buffers[0] ("Hello...")     │
    │                                                      │
    │  Screen 2:                                           │
    │    buffer ────────► VGA_MEMORY (active)               │
    │    saved_buffer ──► saved_buffers[2]                  │
    └─────────────────────────────────────────────────────┘
```

### Alt key detection

We track the Alt key state using make/break scancodes:

```
    ┌───────────────────────────────────────────┐
    │  Scancode 0x38      → Alt PRESSED         │
    │  Scancode 0x38+0x80 → Alt RELEASED        │
    │                                            │
    │  While alt_pressed == true:                │
    │    Scancode 0x02 ('1') → switch to screen 0│
    │    Scancode 0x03 ('2') → switch to screen 1│
    │    ...                                     │
    │    Scancode 0x0A ('9') → switch to screen 8│
    │                                            │
    │  Formula: screen_index = scancode - 0x02   │
    └───────────────────────────────────────────┘
```

### Per-screen state (all independent)

```
    ┌──────────┬──────────┬──────────┬──────────┬─────────┐
    │          │ Screen 0 │ Screen 1 │ Screen 2 │  ...    │
    ├──────────┼──────────┼──────────┼──────────┼─────────┤
    │ row      │    5     │    0     │    12    │         │
    │ column   │   23     │    0     │     7   │         │
    │ color    │  0x0F    │  0x0F   │   0x0A   │         │
    │ cursor   │  visible │ visible │ visible  │         │
    │ history  │ pool[0]  │ pool[1] │ pool[2]  │         │
    │ buffer   │ VGA_MEM  │ saved[1]│ saved[2] │         │
    └──────────┴──────────┴──────────┴──────────┴─────────┘
                  ▲ active    inactive   inactive
```

### Static allocation (no malloc)

All buffers are allocated as **static global arrays** in `kernel.c`:

```c
static uint16_t   saved_buffers[MAX_SCREENS][VGA_BUFFER_SIZE]; // 9 × 4000 cells
static ring_buffer history_pool[MAX_SCREENS];                  // 9 history buffers
```

This works because kernel `.bss` is zeroed by `boot.s` at startup — no dynamic allocation needed.

## Files involved

| File | Role |
|------|------|
| `src/tty/handle_screen.c` | `switch_screen()` — save/restore/swap buffers |
| `src/tty/handle_screen_shortcut.c` | Alt+N shortcut detection |
| `src/kernel.c` | `init_screen()`, `kfs_initialize()`, static buffer arrays |
| `src/kernel.h` | `screen` struct, `kfs` struct, `MAX_SCREENS` |
