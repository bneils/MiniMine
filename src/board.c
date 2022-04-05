#include "board.h"
#include <string.h>
#include <stdint.h>
#include <tice.h>

/* cells must have the same dimensions as the global variables */
void cells_place_mines(struct Cell *cells) {
	int24_t size = width * height;
	memset(cells, 0, size * sizeof(struct Cell));
	for (int24_t i = 0; i < mines; ++i) {
		int24_t pos;
		do {
			pos = random() % size;
		} while (cells[pos].mine);
		
		cells[pos].mine = 1;
		int24_t x = pos % width;
		for (int24_t yoff = -width; yoff <= width; yoff += width)
			for (int24_t xoff = -1; xoff <= 1; ++xoff) {
				int24_t new_x = x + xoff;
				int24_t new_pos = pos + yoff + xoff;
				if (new_x >= 0 && new_x < width && new_pos >= 0 && new_pos < size)
					++cells[new_pos].surrounding;
			}
	}
}
