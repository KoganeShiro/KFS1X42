#include "kernel.h"

/*
** save_line_to_history() - Save one VGA row into the ring buffer
**   Copies VGA_WIDTH uint16_t entries from scr->buffer[row] → history.
**   Advances current_line (wrapping), increments total_lines (capped).
*/
static void save_line_to_history(screen *scr, size_t row)
{
	ring_buffer *h = scr->history;
	uint16_t *dst = &h->buffer[h->current_line * VGA_WIDTH];
	uint16_t *src = &scr->buffer[row * VGA_WIDTH];

	memcpy(dst, src, VGA_WIDTH * sizeof(uint16_t));

	h->current_line = (h->current_line + 1) % HISTORY_MAX_LINES;
	if (h->total_lines < HISTORY_MAX_LINES)
		h->total_lines++;
}

/*
** scroll_redraw() - Repaint VGA from history + live snapshot
**
** When scrolled back by scroll_offset lines, the VGA shows:
**   Row 0 .. (offset-1):  lines from history
**   Row offset .. 24:     lines from the saved live_snapshot
**
**  Example with scroll_offset = 2:
**    VGA row 0  ←  history[current_line - 2]
**    VGA row 1  ←  history[current_line - 1]
**    VGA row 2  ←  live_snapshot row 0
**    VGA row 3  ←  live_snapshot row 1
**    ...  
**    VGA row 24 ←  live_snapshot row 22
*/
static void scroll_redraw(screen *scr)
{
	ring_buffer *h = scr->history;
	size_t offset = h->scroll_offset;

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		uint16_t *dst = &scr->buffer[y * VGA_WIDTH];

		if (y < offset) {
			/* This row comes from history */
			size_t hist_idx = (h->current_line + HISTORY_MAX_LINES - offset + y)
							  % HISTORY_MAX_LINES;
			memcpy(dst, &h->buffer[hist_idx * VGA_WIDTH],
				   VGA_WIDTH * sizeof(uint16_t));
		} else {
			/* This row comes from the live snapshot */
			memcpy(dst, &h->live_snapshot[(y - offset) * VGA_WIDTH],
				   VGA_WIDTH * sizeof(uint16_t));
		}
	}
}

static bool scroll_up(screen *scr, ring_buffer *h)
{
	scr->cursor_visible = false;
/* Nothing in history to scroll to */
	if (h->total_lines == 0)
	   return (true);
	/* Already scrolled back as far as history allows */
	if (h->scroll_offset >= h->total_lines)
	   return (true);
	/* First time leaving live view: snapshot the current VGA */
	if (h->scroll_offset == 0)
	   memcpy(h->live_snapshot, scr->buffer,
	         VGA_BUFFER_SIZE * sizeof(uint16_t));
	h->scroll_offset++;
	scroll_redraw(scr);
	update_cursor_pos(scr);
	hide_cursor(scr);
	return (true);
}

static bool scroll_down(screen *scr, ring_buffer *h)
{
	/* Already at live view */
	if (h->scroll_offset == 0)
		return (true);
	h->scroll_offset--;
	if (h->scroll_offset == 0) {
		/* Back to live: restore the snapshot and re-enable cursor */
		memcpy(scr->buffer, h->live_snapshot,
			   VGA_BUFFER_SIZE * sizeof(uint16_t));
		scr->cursor_visible = true;
		show_cursor(scr);
	} else {
		scr->cursor_visible = false;
		scroll_redraw(scr);
		hide_cursor(scr);
	}
	return (true);
}

/*
** handle_scroll_key() - Page Up / Page Down through history
**   Page Up:   scroll back 1 line (show older content)
**   Page Down: scroll forward 1 line (back toward live view)
**   Returns true if the key was consumed, false otherwise.
*/
bool	handle_scroll_key(kfs *k, uint8_t scancode)
{
	if (scancode != KEY_PG_UP && scancode != KEY_PG_DOWN)
		return (false);

	screen *scr = kfs_active_screen(k);
	ring_buffer *h = scr->history;

	if (scancode == KEY_PG_UP) {
		return (scroll_up(scr, h));
	}
	else if (scancode == KEY_PG_DOWN) {
		return (scroll_down(scr, h));
	}
	return (false);
}

/*
** terminal_overflow_down() - Called when cursor goes past the last VGA row.
**   1. Save row 0 (about to be lost) into the ring buffer history
**   2. Shift all VGA rows up by one using memmove
**   3. Clear the last row
**   4. Keep cursor on the last row
**   5. Reset scroll_offset to 0 (snap back to live view)
*/
void terminal_overflow_down(screen *scr)
{
	if (scr->row < VGA_HEIGHT)
		return;

	/* Save the top row into history before it's overwritten */
	save_line_to_history(scr, 0);

	/* Shift rows 1..24 → 0..23 */
	memmove(&scr->buffer[0],
			&scr->buffer[VGA_WIDTH],
			(VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t));

	/* Clear the last row */
	for (size_t x = 0; x < VGA_WIDTH; x++)
		scr->buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry('\0', scr->color);

	/* Cursor stays on the last row */
	scr->row = VGA_HEIGHT - 1;

	/* Snap back to live view on new output */
	scr->history->scroll_offset = 0;
}

