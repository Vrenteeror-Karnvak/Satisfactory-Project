GXX := g++
GXXFLAGS := -Wall

MAIN := main/satisfactory_main
MAIN_INPUT := main_input/satisfactory_main_input
WELL_INPUT := well_input/satisfactory_main_well_input
WELL_INPUT_TEST := well_input_test/satisfactory_main_well_input_test

TARGETS := $(MAIN) $(MAIN_INPUT) $(WELL_INPUT) $(WELL_INPUT_TEST)

all: $(TARGETS)

main: $(MAIN)
main_input: $(MAIN_INPUT)
well_input: $(WELL_INPUT)
well_input_test: $(WELL_INPUT_TEST)

debug: GXXFLAGS += -g

$(MAIN): main/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(MAIN_INPUT): main_input/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(WELL_INPUT): well_input/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(WELL_INPUT_TEST): well_input_test/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)