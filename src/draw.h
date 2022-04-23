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

#define PAUSE_LABEL_WIDTH 225
#define PAUSE_LABEL_HEIGHT 140
#define PAUSE_LABEL_BORDER_WIDTH 3
#define PAUSE_LABEL_OUTER_X ((LCD_WIDTH - PAUSE_LABEL_WIDTH) / 2)
#define PAUSE_LABEL_OUTER_Y ((LCD_HEIGHT - PAUSE_LABEL_HEIGHT) / 2)
#define PAUSE_LABEL_INNER_X ((LCD_WIDTH - PAUSE_LABEL_WIDTH) / 2 + PAUSE_LABEL_BORDER_WIDTH)
#define PAUSE_LABEL_INNER_Y ((LCD_HEIGHT - PAUSE_LABEL_HEIGHT) / 2 + PAUSE_LABEL_BORDER_WIDTH)

#define PAUSE_LABEL_TEXT_X (PAUSE_LABEL_INNER_X + PAUSE_LABEL_BORDER_WIDTH + CHAR_HEIGHT)
#define PAUSE_LABEL_TEXT_Y (PAUSE_LABEL_INNER_Y + PAUSE_LABEL_BORDER_WIDTH + CHAR_HEIGHT)

#define PAUSE_LABEL_ROW_SPACING (CHAR_HEIGHT + 2)

#define DIGIT_SPRITE_WIDTH 8
#define DIGIT_SPRITE_HEIGHT 14

#define MENU_TOP_PADDING 40
#define MENU_LEFT_PADDING 40
#define MENU_MID_PADDING (LCD_WIDTH / 2)

// this macro depends on the colors being in the same order
// that the palette is in
#define DIGIT_TO_COLOR(digit) (digit)

#define X_PIXEL(v) ((v) * CELL_WIDTH + offset.x)
#define Y_PIXEL(v) ((v) * CELL_WIDTH + offset.y)

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
	LIGHT_GRAY
};

void draw_centered_text(char *s, int y, enum Color);

void draw_pause_screen_key_value(char *s, uint8_t num, int y);
void draw_pause_screen(void);

void set_palette(void);
void draw_menu(const char *);
void draw_board(struct Cell *, bool reveal, struct Vec2D clicked, bool partial_redraw);

#ifdef __cplusplus
}
#endif

#endif // DRAW_H
