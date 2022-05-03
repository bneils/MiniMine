#include "draw.h"
#include "sprites/gfx.h"
#include "board.h"
#include <graphx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <tice.h>
#include <string.h>
#include <stdio.h>

#define PANEL_ROW_PIXEL(row) (PANEL_INNER_Y + PANEL_TEXT_MARGIN + (row) * (CHAR_HEIGHT + 2))

/* draws a canvas for text to be put on */
void draw_panel_canvas(void) {
	gfx_SetColor(WHITE);
	gfx_Rectangle(
		PANEL_OUTER_X, PANEL_OUTER_Y,
		PANEL_WIDTH, PANEL_HEIGHT
	);
	gfx_Rectangle(
		PANEL_OUTER_X + 2, PANEL_OUTER_Y + 2,
		PANEL_WIDTH - 4, PANEL_HEIGHT - 4
	);
	
	gfx_SetColor(BLACK);
	gfx_Rectangle(
		PANEL_OUTER_X + 1, PANEL_OUTER_Y + 1,
		PANEL_WIDTH - 2, PANEL_HEIGHT - 2
	);
	
	gfx_SetColor(LIGHT_GRAY);
	gfx_FillRectangle(
		PANEL_INNER_X, PANEL_INNER_Y, 
		PANEL_WIDTH - 2 * PANEL_THICKNESS, PANEL_HEIGHT - 2 * PANEL_THICKNESS
	);
}

/* draws some aligned text on the canvas.
 * textfgcolor must be set before called.
 */
void draw_panel_text(char *s, int row, enum Alignment align) {
	gfx_SetTextBGColor(TRANSPARENT);

	int x, y, pix_len;
	pix_len = gfx_GetStringWidth(s);

	switch (align) {
		case LEFT:
			x = PANEL_INNER_X + PANEL_TEXT_MARGIN;
			break;
		case RIGHT:
			x = PANEL_OUTER_X - PANEL_THICKNESS - PANEL_TEXT_MARGIN - pix_len;
			break;
		case CENTER:
		default:
			x = (LCD_WIDTH - pix_len) / 2;
			break;
	}

	y = PANEL_ROW_PIXEL(row);
	gfx_PrintStringXY(s, x, y);
}

/* a little arrow that is left of the left alignment */
void draw_panel_selection(int row) {
	gfx_SetTextBGColor(TRANSPARENT);
	gfx_PrintStringXY(">", PANEL_INNER_X + PANEL_TEXT_MARGIN - gfx_GetStringWidth(">"), PANEL_ROW_PIXEL(row));
}

void set_palette(void) {
	gfx_SetTransparentColor(TRANSPARENT);
	gfx_SetTextTransparentColor(0);
	gfx_SetPalette(mypalette, sizeof(mypalette), 0);
}

void draw_menu(enum Difficulty difficulty) {	
	char buf[10];

	gfx_FillScreen(BLACK);
	draw_panel_canvas();
	gfx_SetTextFGColor(BLUE);
	draw_panel_text("MiniMines by superhelix", 0, CENTER);
	
	gfx_SetTextFGColor(GREEN);
	draw_panel_text("Easy", 2, LEFT);
	gfx_SetTextFGColor(YELLOW);
	draw_panel_text("Medium", 3, LEFT);
	gfx_SetTextFGColor(RED);
	draw_panel_text("Hard", 4, LEFT);
	
	gfx_SetTextFGColor(BLACK);
	// depends on the difficulty enum being in ascending order
	draw_panel_selection(2 + difficulty);
	
	sprintf(buf, "%dx%d", width, height);
	draw_panel_text(buf, 1, LEFT);
	sprintf(buf, "%d", mines);
	draw_panel_text(buf, 1, RIGHT);

	draw_panel_text("2nd to interact", 6, LEFT);
	draw_panel_text("Arrow keys to move", 7, LEFT);
	draw_panel_text("Alpha to flag", 8, LEFT);
	draw_panel_text("Mode to pause", 9, LEFT);
}

