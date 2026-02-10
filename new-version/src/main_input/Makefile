GXX := g++
GXXFLAGS := -Wall
TARGET := ../../bin/satisfactory_code_input

all: $(TARGET)

debug: GXXFLAGS += -g
debug: all

$(TARGET): *.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)