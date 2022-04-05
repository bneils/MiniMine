#include "draw.h"
#include "gfx.h"
#include "board.h"
#include <graphx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <tice.h>

#define MENU_TOP_PADDING 40
#define MENU_LEFT_PADDING 40
#define MENU_MID_PADDING (LCD_WIDTH / 2)
#define CHAR_HEIGHT 8

static void draw_digit(int24_t digit, int24_t x, int24_t y);
static void draw_unopened_tile(int24_t x, int24_t y);

void set_palette() {
	gfx_SetTransparentColor(TRANSPARENT);
	gfx_SetTextTransparentColor(0);
	gfx_SetPalette(mypalette, sizeof(mypalette), 0);
}

void draw_menu(const char *difficulty, const uint8_t w, const uint8_t h, const uint8_t m) {
	const char *title = "MiniMines by Ben Neilsen";
	const char *tip = "2nd to continue";
	char buf[4];
	
	gfx_SetTextConfig(gfx_text_clip);
	
	gfx_FillScreen(BLACK);
	gfx_SetTextFGColor(BLUE);
	gfx_SetTextBGColor(TRANSPARENT);
	gfx_PrintStringXY(
		title, 
		(LCD_WIDTH - gfx_GetStringWidth(title)) / 2,
		MENU_TOP_PADDING
	);
	
	gfx_SetTextFGColor(WHITE);
	
	gfx_PrintStringXY(
		difficulty,
		(LCD_WIDTH - gfx_GetStringWidth(difficulty)) / 2,
		MENU_TOP_PADDING + CHAR_HEIGHT * 6
	);
	
	sprintf(buf, "%d", w);
	
	gfx_PrintStringXY(
		"Width:",
		MENU_LEFT_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 7
	);
	
	gfx_PrintStringXY(
		buf,
		MENU_MID_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 7
	);
	
	sprintf(buf, "%d", h);
	
	gfx_PrintStringXY(
		"Height:",
		MENU_LEFT_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 8
	);
	
	gfx_PrintStringXY(
		buf,
		MENU_MID_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 8
	);

	sprintf(buf, "%d", m);
	
	gfx_PrintStringXY(
		"Mines:",
		MENU_LEFT_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 9
	);
	
	gfx_PrintStringXY(
		buf,
		MENU_MID_PADDING,
		MENU_TOP_PADDING + CHAR_HEIGHT * 9
	);
	
	gfx_PrintStringXY(
		tip,
		(LCD_WIDTH - gfx_GetStringWidth(tip)) / 2,
		MENU_TOP_PADDING + CHAR_HEIGHT * 11
	);
}

void draw_board(struct Cell *cells, bool reveal) {
	gfx_FillScreen(BLACK);
	
	int24_t ymin = -yoffset / CELL_WIDTH;
	int24_t ymax = ymin + LCD_HEIGHT / CELL_WIDTH;
	if (ymin < 0) ymin = 0;
	if (ymax > height) ymax = height;
	
	int24_t xmin = -xoffset / CELL_WIDTH;
	int24_t xmax = xmin + LCD_WIDTH / CELL_WIDTH;
	if (xmin < 0) xmin = 0;
	if (xmax > width) xmax = width;

	for (int24_t y = ymin; y < ymax; ++y)
		for (int24_t x = xmin; x < xmax; ++x) {
			struct Cell cell = cells[y * width + x];
			
			int24_t pixel_x = x * CELL_WIDTH + xoffset;
			int24_t pixel_y = y * CELL_WIDTH + yoffset;
			
			if (!cell.open) {
				if (!(reveal && cell.mine))
					draw_unopened_tile(pixel_x, pixel_y);
				else if (cell.flag)
					gfx_TransparentSprite(flag_sprite,
						pixel_x + (CELL_WIDTH - flag_sprite_width) / 2,
						pixel_y + (CELL_WIDTH - flag_sprite_height) / 2
					);
			} else {
				draw_digit(cell.surrounding, pixel_x, pixel_y);
			}
			
			if (reveal && cell.mine) {
				draw_digit(0, pixel_x, pixel_y);
				gfx_TransparentSprite(mine_sprite, 
					pixel_x + (CELL_WIDTH - mine_sprite_width) / 2,
					pixel_y + (CELL_WIDTH - mine_sprite_height) / 2
				);
			}
		}
}

