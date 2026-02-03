GXX = g++

GXXFLAGS = -Wall -g

SOURCES = $(wildcard src/*.cpp)
#linux
#TARGETS = $(addprefix bin/,$(notdir $(basename $(SOURCES))))
#windows
TARGETS = $(addsuffix .exe,$(addprefix bin/,$(notdir $(basename $(SOURCES)))))

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(GXX) $(GXXFLAGS) src/$(notdir $(basename $@)).cpp -o $@

clean:
	rm -f $(TARGETS)