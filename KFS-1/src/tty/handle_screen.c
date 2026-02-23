#include "kernel.h"

/*
** switch_screen() - Switch between virtual screens
**
** How it works:
**   1. Save current VGA content → old screen's saved_buffer (RAM copy)
**   2. Point old screen's buffer to its saved_buffer (writes go to RAM now)
**   3. Copy new screen's saved_buffer → VGA memory (restore its content)
**   4. Point new screen's buffer to VGA_MEMORY (writes go to hardware now)
**   5. Update cursor to new screen's position
**
** This is why each screen has TWO pointers:
**   - saved_buffer: always points to the same RAM slot (set once in init, never changes)
**   - buffer:       swapped between VGA_MEMORY (active) and saved_buffer (inactive)
*/
void switch_screen(kfs *k, int screen_index)
{
	if (screen_index < 0 || screen_index >= k->total_screens)
		return;
	if (screen_index == k->current_screen)
		return;

	screen *old_scr = &k->screens[k->current_screen];
	screen *new_scr = &k->screens[screen_index];

	/* Save what's on the monitor → old screen's private RAM */
	memcpy(old_scr->saved_buffer, (void*)VGA_MEMORY,
		VGA_BUFFER_SIZE * sizeof(uint16_t));
	old_scr->buffer = old_scr->saved_buffer;

	/* Restore new screen's RAM copy → monitor */
	memcpy((void*)VGA_MEMORY, new_scr->saved_buffer,
		VGA_BUFFER_SIZE * sizeof(uint16_t));
	new_scr->buffer = (uint16_t*)VGA_MEMORY;

	k->current_screen = screen_index;
	update_cursor_pos(new_scr);
}




