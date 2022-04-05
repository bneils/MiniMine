#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdlib.h>
#include "board.h"
#include "draw.h"

// I don't want to pass a million parameters to each function.
// This also lets me avoid having some config struct that I have to
// constantly dereference. Also, I hate having to change function prototypes.
int24_t width, height, mines;
int24_t xcur, ycur; // in cells
int24_t xoffset, yoffset; // in px

int menu_screen(void);
void floodfill_open(struct Cell *);
int game(struct Cell *);

/* 0 on success */
int menu_screen(void) {
	uint8_t key = 0;
	uint8_t difficulty = 0;
	const char *diffs[] = {
		"Easy>",
		"<Medium>",
		"<Hard"
	};
	
	uint8_t settings[][3] = {
		{9, 9, 10},
		{16, 16, 40},
		{30, 16, 99}
	};
	
	// Menu screen
	for (;;) {
		if (key == sk_Left && difficulty > 0)
			--difficulty;
		if (key == sk_Right && difficulty < sizeof(diffs) / sizeof(diffs[0]) - 1)
			++difficulty;
		if (key == sk_Clear)
			return 1;
		if (key == sk_2nd)
			return 0;
		
		width = settings[difficulty][0];
		height = settings[difficulty][1];
		mines = settings[difficulty][2];
		
		draw_menu(diffs[difficulty]);
		gfx_SwapDraw();
		
		while (!(key = os_GetCSC()))
			;
	}
}

/* performs a zero flood fill using scan lines */
void floodfill_open(struct Cell *cells) {
	bool spread = true;
	cells[ycur * width + xcur].open = 1;
	while (spread) {
		spread = false;
		for (int24_t y = 0; y < height; ++y)
			for (int24_t x = 0; x < width; ++x) {
				struct Cell *cell;
				cell = &cells[y * width + x];
				if (cell->surrounding == 0 && cell->open)
					for (int24_t i = y - 1; i <= y + 1; ++i)
						for (int24_t j = x - 1; j <= x + 1; ++j) {
							cell = &cells[i * width + j];
							if (i >= 0 && j >= 0 && i < height && j < width && !cell->open) {
								cell->open = 1;
								spread = true;
							}
						}
			}
	}
}

/* 0 on continue, 1 on exit */
int game(struct Cell *cells) {
	bool reveal = false;
	
	for (;;) {
		draw_board(cells, reveal);
		gfx_SwapDraw();
		
		uint8_t key;
		struct Cell *cell = &cells[ycur * width + xcur];
		
		while (!(key = os_GetCSC()))
			;
		
		if (reveal)
			return 0;
		
		switch (key) {
			case sk_Clear:
				return 1;
			case sk_Left:
				if (xcur > 0)
					--xcur;
				break;
			case sk_Right:
				if (xcur < width - 1)
					++xcur;
				break;
			case sk_Up:
				if (ycur > 0)
					--ycur;
				break;
			case sk_Down:
				if (ycur < height - 1)
					++ycur;
				break;
			case sk_2nd:
				if (!cell->flag) {
					if (cell->mine) // Death
						reveal = true;
					else if (cell->open) {
						// Opening a 3x3 when flags are satisifed
						uint8_t surrounding_flags = 0;
						for (int24_t i = ycur - 1; i <= ycur + 1; ++i)
							for (int24_t j = xcur - 1; j <= xcur + 1; ++j)
								if (i >= 0 && j >= 0 && i < height && j < width && cells[i * width + j].flag)
									++surrounding_flags;
						if (surrounding_flags == cell->surrounding) {
							for (int24_t i = ycur - 1; i <= ycur + 1; ++i)
								for (int24_t j = xcur - 1; j <= xcur + 1; ++j)
									if (i >= 0 && j >= 0 && i < height && j < width && !cells[i * width + j].flag)
										cells[i * width + j].open = 1;
						}
					} else
						floodfill_open(cells); // Normal open
				}
				break;
			case sk_Alpha:
				if (!cell->open)
					cell->flag = !cell->flag;
				break;
		}
	}
	
	return 0;
}

int main(void) {
	gfx_Begin();
	gfx_SetDrawBuffer();
	set_palette();
	
	srandom(rtc_Time());
	
	struct Cell *cells = NULL;
	
	for (;;) {
		if (menu_screen())
			break;
		free(cells);
		cells = malloc(width * height * sizeof(*cells));	
		if (!cells)
			break;
		cells_place_mines(cells);
	
		if (game(cells))
			break;
	}
	
	free(cells);
	gfx_End();
}
