GXX := g++
GXXFLAGS := -Wall


all: satisfactory_code

debug: GXXFLAGS += -g
debug: all

satisfactory_code: *.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clean:
	rm -f satisfactory_code