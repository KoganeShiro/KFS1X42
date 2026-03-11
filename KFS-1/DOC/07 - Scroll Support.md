# 07 — Scroll Support

## What is it?
Scroll support handles two distinct scenarios:
1. **Terminal overflow** — when typing pushes content past the bottom of the screen, rows shift up automatically
2. **History browsing** — Page Up/Down lets you scroll back through lines that have scrolled off-screen, using a **ring buffer** to store history

## How it works

### Part 1: Terminal Overflow (`terminal_overflow_down`)

When `kfs_putchar` increments `row` past 24 (VGA_HEIGHT - 1), `terminal_overflow_down()` is triggered:

```
    BEFORE: cursor at row 25 (off-screen trigger)

    VGA Buffer:
    ┌──────────────────────────────────┐
    │ Row  0: "Line A - oldest"        │ ◄── about to be lost
    │ Row  1: "Line B"                 │
    │ Row  2: "Line C"                 │
    │   ...                            │
    │ Row 23: "Line X"                 │
    │ Row 24: "Line Y - newest"        │
    └──────────────────────────────────┘
    Row 25: cursor is here (overflow!)


    ═══════════════════════════════════════

    Step 1: SAVE row 0 to history ring buffer
            (before it gets overwritten)

    ┌──────────────────────────────────┐
    │ History ring buffer:             │
    │   [...] → "Line A - oldest"      │ ← saved here
    └──────────────────────────────────┘

    Step 2: memmove — shift rows 1..24 UP by one row

    ┌──────────────────────────────────┐
    │ Row  0: "Line B"                 │ ← was row 1
    │ Row  1: "Line C"                 │ ← was row 2
    │   ...                            │
    │ Row 22: "Line X"                 │ ← was row 23
    │ Row 23: "Line Y - newest"        │ ← was row 24
    │ Row 24: ?????                    │ ← garbage
    └──────────────────────────────────┘

    Step 3: Clear row 24 (fill with '\0' + color)

    ┌──────────────────────────────────┐
    │ Row  0: "Line B"                 │
    │ Row  1: "Line C"                 │
    │   ...                            │
    │ Row 23: "Line Y - newest"        │
    │ Row 24: "                    "   │ ← clean empty row
    └──────────────────────────────────┘

    Step 4: Pin cursor to row 24, reset scroll_offset to 0

    AFTER: cursor stays at row 24, ready for new input
```

### Part 2: History Ring Buffer

Each screen has a ring buffer that stores up to HISTORY_MAX_LINES of history. It's a circular array — when it fills up, the oldest lines get overwritten.

```
    ring_buffer struct:
    ┌─────────────────────────────────────────────────────────────┐
    │  buffer[VGA_WIDTH * HISTORY_MAX_LINES]  ← stores x lines of VGA cells     │
    │  live_snapshot[VGA_BUFFER_SIZE]   ← snapshot of VGA when scrolling     │
    │  current_line          ← write head (next slot to write to) │
    │  total_lines           ← how many lines stored (max 750)    │
    │  scroll_offset         ← 0 = live, >0 = scrolled back N    │
    └─────────────────────────────────────────────────────────────┘
```

#### Ring buffer write (`save_line_to_history`)

```
    Saving a line (ring wraps around):

    if HISTORY_MAX_LINES = 750

    Before (current_line = 3, total_lines = 3):
    ┌───────┬──────────────────────────────┐
    │ Idx 0 │ "First line ever"            │
    │ Idx 1 │ "Second line"                │
    │ Idx 2 │ "Third line"                 │
    │ Idx 3 │ (empty — next write goes here)│ ◄── current_line
    │ Idx 4 │ (empty)                      │
    │  ...  │                              │
    │Idx 749│ (empty)                      │
    └───────┴──────────────────────────────┘

    After save_line_to_history("Fourth line"):
    ┌───────┬──────────────────────────────┐
    │ Idx 0 │ "First line ever"            │
    │ Idx 1 │ "Second line"                │
    │ Idx 2 │ "Third line"                 │
    │ Idx 3 │ "Fourth line"                │ ← just written
    │ Idx 4 │ (empty — next write goes here)│ ◄── current_line = 4
    │  ...  │                              │
    └───────┴──────────────────────────────┘
    total_lines = 4

    ──────────────────────────────────────────

    When buffer is FULL and wraps (current_line = 749):

    After save_line_to_history("New line"):
    ┌───────┬──────────────────────────────┐
    │ Idx 0 │ (next write goes here)       │ ◄── current_line = 0
    │ Idx 1 │ "Some old line"              │     (wraps around!)
    │  ...  │                              │
    │Idx 749│ "New line"                   │ ← just written
    └───────┴──────────────────────────────┘
    total_lines = 750 (capped, doesn't grow further)
```

### Part 3: Page Up (scroll back into history)

