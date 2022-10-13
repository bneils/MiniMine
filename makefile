NAME = MINIMINE
DESCRIPTION = "Minesweeper by superhelix"
ARCHIVED = YES
COMPRESSED = NO
HAS_PRINTF = NO
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

.PHONY: CEmu cemu

CEmu cemu: all
	$@ --send ./bin/$(NAME).8xp
