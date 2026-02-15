#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>

#include "recipe.h"
#include "resource.h"

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_recipe.json");
    ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");

    json recipe_root;
    recipe_in >> recipe_root;

    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    Recipe test_recipe(test_recipe_root);

    json terminal_root;
    terminal_recipe_in >> terminal_root;
    Resource terminal_resource;
    vector<Resource> terminal_resources;

    vector<Recipe> recipe_list;
    Recipe recipe_input;

    stack<Recipe> recipe_stack;
    Recipe recipe;
    vector<Resource> ingredients;
    Resource ingredient;
    Recipe new_recipe;
    Recipe terminal_recipe;
    bool recipe_found = false;
    bool is_terminal = false;

    // creates a vector of all terminal resources
    for (const auto& terminal : terminal_root) {
        terminal_resource.set_resource(terminal);
        terminal_resources.push_back(terminal_resource);
    }

    // creates a vector of all recipies
    /* This will be fixed later, as it is only meant to have one recipe for each item */
    for (const auto& category : recipe_root) {
        for (const auto& recipe : category["Data"]) {
            recipe_input.set_recipe(recipe);
            recipe_list.push_back(recipe_input);
        }
    }

    recipe_stack.push(test_recipe);
    while (!recipe_stack.empty()) {
        if (recipe_stack.top().is_processed()) {
            // if the current recipe has already been processed, remove it from the stack
            cout << recipe_stack.top().get_product(0).get_name() << endl;
            recipe_stack.pop();
            continue;
        }
        else {
            // if the current recipe has not been processed, process it
            recipe_stack.top().set_processed(); // sets the processed flag to true
            ingredients = recipe_stack.top().get_ingredients(); // gets the ingredients of the current recipe
            for (int i = 0; i < ingredients.size(); i++) { // increments through all the ingredients
                is_terminal = false;
                recipe_found = false;

                for (int j = 0; j < terminal_resources.size(); j++) { // increments through all the terminal resources
                    if (ingredients.at(i) == terminal_resources.at(j)) { // checks if the ingredient is a terminal resource
                        is_terminal = true;
                    }
                }

                if (is_terminal) { // if the ingredient was terminal, moves on to the next one
                    terminal_recipe.set_terminal_recipe(ingredients.at(i));
                    cout << terminal_recipe.get_name() << endl;
                    continue;
                }

                for (int j = 0; j < recipe_list.size(); j++) { // increments through all the recipes in recipe_list
                    if (ingredients.at(i) == recipe_list.at(j).get_product(0)) {
                        // true if the main product of the recipe is the current ingredient
                        new_recipe = recipe_list.at(j); // sets new_recipe to the recipe found
                        recipe_found = true;
                        break;
                    }
                }

                if (recipe_found) {
                    recipe_stack.push(new_recipe); // adds the new recipe to the stack
                }
                else {
                    // if now recipe was found, outputs the fact as there may be missing data somewhere
                    // the program otherwise continues as if the resource was terminal
                    cout << "No recipe found for " << ingredients.at(i).get_name() << endl;
                }
            }
        }
    }

    recipe_in.close();
    test_recipe_in.close();

    cout << "Everything is in working order here." << endl;
}