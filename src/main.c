#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "draw.h"

// TODO:
// [x] More efficient graphics routines by using partial redraw
// [ ] ^ Make redraws faster, in general
// [x] Better flood filling (sacrifice memory for speed)
// [x] Add a better way to quit after failure
// [x] Win check
// [ ] Flashing rectangle that announces if you won or not (Dynamically size w/ a border)
// [ ] Pause screen that shows mine count, time elapsed, 
// [x] First click is always zero
// [x] Have screen follow cursor if it goes off screen
// [ ] Save / resume a game?
// [x] Remember last difficulty used
// [ ] Increment in larger amounts when moving cursor for X amount of times in a row OR when the screen has to shift
// [x] Add an arrow when the cursor is near the edge of the screen that indicates the screen doesn't end there
//     ^ Or, change the side of the cursor to be a different color. (It's a square)

// I don't want to pass a million parameters to each function.
// This also lets me avoid having some config struct that I have to
// constantly dereference. Also, I hate having to change function prototypes.
int width, height, mines, size;
int xcur, ycur; // in cells
int xoffset, yoffset; // in px

int menu_screen(void);
void gameloop(void);

/* 0 on success */
int menu_screen(void) {
	const char *diffs[] = {
		"Easy>",
		"<Medium>",
		"<Hard"
	};
	
	const uint8_t settings[][3] = {
		{9, 9, 10},
		{16, 16, 40},
		{30, 16, 99}
	};
	
	uint8_t key = 0;
	static uint8_t difficulty = 0;
	
	// Menu screen
	for (;;) {
		if (key == sk_Left && difficulty > 0) {
			--difficulty;
		}
		if (key == sk_Right && difficulty < sizeof(diffs) / sizeof(diffs[0]) - 1) {
			++difficulty;
		}
		if (key == sk_Clear) {
			return 1;
		}
		if (key == sk_2nd) {
			return 0;
		}
		
		width = settings[difficulty][0];
		height = settings[difficulty][1];
		mines = settings[difficulty][2];
		size = width * height;
		
		xcur = width / 2;
		ycur = height / 2;
		
		xoffset = (LCD_WIDTH - width * CELL_WIDTH) / 2;
		yoffset = (LCD_HEIGHT - height * CELL_WIDTH) / 2;
		
		draw_menu(diffs[difficulty]);
		gfx_SwapDraw();
		
		while (!(key = os_GetCSC()))
			;
	}
}

/* 0 on continue, 1 on exit */
void gameloop(void) {
	struct Cell cells[MAX_CELLS * sizeof(struct Cell)];
	
	for (;;) {
		// Set global config
		if (menu_screen()) {
			break;
		}
		
		// This'll never happen, but I want to convey that I'm avoiding using malloc by pre-allocating.
		if (size > MAX_CELLS) {
			break;
		}

		cells_init();
		
		bool can_interact = true, died = false, board_generated = false, running = true;
		int clicked_x = 0, clicked_y = 0;
		
		bool force_redraw = true;
		
		while (running) {
			// Sometimes the cursor goes offscreen, so if that happens the offset is changed.
			int pixel;
			int unmodified_offsets = 0;
			
			pixel = X_PIXEL(xcur);
			if (pixel >= LCD_WIDTH) xoffset -= CELL_WIDTH;
			else if (pixel < 0) xoffset += CELL_WIDTH;
			else ++unmodified_offsets;
			
			pixel = Y_PIXEL(ycur);
			if (pixel >= LCD_HEIGHT) yoffset -= CELL_WIDTH;
			else if (pixel < 0) yoffset += CELL_WIDTH;
			else ++unmodified_offsets;
			
			draw_board(cells, died, clicked_x, clicked_y, unmodified_offsets == 2 && !force_redraw);
			force_redraw = false;
			gfx_SwapDraw();
			
			struct Cell *cell = &cells[cursor_pos()];
			bool movement_key;
			uint8_t key;
wait_poll_key:
			while (!(key = os_GetCSC()))
				;
			
			switch (key) {
				case sk_Left:
				case sk_Right:
				case sk_Down:
				case sk_Up:
					movement_key = true;
					cell->changed = true;
					break;
				default:
					movement_key = false;
					break;
			}
			
			switch (key) {
				// Quit the app
				case sk_Clear:
					running = false;
					break;
				// Movement controls
				case sk_Left:
					if (xcur > 0) --xcur;
					break;
				case sk_Right:
					if (xcur < width - 1) ++xcur;
					break;
				case sk_Up:
					if (ycur > 0) --ycur;
					break;
				case sk_Down:
					if (ycur < height - 1) ++ycur;
					break;
				case sk_2nd:
					if (!can_interact) {
						running = false;
						break;
					}

					// I want to make sure the first click is a "good" one
					if (!board_generated) {
						cells_place_mines(cells);
						board_generated = true;
					}
					
					clicked_x = xcur;
					clicked_y = ycur;
					
					if (cells_click(cells, xcur, ycur)) {
						died = true;
						force_redraw = true;
						can_interact = false;
					}
					
					// Scan the entire board to check if the win condition is met.
					// I could alternatively decrement a counter everytime I opened something, but this is easier
					int num_unknown = size;
					for (struct Cell *right = &cells[size - 1]; right >= cells; --right) {
						num_unknown -= right->open;
					}
					
					if (num_unknown == mines) {
						can_interact = false;
						force_redraw = true;
						// All mines should be made flags when the game is won
						for (struct Cell *right = &cells[size - 1]; right >= cells; --right) {
							if (!right->open) {
								right->flag = 1;
							}
						}
					}
					
					break;
				case sk_Alpha:
					// Any interaction (Mutation) with the board when they're not allowed
					// indicates the only thing they can do is quit
					if (!can_interact) {
						running = false;
						break;
					}
					if (!cell->open) {
						cell->changed = true;
						cell->flag = !cell->flag;
					}
					break;
				default:
					goto wait_poll_key;
			}
			
			if (movement_key) {
				cells[cursor_pos()].changed = true;
			}
		}
	}
}

int main(void) {
	gfx_Begin();
	gfx_SetDrawBuffer();
	set_palette();
	srandom(clock()); // hopefully better than rtc_Time(), b/c of ram resets
	gameloop();
	gfx_End();
	return 0;
}
