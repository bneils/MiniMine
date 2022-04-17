#ifndef DRAW_H
#define DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define CELL_WIDTH 20

#define DIGIT_SPRITE_WIDTH 8
#define DIGIT_SPRITE_HEIGHT 14

// this macro depends on the colors being in the same order
// that the palette is in
#define DIGIT_TO_COLOR(digit) (digit)

#define X_PIXEL(x) ((x) * CELL_WIDTH + xoffset)
#define Y_PIXEL(y) ((y) * CELL_WIDTH + yoffset)

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
void draw_menu(const char *);
void draw_board(struct Cell *, bool reveal, int clicked_x, int clicked_y, bool partial_redraw);
void draw_text_label(char *s, int x, int y);

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
