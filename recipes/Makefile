GXX := g++
GXXFLAGS := -g

initializer := bin/initializer
recipe_extractor := bin/recipe_extractor
recipe_fixer := bin/recipe_fixer
recipe_sorter := bin/recipe_sorter
recipe_trimmer := bin/recipe_trimmer
test_file := bin/test_file
resource := bin/resource

TARGETS := $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_trimmer) $(test_file) $(resource)

all: recipes test resource

recipes: $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_trimmer) $(recipe_sorter)
test: $(test_file)
resource: $(resource)

$(initializer): src/Initializer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_extractor): src/Recipe_Extractor.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_fixer): src/Recipe_Fixer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_sorter): src/Recipe_Sorter.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_trimmer): src/Recipe_Trimmer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(test_file): src/Test_File.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(resource): src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

.PHONY: run_recipe

run_recipe: recipes
	./bin/recipe_extractor
	./bin/recipe_fixer
	./bin/recipe_trimmer
	./bin/recipe_sorter

clean:
	rm -f $(TARGETS)