CC           := cc
UNAME        := $(shell uname -s)

OBJDIR       := build
OUTPUT       := $(OBJDIR)/CandyCrisis

OPTIMIZATION := -Os
WARNINGS     := -Wall
INCLUDES     := -IFMOD/api/core/inc
DEFS         :=

CFLAGS        = -std=c99 $(WARNINGS) $(INCLUDES) $(DEFS) $(OPTIMIZATION)
LDFLAGS       = $(shell sdl2-config --libs) -lSDL2_image -LFMOD/api/core/lib -lfmod

SOURCE_FILES  := blitter.c control.c fmodmusic.c fmodsoundfx.c font.c gameticks.c
SOURCE_FILES  += graphics.c graymonitor.c grays.c gworld.c hiscore.c keyselect.c
SOURCE_FILES  += level.c main.c moving.c next.c opponent.c pause.c players.c prefs.c
SOURCE_FILES  += random.c score.c SDLU.c tutorial.c tweak.c victory.c zap.c

MACOS_BUNDLE_OUTPUT = "$(OBJDIR)/Candy Crisis"
MACOS_BUNDLE_LIB_SEARCH_DIRS = FMOD/api/core/lib

SOURCES       := $(addprefix Source/,$(SOURCE_FILES))
OBJECTS       := $(addprefix $(OBJDIR)/,$(SOURCES:.c=.o))

.PHONY: all debug test clean
ifndef VERBOSE
.SILENT:
endif

all: $(OUTPUT)

debug: CFLAGS += -g -O0 -fsanitize=address -fno-omit-frame-pointer
debug: LDFLAGS += -g -fsanitize=address
debug: $(OUTPUT)

osxbundle: DEFS += -DOSXBUNDLE
osxbundle: LDFLAGS += -rpath "../Frameworks"
osxbundle: $(OUTPUT)
	@printf "\e[1;35mBUNDLE\e[m $@\n"
	bundle/makeBundle.rb -e "$(OUTPUT)" $(addprefix -l ,$(MACOS_BUNDLE_LIB_SEARCH_DIRS)) -o $(MACOS_BUNDLE_OUTPUT)

$(OUTPUT): $(OBJECTS)
	@printf "\e[1;32m  LINK\e[m $@\n"
	$(CC) $^ $(LDFLAGS) -o $@

$(OBJECTS): $(DEPS) | $(OBJDIR)/Source/

$(OBJDIR)/%.d:
	@true

$(OBJDIR)/%.o: %.c
	@printf "\e[1;34m    CC\e[m $<\n"
	$(CC) $(CFLAGS) -MMD -MF $(@:.o=.d) -c $< -o $@

$(OBJDIR):
	@printf "\e[1;33m MKDIR\e[m $@\n"
	mkdir -p $@

$(OBJDIR)/%/: | $(OBJDIR)
	@printf "\e[1;33m MKDIR\e[m $@\n"
	mkdir -p $@

clean:
	@printf "\e[1;31m    RM\e[m $(OBJDIR)\n"
	rm -rf $(OBJDIR)

-include $(OBJECTS:.o=.d) $(TEST_OBJECTS:.o=.d)
