#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include "board.h"
#include "draw.h"

int g_width;
int g_height;
int g_mines;
int g_size;
int g_flags;

// Used to measure time not spent paused
int g_seconds_elapsed;

struct Vec2D g_cur; // in cells
struct Vec2D g_offset; // in px

static enum MenuOption menu_option;

static const char *difficulty_names[] = {
	"Easy", "Medium", "Hard"
};

/* The function selection_prompt() works by managing an index in an interval
 * based on the calculator's arrow keys. It then invokes a callback function
 * with: what the user pressed on the calculator, and what the index is now.
 * It will invoke the callback if the user presses a key, regardless of whether
 * it affected the index.
 *
 * The function takes as parameters:
 *  - The lower bound
 *  - The upper bound
 *  - The callback
 *
 * The callback should return 0 to exit the loop.
 * The function returns the index of the user's option.
 */
int selection_prompt(int, int, int (*)(int key, int idx));

/* The menu_screen() function is a callback of selection_prompt().
 * It asks the user to configure the game variables.
 * The struct g_board is set if 0 is returned, but not otherwise.
 */
int menu_screen(int, int);

/* The pause_screen() function is a callback of selection_prompt().
 * It gives the user the opportunity to choose from a list of pause screen
 * options.
 */
int pause_screen(int, int);

/* The gameloop() function begins the game loop, performs all rendering,
 * all game logic, etc.
 */
void gameloop(void);

int main(void) {
	gfx_Begin();
	gfx_SetDrawBuffer();
	set_palette();
	gameloop();
	gfx_End();
	return 0;
}

int selection_prompt(int lower, int upper, int (*callback)(int key, int idx)) {
	int current = lower;
	int key = 0;

	while (callback(key, current)) {
		while (!(key = os_GetCSC()))
			;
		switch (key) {
			case sk_Up:
				if (current > lower) --current;
				break;
			case sk_Down:
				if (current < upper) ++current;
				break;
		}
	}
	return current;
}

