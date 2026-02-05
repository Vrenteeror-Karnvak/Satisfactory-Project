GXX = g++

GXXFLAGS = -Wall -g

/ := /

SOURCES = $(wildcard src$(/)*.cpp)
TARGETS = $(addsuffix .exe,$(addprefix bin$(/),$(notdir $(basename $(SOURCES)))))

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(GXX) $(GXXFLAGS) src$(/)$(notdir $(basename $@)).cpp -o $@

clean:
	rm -f $(TARGETS)