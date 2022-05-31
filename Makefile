TARGET := CandyCrisis
CC     := clang
CFLAGS := -std=c99 -Wall -O3 -IFMOD/api/core/inc
LIBS   := $(shell sdl2-config --libs) -lSDL2_image -LFMOD/api/core/lib -lfmod

SOURCE_DIRS := Source
SOURCES     := $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.c))
OBJECTS     := $(SOURCES:.c=.o)

.PHONY: all debug clean osxbundle

all: $(TARGET)

debug: CFLAGS += -O0 -ggdb
debug: LIBS   += -ggdb
debug: all

osxbundle: CFLAGS += -DOSXBUNDLE
osxbundle: LIBS   += -rpath "../Frameworks"
osxbundle: all
	@echo BUNDLE
	@bundle/makeBundle.rb

$(TARGET): $(OBJECTS)
	@echo LINK $@
	@$(CC) $^ $(LIBS) -o $@

%.o: %.c
	@echo CC   $@
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleanup complete!"
