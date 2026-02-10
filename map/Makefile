GXX = g++
GXXFLAGS = -g

main := bin/satisfactory_code
main_input := bin/satisfactory_code_input
well_input := bin/satisfactory_code_well_input
well_test := bin/satisfactory_code_well_input_test

TARGETS := $(main) $(main_input) $(well_input) $(well_test)

all: main node_input well_input well_test

main: $(main)
node_input: $(main_input)
well_input: $(well_input)
well_test: $(well_test)


$(main): src/Satisfactory_Code.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(main_input): src/Satisfactory_Node_Input.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(well_input): src/Satisfactory_Well_Input.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(well_test): src/Satisfactory_Well_Input_Test.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f $(TARGETS)