GXX = g++

GXXFLAGS = -Wall -g

SOURCES = $(wildcard src/*.cpp)
TARGETS = $(addprefix bin/,$(notdir $(basename $(SOURCES))))

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(GXX) $(GXXFLAGS) src/$(notdir $(basename $@)).cpp -o $@

clean:
	rm $(TARGETS)