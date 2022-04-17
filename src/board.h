#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CELLS (16 * 30)
#define cells_init() { memset(cells, 0, size * sizeof(struct Cell)); }

// You might cringe at this, but it's the best solution I have now
extern int width, height, mines, xoffset, yoffset, xcur, ycur, size;

struct __attribute__((__packed__)) Cell {
	// This bit is being used for partial redraws.
	unsigned char changed: 1;
	
	unsigned char flag: 1;
	unsigned char open: 1;
	unsigned char mine: 1;
	unsigned char surrounding: 4;	// Only mutated during board setup
};

bool check_bounds(int x, int y);
void cells_place_mines(struct Cell *);
bool cells_click(struct Cell *cells, int x, int y);

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
