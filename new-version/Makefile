GXX := g++
GXXFLAGS := -Wall

MAIN := bin/satisfactory_main
MAIN_INPUT := bin/satisfactory_main_input
WELL_INPUT := bin/satisfactory_main_well_input
WELL_INPUT_TEST := bin/satisfactory_main_well_input_test

TARGETS := $(MAIN) $(MAIN_INPUT) $(WELL_INPUT) $(WELL_INPUT_TEST)

all: $(TARGETS)

main: $(MAIN)
main_input: $(MAIN_INPUT)
well_input: $(WELL_INPUT)
well_input_test: $(WELL_INPUT_TEST)

debug: GXXFLAGS += -g

$(MAIN): src/main/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(MAIN_INPUT): src/main_input/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(WELL_INPUT): src/well_input/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(WELL_INPUT_TEST): src/well_input_test/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)