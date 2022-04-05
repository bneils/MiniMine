#ifndef DRAW_H
#define DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "board.h"
#include <stdbool.h>

#define CELL_WIDTH 20

#define DIGIT_SPRITE_WIDTH 8
#define DIGIT_SPRITE_HEIGHT 14

// this macro depends on the colors being in the same order
// that the palette is in

#define DIGIT_TO_COLOR(digit) (digit)

enum Colors {
	TRANSPARENT = 0,
	BLUE,
	GREEN,
	RED,
	NAVY_BLUE,
	REDBROWN,
	CYAN,
	VERY_DARK_GRAY,
	BLACK,
	DARK_GRAY,
	WHITE,
	LIGHT_GRAY
};

void set_palette();
void draw_menu(const char *, const uint8_t w, const uint8_t h, const uint8_t m);
void draw_board(struct Cell *, bool reveal);

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
