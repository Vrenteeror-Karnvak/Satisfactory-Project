GXX := g++
GXXFLAGS := -g

initializer := bin/initializer
recipe_extractor := bin/recipe_extractor
recipe_fixer := bin/recipe_fixer
recipe_sorter_sorter := ./bin/recipe_sorter_sorter
recipe_sorter := bin/recipe_sorter
recipe_trimmer := bin/recipe_trimmer
test_file := bin/test_file
main := bin/main
main_mega := bin/main_mega
test := bin/test
unordered_test := bin/unordered_test
original_test := bin/original_test
obsidian := bin/obsidian

TARGETS := $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_sorter) $(recipe_sorter_sorter) $(recipe_trimmer) $(test_file) $(main) $(main_mega) $(test) $(unordered_test) $(original_test) $(obsidian)

all: recipes test resource obsidian

recipes: $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_trimmer) $(recipe_sorter) $(recipe_sorter_sorter) $(test_file)
test: $(test) $(unordered_test) $(original_test)
resource: $(main) $(main_mega) $(test)
obsidian: $(obsidian)

$(initializer): src/Initializer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_extractor): src/Recipe_Extractor.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_fixer): src/Recipe_Fixer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_sorter_sorter): src/Recipe_Sorter_Sorter.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_sorter): src/Recipe_Sorter.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(recipe_trimmer): src/Recipe_Trimmer.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(test_file): src/Test_File.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(main): src/main/main.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(main_mega): src/main/main_mega.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(test): src/main/test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(unordered_test): src/main/unordered_test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(original_test): src/main/original_test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(obsidian): src/Obsidian.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

clear_obs:
	rm -f bin/dat/satisfactory_vault/*

.PHONY: run_recipe

run_recipe: recipes
	./bin/recipe_extractor
	./bin/recipe_fixer
	./bin/recipe_trimmer
	./bin/recipe_sorter
	./bin/recipe_sorter_sorter

run_test: test
	./bin/test
	./bin/unordered_test
	./bin/original_test

clean:
	rm -f $(TARGETS)