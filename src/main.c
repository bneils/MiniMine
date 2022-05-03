#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "board.h"
#include "draw.h"

// I don't want to pass a million parameters to each function.
// This also lets me avoid having some config struct that I have to
// constantly dereference. Also, I hate having to change function prototypes.
int width, height, mines, size;
struct Vec2D cur; // in cells
struct Vec2D offset; // in px

int menu_screen(void);
void gameloop(void);

/*
 * Asks the user to configure global game variables.
 * width, height, mines, offset.x, offset.y, cur.x, cur.y, size are set if 0 is returned.
 */
int menu_screen(void) {	
	const uint8_t settings[][3] = {
		{9, 9, 10},
		{16, 16, 40},
		{30, 16, 99}
	};
	
	uint8_t key = 0;
	static enum Difficulty difficulty = EASY;
	
	// Menu screen
	for (;;) {
		if (key == sk_Up && difficulty > EASY) {
			--difficulty;
		} else if (key == sk_Down && difficulty < HARD) {
			++difficulty;
		} else if (key == sk_2nd) {
			return 0;
		} else if (key == sk_Clear) {
			return 1;
		}
		
		width = settings[difficulty][0];
		height = settings[difficulty][1];
		mines = settings[difficulty][2];		
		size = width * height;
	
		cur.x = width / 2;
		cur.y = height / 2;
		
		offset.x = (LCD_WIDTH - width * CELL_WIDTH) / 2;
		offset.y = (LCD_HEIGHT - height * CELL_WIDTH) / 2;
	
		draw_menu(difficulty);
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
		
		// I need to zero initialize the buffer because it needs to be displayed,
		// and the actual board generation (that should do this) happens later.
		memset(cells, 0, size * sizeof(struct Cell));
		
		bool force_redraw = true;
		bool can_interact = true, died = false, board_generated = false, running = true;
		struct Vec2D clicked;
		clicked.x = clicked.y = 0;
		
		cells_place_mines(cells);
		board_generated = true;
		
		while (running) {
			// Sometimes the cursor goes offscreen, so if that happens the offset is changed.
			int pixel;
			int num_unmodified_offsets = 0;
			
			pixel = X_PIXEL(cur.x);
			if (pixel >= LCD_WIDTH) offset.x -= CELL_WIDTH;
			else if (pixel < 0) offset.x += CELL_WIDTH;
			else ++num_unmodified_offsets;
			
			pixel = Y_PIXEL(cur.y);
			if (pixel >= LCD_HEIGHT) offset.y -= CELL_WIDTH;
			else if (pixel < 0) offset.y += CELL_WIDTH;
			else ++num_unmodified_offsets;

			draw_board(cells, died, clicked, num_unmodified_offsets == 2 && !force_redraw);			
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
					if (cur.x > 0) --cur.x;
					break;
				case sk_Right:
					if (cur.x < width - 1) ++cur.x;
					break;
				case sk_Up:
					if (cur.y > 0) --cur.y;
					break;
				case sk_Down:
					if (cur.y < height - 1) ++cur.y;
					break;
				case sk_2nd:
					if (!can_interact) {
						running = false;
						break;
					}

					// I want to make sure the first click is a "good" one
					if (!board_generated) {
						// This should prevent the user from getting the same thing EVEN after ram resets.
						srandom(clock());
						cells_place_mines(cells);
						board_generated = true;
					}
					
					// When a mine is hit we need to render what mine was hit
					clicked = cur;
					
					if (cells_click(cells, cur)) {
						died = true;
						can_interact = false;
						force_redraw = true;
						draw_board(cells, died, clicked, !force_redraw);
						// Tell the player they have no skill
						// You need to force a redraw here because it won't happen until
						// after the pause screen is shown
						draw_panel_canvas();
						gfx_SetColor(BLACK);
						draw_panel_text("You lost!", 0, CENTER);
						draw_panel_text("Get gud. ;(", 1, CENTER);
						gfx_SwapDraw();
						msleep(500);
						while (!(os_GetCSC()))
							;
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
						
						// Tell the player they're adequate
						draw_board(cells, died, clicked, !force_redraw);
						draw_panel_canvas();
						gfx_SetColor(BLACK);
						draw_panel_text("You won!", 0, CENTER);
						draw_panel_text("Good job!", 1, CENTER);
						gfx_SwapDraw();
						msleep(500);
						while (!(os_GetCSC()))
							;
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
				case sk_Mode:
					// Bring up the pause screen, which can show information
					gfx_BlitScreen();
					draw_panel_canvas();
					
					int nflags = 0;
					for (int i = 0; i < size; ++i) {
						if (cells[i].flag) {
							++nflags;
						}
					}
					
					char buf[15];
					sprintf(buf, "%d/%d", (nflags <= mines) ? mines - nflags : 0, mines);
					gfx_SetColor(BLACK);
					draw_panel_text(buf, 0, LEFT);
					gfx_SwapDraw();
					while (!(os_GetCSC()))
						;
					force_redraw = true;
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
	gameloop();
	gfx_End();
	return 0;
}
