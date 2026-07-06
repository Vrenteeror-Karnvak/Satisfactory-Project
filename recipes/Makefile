GXX := g++
GXXFLAGS := -Wall -std=c++17

initializer := bin/initializer
recipe_extractor := bin/recipe_extractor
recipe_fixer := bin/recipe_fixer
recipe_sorter_sorter := ./bin/recipe_sorter_sorter
recipe_sorter := bin/recipe_sorter
recipe_trimmer := bin/recipe_trimmer
test_file := bin/test_file
main := bin/main
test := bin/test
ficsonium_test := bin/ficsonium_test
backup_test := bin/backup_test
obsidian := bin/obsidian

# Archived:
main_mega := bin/main_mega
filter_creation := bin/filter_creation
equivalent_remover := bin/equivalent_remover
test_compressed := bin/test_compressed
test_base := bin/test_base

TARGETS := $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_sorter) $(recipe_sorter_sorter) $(recipe_trimmer) $(test_file) $(main) $(test) $(ficsonium_test) $(backup_test) $(obsidian)
ARCHIVED_TARGETS := $(main_mega) $(test_compressed) $(test_base) $(filter_creation) $(equivalent_remover)

all: recipes test resource obsidian

debug: GXXFLAGS += -g
debug: all

recipes: $(initializer) $(recipe_extractor) $(recipe_fixer) $(recipe_trimmer) $(recipe_sorter) $(recipe_sorter_sorter) $(test_file)
test: ensure_critical_files $(test) $(backup_test) $(ficsonium_test)
resource: ensure_critical_files $(main) $(test)
obsidian: $(obsidian)
archived: $(ARCHIVED_TARGETS)

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

$(test): src/main/test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(ficsonium_test): src/main/ficsonium_test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(backup_test): src/main/backup_test.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(obsidian): src/Obsidian.cpp
	$(GXX) $(GXXFLAGS) $^ -o $@

$(main_mega): src/Archive/Archived_main_mega.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(test_compressed): src/Archive/Archived_test_compressed.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(test_base): src/Archive/Archived_test_base.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(filter_creation): src/Archive/Archived_filter_creation.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

$(equivalent_remover): src/Archive/Archived_equivalent_remover.cc src/resources/*.cc
	$(GXX) $(GXXFLAGS) $^ -o $@

clear_obs:
	rm -f bin/dat/satisfactory_vault/*

bin/dat/en-US.json:
	@echo "\e[31m\
	dat/en-US.json not found\nExamine Steam/steamapps/common/Satisfactory/CommunityResources/Languages/Translations\
	\e[0m"

ensure_critical_files:
	@if [ ! -f bin/dat/test_input.json ]; then cp debug/dat/test_input.json bin/dat/; fi
	@if [ ! -f bin/dat/item_filters.json ]; then cp debug/dat/item_filters.json bin/dat/; fi

run_recipes: recipes bin/dat/en-US.json
	./bin/recipe_extractor
	./bin/recipe_fixer
	./bin/recipe_trimmer
	./bin/recipe_sorter
	./bin/recipe_sorter_sorter

run_test: test ensure_critical_files
	./bin/test > ./bin/tests/test.dat
#	./bin/backup_test > ./bin/tests/backup_test.dat
	./bin/filter_creation > ./bin/tests/filter_creation.dat

clean:
	@if [ -f bin/dat/test_input.json ]; then cp bin/dat/test_input.json debug/dat/; fi
	@if [ -f bin/dat/item_filters.json ]; then cp bin/dat/item_filters.json debug/dat/; fi
	rm -f $(TARGETS)
	rm -f $(ARCHIVED_TARGETS)

uninstall: clean
	rm -rf bin/*
	mkdir bin/dat bin/int bin/tests
	@if [ -f debug/dat/test_input.json ]; then cp debug/dat/test_input.json bin/dat/; fi
	@if [ -f debug/dat/item_filters.json ]; then cp debug/dat/item_filters.json bin/dat/; fi
