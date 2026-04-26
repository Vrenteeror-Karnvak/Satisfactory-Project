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
backup_test := bin/backup_test
filter_creation := bin/filter_creation
obsidian := bin/obsidian

TARGETS := $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_sorter) $(recipe_sorter_sorter) $(recipe_trimmer) $(test_file) $(main) $(main_mega) $(test) $(backup_test) $(filter_creation) $(obsidian)

all: recipes test resource obsidian

recipes: $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_trimmer) $(recipe_sorter) $(recipe_sorter_sorter) $(test_file)
test: $(test) $(backup_test) $(filter_creation)
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

$(backup_test): src/main/backup_test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(filter_creation): src/main/filter_creation.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(obsidian): src/Obsidian.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

clear_obs:
	rm -f bin/dat/satisfactory_vault/*

.PHONY: run_recipes

run_recipes: recipes
	./bin/recipe_extractor
	./bin/recipe_fixer
	./bin/recipe_trimmer
	./bin/recipe_sorter
	./bin/recipe_sorter_sorter

run_test: test
	./bin/test > ./tests/test.dat
	./bin/backup_test > ./tests/backup_test.dat
	./bin/filter_creation > ./tests/filter_creation.dat

clean:
	rm -f $(TARGETS)