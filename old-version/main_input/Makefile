GXX := g++
GXXFLAGS := -Wall


all: satisfactory_code_input

debug: GXXFLAGS += -g
debug: all

satisfactory_code_input: *.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f satisfactory_code_input