static void draw_unopened_tile(int24_t x, int24_t y) {
	// All you need to know is that the border is 2 px thick, the inside
	// is a square, and that the topright and bottomleft corners are a
	// gradient. Don't try to understand this.
	
	// We're avoiding drawing the two pixels in the topright and bottomleft
	// by making this a pixel thicker.
	gfx_SetColor(LIGHT_GRAY);
	gfx_FillRectangle(x, y, CELL_WIDTH, CELL_WIDTH);
	
	gfx_SetColor(WHITE);
	gfx_FillRectangle(x, y, CELL_WIDTH - 2, 2);
	gfx_FillRectangle(x, y + 2, 2, CELL_WIDTH - 4);
	gfx_SetPixel(x, y + CELL_WIDTH - 2);
	gfx_SetPixel(x + CELL_WIDTH - 2, y);
	
	gfx_SetColor(DARK_GRAY);
	gfx_SetPixel(x + 1, y + CELL_WIDTH - 1);
	gfx_SetPixel(x + CELL_WIDTH - 1, y + 1);
	gfx_FillRectangle(x + 2, y + CELL_WIDTH - 2, CELL_WIDTH - 2, 2);
	gfx_FillRectangle(x + CELL_WIDTH - 2, y + 2, 2, CELL_WIDTH - 4);
}

static void draw_digit(int24_t digit, int24_t x, int24_t y) {
	static const uint8_t digit_sprites[8][DIGIT_SPRITE_HEIGHT] = {
		{0x38, 0x78, 0xF8, 0xD8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xFF, 0xFF},
		{0x7E, 0xFF, 0xC7, 0x83, 0x07, 0x0F, 0x1E, 0x38, 0x70, 0x60, 0xE0, 0xC0, 0xFF, 0xFF},
		{0x7C, 0xFE, 0xC7, 0x03, 0x03, 0x07, 0x3E, 0x3E, 0x07, 0x03, 0x03, 0xC7, 0xFE, 0x7C},
		{0x33, 0x73, 0x63, 0x63, 0xC3, 0xFF, 0x7F, 0x07, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},
		{0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xFC, 0xFE, 0x07, 0x03, 0x03, 0x03, 0x07, 0xFE, 0xFC},
		{0x1F, 0x3F, 0x70, 0x60, 0xC0, 0xC0, 0xFC, 0xFE, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0x7E},
		{0xFF, 0xFF, 0x03, 0x03, 0x07, 0x06, 0x0E, 0x1C, 0x38, 0x30, 0x70, 0x60, 0xE0, 0xC0},
		{0x7E, 0xFF, 0xE7, 0xC3, 0xE7, 0xFF, 0x7E, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0x7E}
	};
	
	gfx_SetColor(DARK_GRAY);
	gfx_Rectangle(x, y, CELL_WIDTH, CELL_WIDTH);
	gfx_SetColor(LIGHT_GRAY);
	gfx_FillRectangle(x + 1, y + 1, CELL_WIDTH - 2, CELL_WIDTH - 2);
			
	if (1 <= digit && digit <= 8) {
		// I manually iterate over each byte, and draw any ON bits.
		// This is a major storage benefit, as instead of using
		// 8(14)(8)=896 bytes, I only need 112.
		// I would opt to use sprites instead if this was required to be
		// done more often.
		
		x += (CELL_WIDTH - DIGIT_SPRITE_WIDTH) / 2;
		y += (CELL_WIDTH - DIGIT_SPRITE_HEIGHT) / 2;
		
		const uint8_t *row = digit_sprites[digit - 1];
		gfx_SetColor(DIGIT_TO_COLOR(digit));
		for (int24_t i = 0; i < DIGIT_SPRITE_HEIGHT; ++i) {
			int24_t temp_x = x;
			int24_t cols = *row;
			while (cols) {
				cols <<= 1;
				if (cols & (1 << 7))
					gfx_SetPixel(temp_x, y + i);
				++temp_x;
			}
			
			++row;
		}
	}
}
