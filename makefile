NAME = MINIMINE
DESCRIPTION = "Minesweeper by superhelix"
ARCHIVED = YES
COMPRESSED = NO
HAS_PRINTF = NO
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

cemu: all
	cemu --send ./bin/$(NAME).8xp