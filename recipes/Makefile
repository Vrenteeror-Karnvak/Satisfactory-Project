GXX = g++

GXXFLAGS = -Wall -g

SOURCES := $(wildcard src/*.cpp)
TARGETS := $(addprefix bin/,$(notdir $(basename $(SOURCES))))

TARGET_A := bin/Resource
SOURCE_A := $(wildcard src/resources/*.cc)

all: recipes resources

recipes: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(GXX) $(GXXFLAGS) src/$(notdir $(basename $@)).cpp -o $@

resources: $(TARGET_A)

$(TARGET_A): $(SOURCE_A) 
	$(GXX) $(GXXFLAGS) $^ -o $@ 

clean:
	rm $(TARGETS) $(TARGET_A)