NAME = MINIMINE
DESCRIPTION = "Minesweeper by superhelix"
ARCHIVED = YES
COMPRESSED = NO
HAS_PRINTF = NO
CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

# The AUR package maintainer installs it as "CEmu"
CEMU := $(shell command -v cemu 2> /dev/null)
ifndef CEMU
	CEMU = CEmu
endif

cemu: all
	$(CEMU) --send ./bin/$(NAME).8xp
