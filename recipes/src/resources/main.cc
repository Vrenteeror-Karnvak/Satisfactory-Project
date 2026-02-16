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

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_recipe.json");
    ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
    ofstream results(exePath / "dat" / "results.json");

    // The json file containing all recipes
    json recipe_root;
    recipe_in >> recipe_root;
    Recipe recipe_input;
    vector<Recipe> recipe_list;

    // The json file containing the test recipe
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    Recipe test_recipe(test_recipe_root);
    

    // The json file containing the terminal resources
    json terminal_root;
    terminal_recipe_in >> terminal_root;
    Resource terminal_resource;
    vector<Resource> terminal_resources;

    // The variables that the stack uses to increment through all nodes
    stack<Recipe> recipe_stack; // the stack of recipes in the chain
    vector<Resource> ingredients; // the ingredients being added
    Resource ingredient; // the current ingredient being added
    Recipe new_recipe; // the recipe being added to the stack for non terminal resources
    Recipe terminal_recipe; // the recipe being added to the stack for terminal resources
    bool recipe_found = false; // marks if a resource has a recipe
    bool is_terminal = false; // marks if a resource is terminal

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain
    int location = 0; // the location of the identical recipe in the vector
    bool already_added = false; // marks if a recipe is already in the vector

    // The variables used to output the data
    json output_object = json::object();
    json output_array = json::array();

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
            already_added = false;
            // if the current recipe has already been processed, remove it from the stack
            for (int i = 0; i < output_recipes.size(); i++) {
                if (output_recipes.at(i) == recipe_stack.top()) {
                    already_added = true;
                    location = i;
                    break;
                }
            }
            if (already_added) {
                output_recipes.at(location).combine_recipes(recipe_stack.top());
            }
            else {
                output_recipes.push_back(recipe_stack.top());
            }
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
                    recipe_stack.push(terminal_recipe);
                    continue;
                }

                for (int j = 0; j < recipe_list.size(); j++) { // increments through all the recipes in recipe_list
                    if (ingredients.at(i) == recipe_list.at(j).get_product(0)) {
                        // true if the main product of the recipe is the current ingredient
                        new_recipe = recipe_list.at(j); // sets new_recipe to the recipe found
                        new_recipe.set_to(ingredients.at(i).get_amount());
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
                    cout << "No recipe found for " << ingredients.at(i).get_name() << "." << endl;
                }
            }
        }
    }

    for (int i = 0; i < output_recipes.size(); i++) {
        output_object = output_recipes.at(i).to_json();
        output_array.push_back(output_object);
    }

    results << output_array.dump(4);

    recipe_in.close();
    test_recipe_in.close();
    results.close();

    cout << "Everything is in working order here." << endl;
}