```
    Page Up pressed (scroll_offset goes from 0 → 1 → 2 ...):

    ┌──────────────────────────────────────────────────────────┐
    │  FIRST Page Up (scroll_offset was 0):                    │
    │                                                          │
    │  1. Snapshot current VGA → live_snapshot[]                │
    │     (preserve what user was looking at)                   │
    │                                                          │
    │  2. scroll_offset = 1                                    │
    │                                                          │
    │  3. scroll_redraw():                                     │
    │     VGA row 0  ← history[current_line - 1]  (1 old line)│
    │     VGA row 1  ← live_snapshot row 0                     │
    │     VGA row 2  ← live_snapshot row 1                     │
    │     ...                                                  │
    │     VGA row 24 ← live_snapshot row 23                    │
    │                                                          │
    │  4. Hide cursor + set cursor_visible = false              │
    │     (keyboard input is blocked)                          │
    └──────────────────────────────────────────────────────────┘
```

#### Visual example of `scroll_redraw` with `scroll_offset = 3`:

```
    History ring buffer (current_line = 50):
    ┌───────┬────────────────────────┐
    │ Idx 47│ "old line A"           │ ← current_line - 3
    │ Idx 48│ "old line B"           │ ← current_line - 2
    │ Idx 49│ "old line C"           │ ← current_line - 1
    │ Idx 50│ (write head)           │ ← current_line
    └───────┴────────────────────────┘

    live_snapshot (25 rows saved when entering scroll mode):
    ┌────────────────────────────────┐
    │ snap row  0: "live line 0"     │
    │ snap row  1: "live line 1"     │
    │ ...                            │
    │ snap row 21: "live line 21"    │
    │ snap row 22: "typed text here" │
    │ snap row 23: "cursor was here" │
    │ snap row 24: "                "│
    └────────────────────────────────┘

    scroll_redraw produces (offset = 3):
    ┌────────────────────────────────────────────────────────┐
    │ VGA Row  0 ← history[47]  "old line A"                │ ─┐
    │ VGA Row  1 ← history[48]  "old line B"                │  │ from history
    │ VGA Row  2 ← history[49]  "old line C"                │ ─┘
    │ VGA Row  3 ← snap row 0   "live line 0"               │ ─┐
    │ VGA Row  4 ← snap row 1   "live line 1"               │  │
    │   ...                                                  │  │ from snapshot
    │ VGA Row 24 ← snap row 21  "live line 21"              │  │
    │            (snap rows 22-24 are pushed off-screen)     │ ─┘
    └────────────────────────────────────────────────────────┘

    Note: as scroll_offset increases, more history rows appear
    at the top, and live rows are pushed off the bottom.
```

### Part 4: Page Down (scroll back toward live view)

```
    Page Down pressed (scroll_offset decreases):

    ┌──────────────────────────────────────────────────────────┐
    │  If scroll_offset > 1:                                    │
    │     scroll_offset--                                       │
    │     scroll_redraw() with new offset                       │
    │     cursor stays hidden                                   │
    │                                                          │
    │  If scroll_offset reaches 0 (back to live view):         │
    │     1. memcpy(live_snapshot → VGA)                         │
    │        (restore the original screen content)              │
    │     2. cursor_visible = true                              │
    │     3. show_cursor() — re-enable hardware cursor          │
    │     → User can type again!                                │
    └──────────────────────────────────────────────────────────┘
```

### Complete state diagram

```
    ┌─────────────────┐
    │   LIVE VIEW     │ ◄──────────────────────────────┐
    │  scroll_offset=0│                                │
    │  cursor visible │     Page Down                   │
    │  input enabled  │     (scroll_offset reaches 0)   │
    └────────┬────────┘                                │
             │ Page Up                                  │
             │ (save live_snapshot)                     │
             ▼                                         │
    ┌─────────────────┐                                │
    │  SCROLL MODE    │                                │
    │  scroll_offset>0│                                │
    │  cursor hidden  │ ──── Page Down ───────────────┘
    │  input blocked  │     (scroll_offset--)
    │                 │
    │                 │ ──── Page Up (scroll_offset++)
    │                 │     (capped at total_lines)
    └─────────────────┘

    New output while scrolled back:
    terminal_overflow_down() resets scroll_offset to 0
    → snaps back to live view automatically
```

## Files involved

| File | Role |
|------|------|
| `src/tty/handle_scroll.c` | Ring buffer save, scroll_redraw, Page Up/Down, terminal_overflow_down |
| `src/drivers/handle_cursor.c` | `hide_cursor()`, `show_cursor()` — VGA hardware cursor control |
| `src/drivers/handle_keyboard.c` | Checks `cursor_visible` to block input during scroll mode |
| `src/kernel.h` | `ring_buffer` struct, `HISTORY_MAX_LINES`, `KEY_PG_UP`, `KEY_PG_DOWN` |
| `src/kernel.c` | `history_pool[MAX_SCREENS]` — static allocation of per-screen ring buffers |
