#ifndef DRAW_H
#define DRAW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define CELL_WIDTH 20
#define CHAR_HEIGHT 8

#define PANEL_WIDTH 200
#define PANEL_HEIGHT (PANEL_WIDTH * LCD_HEIGHT / LCD_WIDTH)
#define PANEL_THICKNESS 3
#define PANEL_OUTER_X ((LCD_WIDTH - PANEL_WIDTH) / 2)
#define PANEL_OUTER_Y ((LCD_HEIGHT - PANEL_HEIGHT) / 2)
#define PANEL_INNER_X (PANEL_OUTER_X + PANEL_THICKNESS)
#define PANEL_INNER_Y (PANEL_OUTER_Y + PANEL_THICKNESS)

// Empirically derived
#define PANEL_TEXT_MARGIN 10

#define DIGIT_SPRITE_WIDTH 8
#define DIGIT_SPRITE_HEIGHT 14

#define MENU_TOP_PADDING 40
#define MENU_LEFT_PADDING 40
#define MENU_MID_PADDING (LCD_WIDTH / 2)

// this macro depends on the colors being in the same order
// that the palette is in
#define DIGIT_TO_COLOR(digit) (digit)

#define X_PIXEL(v) ((v) * CELL_WIDTH + g_offset.x)
#define Y_PIXEL(v) ((v) * CELL_WIDTH + g_offset.y)

enum Alignment {
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

// these are the colors of digits
enum Color {
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
	LIGHT_GRAY,
	YELLOW, // misc.
};

void draw_panel_canvas(void);
void draw_panel_text(const char *, int row, enum Alignment);
void draw_panel_selection(int row);

void set_palette(void);
void draw_menu(enum MenuOption);
void draw_board(
	struct Cell *,
	bool reveal,
	struct Vec2D clicked,
	bool partial_redraw
);

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