void gameloop(void) {
	struct Cell cells[MAX_CELLS * sizeof(struct Cell)];

	for (;;) {
		// Set global config
		int menu_option = selection_prompt(MENU_EASY, MENU_EXIT, menu_screen);
		if (menu_option == MENU_EXIT) {
			return;
		}

		// I need to zero initialize the buffer because it needs to be displayed,
		// and the actual board generation (that should do this) happens later.
		memset(cells, 0, g_size * sizeof(struct Cell));

		bool force_redraw = true;
		bool can_interact, died, board_generated, running;
		can_interact = running = true;
		died = board_generated = false;

		struct Vec2D clicked;
		clicked.x = clicked.y = 0;

		// Denotes the beginning of a time duration, where the end is
		// whenever the pause screen is shown. last_game_timestamp is undefined
		// until the board is generated.
		clock_t last_game_timestamp, clocks_elapsed = 0;

		while (running) {
			// Sometimes the cursor goes offscreen, so if that happens the
			// g_offset is changed.
			// This is also used to calculate whether or not we need a redraw.

			int pixel;
			int num_unmodified_offsets = 0;

			pixel = X_PIXEL(g_cur.x);
			if (pixel >= LCD_WIDTH) g_offset.x -= CELL_WIDTH;
			else if (pixel < 0) g_offset.x += CELL_WIDTH;
			else ++num_unmodified_offsets;

			pixel = Y_PIXEL(g_cur.y);
			if (pixel >= LCD_HEIGHT) g_offset.y -= CELL_WIDTH;
			else if (pixel < 0) g_offset.y += CELL_WIDTH;
			else ++num_unmodified_offsets;

			draw_board(cells, died, clicked,
				num_unmodified_offsets == 2 && !force_redraw);
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
				// Movement controls
				case sk_Left:
					if (g_cur.x > 0) --g_cur.x;
					break;
				case sk_Right:
					if (g_cur.x < g_width - 1) ++g_cur.x;
					break;
				case sk_Up:
					if (g_cur.y > 0) --g_cur.y;
					break;
				case sk_Down:
					if (g_cur.y < g_height - 1) ++g_cur.y;
					break;
				case sk_2nd:
					if (!can_interact) {
						running = false;
						break;
					}

					// I want to make sure the first click is a "good" one
					if (!board_generated) {
						// This should prevent the user from getting the same 
						// thing EVEN after ram resets.
						last_game_timestamp = clock();
						srandom(last_game_timestamp);
						cells_place_mines(cells);
						board_generated = true;
					}

					// When a mine is hit we need to render what mine was hit
					clicked = g_cur;

					if (cells_click(cells, g_cur)) {
						died = true;
						can_interact = false;
						force_redraw = true;
						draw_board(cells, died, clicked, !force_redraw);
						// Tell the player they have no skill
						// You need to force a redraw here because it won't
						// happen until after the pause screen is shown
						draw_panel_canvas();
						gfx_SetTextFGColor(RED);
						draw_panel_text("You lost!", 2, ALIGN_CENTER);
						gfx_SetTextFGColor(BLACK);
						draw_panel_text("Practice makes progress.", 3,
							ALIGN_CENTER);
						draw_panel_text("To continue, press 2nd.", 4,
							ALIGN_LEFT);
						gfx_SwapDraw();
						msleep(250);
						while (!(os_GetCSC()))
							;
					}

					// Scan the entire board to check if the win condition is
					// met. I could alternatively decrement a counter everytime
					// I opened something, but this is easier
					int num_unknown = g_size;
					for (struct Cell *right = &cells[g_size - 1];
						right >= cells;
						--right) {
						num_unknown -= right->open;
					}

					if (num_unknown == g_mines) {
						can_interact = false;
						force_redraw = true;
						// All mines should be made flags when the game is won
						for (struct Cell *right = &cells[g_size - 1];
							right >= cells;
							--right) {
							if (!right->open) {
								right->flag = 1;
							}
						}

						// Tell the player they're adequate
						draw_board(cells, died, clicked, !force_redraw);
						draw_panel_canvas();
						gfx_SetColor(BLACK);
						draw_panel_text("You won!", 0, ALIGN_CENTER);
						draw_panel_text("Good job!", 1, ALIGN_CENTER);
						gfx_SwapDraw();
						msleep(500);
						while (!(os_GetCSC()))
							;
					}

					break;
				case sk_Alpha:
					// Any interaction (Mutation) with the board when they're
					// not allowed indicates the only thing they can do is quit
					if (!can_interact) {
						running = false;
						break;
					}
					if (!cell->open) {
						cell->changed = true;
						if (cell->flag) --g_flags;
						else			++g_flags;
						cell->flag = !cell->flag;
					}
					break;
				case sk_Mode:
					// Bring up the pause screen
					if (board_generated) {
						clock_t now = clock();
						clocks_elapsed += now - last_game_timestamp;
						g_seconds_elapsed = clocks_elapsed / CLOCKS_PER_SEC;
						if (g_seconds_elapsed > 999) {
							g_seconds_elapsed = 999;
						}
					} else {
						g_seconds_elapsed = 0;
					}

					enum PauseOption option = selection_prompt(
						PAUSE_RESUME, PAUSE_QUIT, pause_screen
					);
					switch (option) {
						case PAUSE_QUIT:
							running = false;
							break;
						case PAUSE_RESUME:
							break;
					}

					last_game_timestamp = clock();
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

int menu_screen(int key_pressed, int selection) {
	const uint8_t settings[][3] = {
		{9, 9, 10},
		{16, 16, 40},
		{30, 16, 99}
	};

	if (key_pressed == sk_2nd) {
		return 0;
	}

	// Assigning these values is important before calling draw_menu().
	// The function uses these global values to tell the user the board
	// dimensions.
	if (selection != MENU_EXIT) {
		g_width = settings[selection][0];
		g_height = settings[selection][1];
		g_mines = settings[selection][2];
		g_size = g_width * g_height;
		g_cur.x = g_width / 2;
		g_cur.y = g_height / 2;

		g_offset.x = (LCD_WIDTH - g_width * CELL_WIDTH) / 2;
		g_offset.y = (LCD_HEIGHT - g_height * CELL_WIDTH) / 2;
		g_flags = 0;
	}

	draw_menu((enum MenuOption)selection);
	gfx_SwapDraw();

	return 1;
}

int pause_screen(int key_pressed, int selection) {
	gfx_BlitScreen();
	draw_panel_canvas();

	char buf[10];
	int effective_mines = g_mines - g_flags;
	sprintf(buf, "%d/%d", (effective_mines >= 0) ? effective_mines : 0, g_mines);
	gfx_SetTextFGColor(BLACK);
	draw_panel_text(buf, 0, ALIGN_LEFT);
	draw_panel_text(difficulty_names[menu_option], 0, ALIGN_CENTER);

	sprintf(buf, "%03d", g_seconds_elapsed);
	draw_panel_text(buf, 0, ALIGN_RIGHT);

	draw_panel_text("Resume", 2, ALIGN_LEFT);
	draw_panel_text("Quit", 3, ALIGN_LEFT);

	draw_panel_selection(selection + 2);
	gfx_SwapDraw();
	return key_pressed != sk_2nd;
}
