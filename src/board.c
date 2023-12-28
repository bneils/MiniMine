#include <stdint.h>
#include <stdlib.h>

#include <tice.h>
#include <graphx.h>

#include "board.h"

int cursor_pos(void) {
	return g_cur.y * g_width + g_cur.x;
}

/* commonly used in loops to check if surrounding positions are valid */
bool check_bounds(int x, int y) {
	return x >= 0 && y >= 0 && x < g_width && y < g_height;
}

/* cells must have the same dimensions as the global variables */
void cells_place_mines(struct Cell *cells) {
	for (int m = 0; m < g_mines; ++m) {
		unsigned idx;
		struct Vec2D pos;
		// Make sure not to generate a mine such as to make (xcur, ycur)
		// a mine, or a label with a number > 0. This will cause an infinite
		// loop if mines is too big that enough cells won't be available.
		do {
			idx = random() % g_size;
			pos.x = idx % g_width;
			pos.y = idx / g_width;
		} while (cells[idx].mine || (abs(pos.x - g_cur.x) <= 1 &&
		         abs(pos.y - g_cur.y) <= 1));

		cells[idx].mine = true;
		for (int i = pos.y - 1; i <= pos.y + 1; ++i) {
			for (int j = pos.x - 1; j <= pos.x + 1; ++j) {
				if (check_bounds(j, i)) {
					++cells[i * g_width + j].surrounding;
				}
			}
		}
	}
}

/* Perform the action of clicking on a cell.
 * Returns whether a mine was activated (dead).
 */
bool cells_click(struct Cell *cells, struct Vec2D pos) {
	struct Cell *cell = &cells[pos.y * g_width + pos.x];

	if (cell->flag) {
		return false;
	}

	if (cell->mine) {
		return true;
	}

	// We want to open a 3 x 3
	if (cell->open) {
		// We first need to make sure the number of surrounding flags
		// is equal to the cell's number
		int num_flags = 0;
		for (int i = pos.y - 1; i <= pos.y + 1; ++i) {
			for (int j = pos.x - 1; j <= pos.x + 1; ++j) {
				struct Cell *surrounding_cell = &cells[i * g_width + j];
				if (check_bounds(j, i) && surrounding_cell->flag) {
					++num_flags;
				}
			}
		}

		if (num_flags == cell->surrounding) {
			for (int i = pos.y - 1; i <= pos.y + 1; ++i) {
				for (int j = pos.x - 1; j <= pos.x + 1; ++j) {
					struct Cell *surrounding_cell = &cells[i * g_width + j];
					if (check_bounds(j, i) && !surrounding_cell->open &&
					    surrounding_cell->mine) {
						return true;
					}
					surrounding_cell->open = true;
					surrounding_cell->gfxupdate = true;
				}
			}
		}
		return false;
	}

	if (cell->surrounding) {
		cell->open = true;
		cell->gfxupdate = true;
	} else {
		// Flood fill using a FIFO queue data structure
		// using an array implementation.
		struct Vec2D next_cells[MAX_CELLS];
		struct Vec2D *left = &next_cells[0];
		struct Vec2D *right = &next_cells[1];

		*left = pos;

		for (; left < right; ++left) {
			struct Vec2D next = *left;

			for (int i = next.y - 1; i <= next.y + 1; ++i) {
				for (int j = next.x - 1; j <= next.x + 1; ++j) {
					int idx = i * g_width + j;
					if (check_bounds(j, i) && !cells[idx].open &&
					    !cells[idx].flag) {
						cells[idx].open = true;
						cells[idx].gfxupdate = true;
						if (cells[idx].surrounding == 0) {
							*right++ = (struct Vec2D) {.x = j, .y = i};
						}
					}
				}
			}
		}
	}
	return false;
}
