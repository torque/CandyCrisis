TARGET := CandyCrisis
CXX    := clang++
CFLAGS := -Wall -O3
LIBS   := $(shell sdl-config --libs) -lSDL_image -lfmodex

SOURCE_DIRS := Source
SOURCES     := $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.cpp))
OBJECTS     := $(SOURCES:.cpp=.o)

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
	@$(CXX) $^ $(LIBS) -o $@

%.o: %.cpp
	@echo CXX $@
	@$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleanup complete!"
