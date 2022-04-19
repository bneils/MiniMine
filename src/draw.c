#include "draw.h"
#include "sprites/gfx.h"
#include "board.h"
#include <graphx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <tice.h>

/* display a centered label in the pause menu */
void draw_centered_text(char *s, int y, enum Color color) {
	gfx_SetTextFGColor(color);
	gfx_SetTextBGColor(TRANSPARENT);
	gfx_PrintStringXY(s, 
		(LCD_WIDTH - gfx_GetStringWidth(s)) / 2, y
	);
}

/* display a key/value pair in the pause menu */
void draw_pause_screen_key_value(char *s, uint8_t num, int y) {
	char byte_buf[4];
	gfx_SetTextFGColor(BLACK);
	gfx_SetTextBGColor(TRANSPARENT);
	sprintf(byte_buf, "%d", num);
	gfx_PrintStringXY(s, PAUSE_LABEL_TEXT_X, y);
	gfx_PrintStringXY(byte_buf,
		PAUSE_LABEL_TEXT_X + CHAR_HEIGHT + gfx_GetStringWidth(s),
		y
	);
}

void draw_pause_screen(void) {
	gfx_SetColor(WHITE);
	gfx_Rectangle(
		PAUSE_LABEL_OUTER_X, PAUSE_LABEL_OUTER_Y,
		PAUSE_LABEL_WIDTH, PAUSE_LABEL_HEIGHT
	);
	gfx_Rectangle(
		PAUSE_LABEL_OUTER_X + 2, PAUSE_LABEL_OUTER_Y + 2,
		PAUSE_LABEL_WIDTH - 4, PAUSE_LABEL_HEIGHT - 4
	);
	
	gfx_SetColor(BLACK);
	gfx_Rectangle(
		PAUSE_LABEL_OUTER_X + 1, PAUSE_LABEL_OUTER_Y + 1,
		PAUSE_LABEL_WIDTH - 2, PAUSE_LABEL_HEIGHT - 2
	);
	
	gfx_SetColor(LIGHT_GRAY);
	gfx_FillRectangle(
		PAUSE_LABEL_INNER_X, PAUSE_LABEL_INNER_Y, 
		PAUSE_LABEL_WIDTH - 2 * PAUSE_LABEL_BORDER_WIDTH, PAUSE_LABEL_HEIGHT - 2 * PAUSE_LABEL_BORDER_WIDTH
	);
}

void set_palette(void) {
	gfx_SetTransparentColor(TRANSPARENT);
	gfx_SetTextTransparentColor(0);
	gfx_SetPalette(mypalette, sizeof(mypalette), 0);
}

void draw_menu(const char *difficulty) {	
	char buf[4];
	
	gfx_FillScreen(BLACK);
	draw_centered_text("MiniMines by superhelix", MENU_TOP_PADDING, BLUE);

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
	
	draw_centered_text("2nd to interact (labels & unopened)", MENU_TOP_PADDING + CHAR_HEIGHT * 11, WHITE);
	draw_centered_text("Alpha to flag", MENU_TOP_PADDING + CHAR_HEIGHT * 12, WHITE);
	draw_centered_text("Arrow keys to move", MENU_TOP_PADDING + CHAR_HEIGHT * 13, WHITE);
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
				
				if (cell.surrounding >= 1) {
					const gfx_sprite_t *sprites[] = { _1, _2, _3, _4, _5, _6, _7, _8 };
					gfx_TransparentSprite(sprites[cell.surrounding - 1],
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