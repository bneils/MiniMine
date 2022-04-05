# ----------------------------
# Makefile Options
# ----------------------------

NAME = MINIMINE
DESCRIPTION = "Minesweeper by Ben Neilsen"
ARCHIVED = YES
COMPRESSED = YES
HAS_PRINTF = NO
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

cemu: all
	cemu --send ./bin/MINIMINE.8xp
