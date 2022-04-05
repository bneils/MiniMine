#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern int24_t width, height, mines, xoffset, yoffset, xcur, ycur;

struct __attribute__((__packed__)) Cell {
	// An ad hoc bit I'm using to save on memory
	unsigned char floodfilled: 1;
	
	unsigned char flag: 1;
	unsigned char open: 1;
	unsigned char mine: 1;
	unsigned char surrounding: 4;
};

void cells_place_mines(struct Cell *);

#ifdef __cplusplus
}
#endif

#endif // BOARD_H
