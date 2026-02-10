GXX := g++
GXXFLAGS := -Wall


all: satisfactory_code_well_input_test

debug: GXXFLAGS += -g
debug: all

satisfactory_code_well_input_test: *.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f satisfactory_code_well_input_test