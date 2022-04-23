#include <stdint.h>
#include <stdlib.h>
#include <tice.h>

#include "board.h"
#include <stdio.h> // TENT
#include <graphx.h>

int cursor_pos(void) {
	return cur.y * width + cur.x;
}

/* commonly used in loops to check if surrounding positions are valid */
bool check_bounds(int x, int y) {
	return x >= 0 && y >= 0 && x < width && y < height;
}

/* cells must have the same dimensions as the global variables */
void cells_place_mines(struct Cell *cells) {
	for (int i = 0; i < mines; ++i) {
		int idx;
		struct Vec2D pos;
		// Make sure not to generate a mine such as to make (xcur, ycur)
		// a mine, or a label with a number > 0. This will fail (infinite loop)
		// if mines is too big that enough cells won't be available.
		do {
			idx = random() % size;
			pos.x = idx % width;
			pos.y = idx / width;
		} while (cells[idx].mine || (abs(pos.x - cur.x) <= 1 && abs(pos.y - cur.y) <= 1));
		
		cells[idx].mine = true;
		for (int i = pos.y - 1; i <= pos.y + 1; ++i) {
			for (int j = pos.x - 1; j <= pos.x + 1; ++j) {
				if (check_bounds(j, i)) {
					++cells[i * width + j].surrounding;
				}
			}
		}
	}
}

/* Perform the action of clicking on a cell. 
 * Returns whether a mine was activated (dead).
 */
bool cells_click(struct Cell *cells, struct Vec2D pos) {
	struct Cell *cell = &cells[pos.y * width + pos.x];
	
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
				struct Cell *surrounding_cell = &cells[i * width + j];
				if (check_bounds(j, i) && surrounding_cell->flag) {
					++num_flags;
				}
			}
		}

		if (num_flags == cell->surrounding) {
			for (int i = pos.y - 1; i <= pos.y + 1; ++i) {
				for (int j = pos.x - 1; j <= pos.x + 1; ++j) {
					struct Cell *surrounding_cell = &cells[i * width + j];
					struct Vec2D surrounding_pos = { .x = j, .y = i };
					if (check_bounds(j, i) && !surrounding_cell->open && cells_click(cells, surrounding_pos)) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	if (cell->surrounding) {
		cell->open = true;
		cell->changed = true;
	} else {
		// Begin flood fill
		// To explain this, left chases the right pointer until they meet.
		// As left pointer moves to the right, it assigns new values at the right pointer (more than one) and incrementing that.
		int next_cells[MAX_CELLS];
		int *left = &next_cells[0];
		int *right = &next_cells[1];
		
		*left = pos.y * width + pos.x;
		
		for (; left < right; ++left) {
			struct Vec2D next = { 
				.x = *left % width,
				.y = *left / width
			};
			
			for (int i = next.y - 1; i <= next.y + 1; ++i) {
				for (int j = next.x - 1; j <= next.x + 1; ++j) {
					int idx = i * width + j;
					if (check_bounds(j, i) && !cells[idx].open && !cells[idx].flag) {
						cells[idx].open = true;
						cells[idx].changed = true;
						if (cells[idx].surrounding == 0) {
							*right++ = idx;
						}
					}
				}
			}
		}
	}
	return false;
}
