#include <stdint.h>
#include <stdlib.h>
#include <tice.h>

#include "board.h"

int cursor_pos(void) {
	return ycur * width + xcur;
}

/* commonly used in loops to check if surrounding positions are valid */
bool check_bounds(int x, int y) {
	return x >= 0 && y >= 0 && x < width && y < height;
}

/* cells must have the same dimensions as the global variables */
void cells_place_mines(struct Cell *cells) {
	for (int i = 0; i < mines; ++i) {
		int pos, x, y;
		// Make sure not to generate a mine such as to make (xcur, ycur)
		// a mine, or a label with a number > 0. This will fail (infinite loop)
		// if mines is too big that enough cells won't be available.
		do {
			pos = random() % size;
			x = pos % width;
			y = pos / width;
		} while (cells[pos].mine || (abs(x - xcur) <= 1 && abs(y - ycur) <= 1));
		
		cells[pos].mine = 1;
		for (int i = y - 1; i <= y + 1; ++i) {
			for (int j = x - 1; j <= x + 1; ++j) {
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
bool cells_click(struct Cell *cells, int x, int y) {
	struct Cell *cell = &cells[y * width + x];
	if (cell->flag) {
		return false;
	}
	
	// We want to open a 3 x 3
	if (cell->open) {
		// We first need to make sure the number of surrounding flags 
		// is equal to the cell's number
		int num_flags = 0;
		for (int i = y - 1; i <= y + 1; ++i) {
			for (int j = x - 1; j <= x + 1; ++j) {
				struct Cell *surrounding_cell = &cells[i * width + j];
				if (check_bounds(j, i) && surrounding_cell->flag) {
					++num_flags;
				}
			}
		}
		
		if (num_flags != cell->surrounding) {
			return false;
		}
		
		for (int i = y - 1; i <= y + 1; ++i) {
			for (int j = x - 1; j <= x + 1; ++j) {
				struct Cell *surrounding_cell = &cells[i * width + j];
				if (check_bounds(j, i) && !surrounding_cell->open && cells_click(cells, j, i)) {
					return true;
				}
			}
		}
		return false;
	}
	
	if (cell->mine) {
		return true;
	}
	
	cell->changed = true;
	cell->open = 1;
	
	// Begin flood fill
	if (cell->surrounding == 0) {
		// To explain this, left chases the right pointer until they meet.
		// As left pointer moves to the right, it assigns new values at the right pointer (more than one) and incrementing that.
		int next_cells[MAX_CELLS];
		int *left = &next_cells[0];
		int *right = &next_cells[1];
		
		*left = y * width + x;
		
		for (; left < right; ++left) {
			int y = *left / width;
			int x = *left % width;
			for (int i = y - 1; i <= y + 1; ++i) {
				for (int j = x - 1; j <= x + 1; ++j) {
					int pos = i * width + j;
					if (check_bounds(j, i) && !cells[pos].open && !cells[pos].flag) {
						cells[pos].open = 1;
						cells[pos].changed = true;
						if (cells[pos].surrounding == 0) {
							*right++ = pos;
						}
					}
				}
			}
		}
	}
	return false;
}
