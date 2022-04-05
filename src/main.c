#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdlib.h>
#include "board.h"
#include "draw.h"

int24_t width, height, mines;
int24_t xoffset = -60, yoffset = 60; // in px

bool step() {
	uint8_t key;
	
	while (!(key = os_GetCSC()))
		;
	
	if (key == sk_Clear) {
		return false;
	}
	
	return true;
}

int main(void) {
	gfx_Begin();
	gfx_SetDrawBuffer();
	set_palette();
	
	srandom(rtc_Time());
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
		if (key == sk_Clear) {
			gfx_End();
			return 0;
		}
		if (key == sk_2nd)
			break;
		
		width = settings[difficulty][0];
		height = settings[difficulty][1];
		mines = settings[difficulty][2];
		
		draw_menu(diffs[difficulty], width, height, mines);
		gfx_SwapDraw();
		
		while (!(key = os_GetCSC()))
			;
	}
	
	struct Cell *cells = malloc(width * height * sizeof(cells));
	cells_place_mines(cells);
	
	gfx_FillScreen(BLACK);
	draw_board(cells, false);
	gfx_SwapDraw();
	
	while (step()) {
		
	}
	
	free(cells);
	gfx_End();
}
