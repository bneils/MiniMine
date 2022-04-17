#include "draw.h"
#include "sprites/gfx.h"
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

void set_palette() {
	gfx_SetTransparentColor(TRANSPARENT);
	gfx_SetTextTransparentColor(0);
	gfx_SetPalette(mypalette, sizeof(mypalette), 0);
}

void draw_text_label(char *s, int x, int y) {
	int w = gfx_GetStringWidth(s);
	x -= w / 2;
	gfx_SetTextBGColor(LIGHT_GRAY);
	gfx_SetTextFGColor(WHITE);
	gfx_PrintStringXY(s, x, y);
	gfx_SetColor(BLACK);
	gfx_Rectangle(x, y, w, CHAR_HEIGHT); 
}

void draw_menu(const char *difficulty) {	
	const char *title = "MiniMines by Ben Neilsen";
	const char *tip = "2nd to continue";
	char buf[4];
	
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
	
	sprintf(buf, "%d", width);
	
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
	
	sprintf(buf, "%d", height);
	
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

	sprintf(buf, "%d", mines);
	
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

void draw_board(struct Cell *cells, bool reveal, int clicked_x, int clicked_y, bool partial_redraw) {
	// This is used to optimize the drawing routines.
	// Plus, it reduces some drawing artifacts caused by out-of-buffer
	// graphic calls
	// It took me a while to get the math together on paper, so I would
	// just trust that this works, but you can try to understand it if you
	// want.
	int ymin = -yoffset / CELL_WIDTH;
	int ymax = ymin + LCD_HEIGHT / CELL_WIDTH;
	if (ymin < 0) ymin = 0;
	if (ymax > height) ymax = height;
	
	int xmin = -xoffset / CELL_WIDTH;
	int xmax = xmin + LCD_WIDTH / CELL_WIDTH;
	if (xmin < 0) xmin = 0;
	if (xmax > width) xmax = width;
	
	if (partial_redraw) {
		gfx_BlitScreen();
	} else if (width < LCD_WIDTH / CELL_WIDTH || height < LCD_HEIGHT / CELL_WIDTH) {
		// There's no reason to draw the black background if nothing would show
		gfx_FillScreen(BLACK);
	}
	
	for (int y = ymin; y < ymax; ++y) {
		for (int x = xmin; x < xmax; ++x) {
			struct Cell *cell_ptr = &cells[y * width + x];
			struct Cell cell = *cell_ptr;
			
			if (partial_redraw && !cell.changed) {
				continue;
			}
			
			cell_ptr->changed = 0;
			
			int pixel_x = X_PIXEL(x);
			int pixel_y = Y_PIXEL(y);
			
			if (!cell.open) {
				if (reveal && cell.mine) {
					gfx_SetColor(DARK_GRAY);
					gfx_Rectangle(pixel_x, pixel_y, CELL_WIDTH, CELL_WIDTH);
					gfx_SetColor((clicked_x == x && clicked_y == y) ? RED : LIGHT_GRAY);
					gfx_FillRectangle(pixel_x + 1, pixel_y + 1, CELL_WIDTH - 2, CELL_WIDTH - 2);
					
					gfx_TransparentSprite(mine_sprite,
						pixel_x + (CELL_WIDTH - mine_sprite_width) / 2,
						pixel_y + (CELL_WIDTH - mine_sprite_height) / 2
					);
				} else {
					gfx_TransparentSprite(hidden, pixel_x, pixel_y);
					if (cell.flag) {
						gfx_TransparentSprite(flag_sprite,
							pixel_x + (CELL_WIDTH - flag_sprite_width) / 2,
							pixel_y + (CELL_WIDTH - flag_sprite_height) / 2
						);
					}
				}
			} else {
				gfx_SetColor(LIGHT_GRAY);
				gfx_FillRectangle(pixel_x + 1, pixel_y + 1, CELL_WIDTH - 2, CELL_WIDTH - 2);
				gfx_SetColor(DARK_GRAY);
				gfx_Rectangle(pixel_x, pixel_y, CELL_WIDTH, CELL_WIDTH);
				
				uint8_t num = cell.surrounding - 1;
				if (num >= 0) {
					const gfx_sprite_t *sprites[] = { _1, _2, _3, _4, _5, _6, _7, _8 };
					gfx_TransparentSprite(sprites[num],
						pixel_x + (CELL_WIDTH - DIGIT_SPRITE_WIDTH) / 2,
						pixel_y + (CELL_WIDTH - DIGIT_SPRITE_HEIGHT) / 2
					);
				}
			}
		}
	}

	// Box outline overlay (cursor)
	gfx_SetColor(BLACK);
	gfx_Rectangle(
		X_PIXEL(xcur), 
		Y_PIXEL(ycur), 
		CELL_WIDTH, CELL_WIDTH
	);
	
	// Draw a border if it goes no further
	gfx_SetColor(RED);	
	if (ymin == 0 && 0 <= yoffset && yoffset <= 2) gfx_FillRectangle(0, 0, LCD_WIDTH, 2);
	if (ymax == height && yoffset <= 0) gfx_FillRectangle(0, LCD_HEIGHT - 2, LCD_WIDTH, 2);
	if (xmin == 0 && 0 <= xoffset && xoffset <= 2) gfx_FillRectangle(0, 0, 2, LCD_HEIGHT);
	if (xmax == width && xoffset <= 0) gfx_FillRectangle(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT);
}