void draw_board(struct Cell *cells, bool reveal, struct Vec2D clicked, bool partial_redraw) {
	// This is used to optimize the drawing routines.
	// Plus, it reduces some drawing artifacts caused by out-of-buffer
	// graphic calls
	// It took me a while to get the math together on paper, so I would
	// just trust that this works, but you can try to understand it if you
	// want.
	struct Vec2D min, max;
	
	min.y = -offset.y / CELL_WIDTH;
	max.y = min.y + LCD_HEIGHT / CELL_WIDTH;
	if (min.y < 0) min.y = 0;
	if (max.y > height) max.y = height;
	
	min.x = -offset.x / CELL_WIDTH;
	max.x = min.x + LCD_WIDTH / CELL_WIDTH;
	if (min.x < 0) min.x = 0;
	if (max.x > width) max.x = width;
	
	if (partial_redraw) {
		gfx_BlitScreen();
	} else if (width < LCD_WIDTH / CELL_WIDTH || height < LCD_HEIGHT / CELL_WIDTH) {
		// There's no reason to draw the black background if nothing would show
		gfx_FillScreen(BLACK);
	}
	
	for (int y = min.y; y < max.y; ++y) {
		for (int x = min.x; x < max.x; ++x) {
			struct Cell *cell_ptr = &cells[y * width + x];
			struct Cell cell = *cell_ptr;
			
			if (partial_redraw && !cell.changed) {
				continue;
			}
			
			cell_ptr->changed = false;
			
			struct Vec2D pixel = {
				.x = X_PIXEL(x),
				.y = Y_PIXEL(y)
			};
			
			if (!cell.open) {
				if (reveal && cell.mine) {
					gfx_SetColor(DARK_GRAY);
					gfx_Rectangle(pixel.x, pixel.y, CELL_WIDTH, CELL_WIDTH);
					gfx_SetColor((clicked.x == x && clicked.y == y) ? RED : LIGHT_GRAY);
					gfx_FillRectangle(pixel.x + 1, pixel.y + 1, CELL_WIDTH - 2, CELL_WIDTH - 2);
					
					gfx_TransparentSprite(mine_sprite,
						pixel.x + (CELL_WIDTH - mine_sprite_width) / 2,
						pixel.y + (CELL_WIDTH - mine_sprite_height) / 2
					);
				} else {
					gfx_TransparentSprite(hidden, pixel.x, pixel.y);
					if (cell.flag) {
						gfx_TransparentSprite(flag_sprite,
							pixel.x + (CELL_WIDTH - flag_sprite_width) / 2,
							pixel.y + (CELL_WIDTH - flag_sprite_height) / 2
						);
					}
				}
			} else {
				gfx_SetColor(LIGHT_GRAY);
				gfx_FillRectangle(pixel.x + 1, pixel.y + 1, CELL_WIDTH - 2, CELL_WIDTH - 2);
				gfx_SetColor(DARK_GRAY);
				gfx_Rectangle(pixel.x, pixel.y, CELL_WIDTH, CELL_WIDTH);
				
				if (1 <= cell.surrounding && cell.surrounding <= 8) {
					const gfx_sprite_t *sprites[] = { _1, _2, _3, _4, _5, _6, _7, _8 };
					gfx_TransparentSprite(sprites[cell.surrounding - 1],
						pixel.x + (CELL_WIDTH - DIGIT_SPRITE_WIDTH) / 2,
						pixel.y + (CELL_WIDTH - DIGIT_SPRITE_HEIGHT) / 2
					);
				}
			}
		}
	}

	// Box outline overlay (cursor)
	gfx_SetColor(BLACK);
	gfx_Rectangle(
		X_PIXEL(cur.x),
		Y_PIXEL(cur.y), 
		CELL_WIDTH, CELL_WIDTH
	);
	
	// Draw a border if it goes no further
	gfx_SetColor(RED);	
	if (min.y == 0 && 0 <= offset.y && offset.y <= 2) gfx_FillRectangle(0, 0, LCD_WIDTH, 2);
	if (max.y == height && offset.y <= 0) gfx_FillRectangle(0, LCD_HEIGHT - 2, LCD_WIDTH, 2);
	if (min.x == 0 && 0 <= offset.x && offset.x <= 2) gfx_FillRectangle(0, 0, 2, LCD_HEIGHT);
	if (max.x == width && offset.x <= 0) gfx_FillRectangle(LCD_WIDTH - 2, 0, 2, LCD_HEIGHT);
}
