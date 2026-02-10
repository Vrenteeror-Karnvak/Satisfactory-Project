GXX := g++
GXXFLAGS := -Wall
TARGET := ../../bin/satisfactory_code_well_input_test

all: $(TARGET)

debug: GXXFLAGS += -g
debug: all

$(TARGET): *.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)