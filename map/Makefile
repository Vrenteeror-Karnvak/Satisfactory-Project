GXX = g++

GXXFLAGS = -Wall -g

Targets = $(wildcard src/*.cpp)

Names = $(notdir $(basename $(Targets)))

all: $(Names)

$(Names): $(Targets)
	$(GXX) $(GXXFLAGS) src/$@.cpp -o bin/$@

clean:
	rm $(addprefix bin/,$(Names))