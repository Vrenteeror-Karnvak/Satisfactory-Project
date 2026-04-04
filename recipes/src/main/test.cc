#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include <chrono>

#include "../resources/recipe.h"
#include "../resources/resource.h"
#include "../resources/fraction.h"

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    bool loop_termination = false; // Triggers if the loop runs a set number of times
    bool time_termination = false; // Triggers if the loops runs a set amount of time

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_recipe.json");
    ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
    ofstream results(exePath / "dat" / "test_results.json");

    // The json file containing all recipes as well as the variables needed to increment through them
    json recipe_root;
    recipe_in >> recipe_root;
    Recipe recipe_input;
    map<string, Recipe> recipe_map;
    int vector_size;
    vector<int> incrementor;
    vector<int> incrementor_max;
    vector<int> all_zeros;
    map<string, int> incrementor_map; // the location of the incrementor for a given product inside of the incrementor vector
    int m = 0;

    // The json file containing the recipe or item
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    Recipe test_recipe(test_recipe_root.at(0)); // Use to inject a RECIPE into the system
    string test_item = test_recipe_root.at(1).value("ItemClass", ""); // Use to inject an ITEM into the system
    string current_item; // the item being worked with

    // The auto terminate information
    const int max_loops = stoi(test_recipe_root.at(2).value("max_loops", "")); // the maximum number of loops the program is allowed to run
    const chrono::minutes max_time(stoi(test_recipe_root.at(2).value("max_time", ""))); // the max time the program is allowed to run
    const chrono::seconds update_frequency(stoi(test_recipe_root.at(3).value("update_frequency", ""))); // the frequency the program updates its progress
    int k = 1; // the number of updates

    // The json file containing the terminal resources
    json terminal_root;
    terminal_recipe_in >> terminal_root;
    Resource terminal_resource;
    map<string, Resource> terminal_map;

    // The variables that the stack uses to increment through all nodes
    stack<Recipe> recipe_stack; // the stack of recipes in the chain
    vector<Resource> ingredients; // the ingredients being added
    Resource ingredient; // the current ingredient being added
    Recipe new_recipe; // the recipe being added to the stack for non terminal resources
    Recipe terminal_recipe; // the recipe being added to the stack for terminal resources
    bool is_terminal = false; // marks if a resource is terminal

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain
    int location = 0; // the location of the identical recipe in the vector
    bool already_added = false; // marks if a recipe is already in the vector

    // The variables used to output the data
    json chain_object = json::object();
    json chain_array = json::array();
    json output_object = json::object();
    json output_array = json::array();
    string product_name;
    bool found; // determines if the chain has already been found

    // an empty json array to make the .value() function work
    json empty_array = json::array();

    // creates a vector of all terminal resources
    for (const auto& terminal : terminal_root) {
        terminal_resource.set_resource(terminal);
        terminal_map.insert({terminal_resource.get_name(), terminal_resource});
    }

    auto start = chrono::steady_clock::now(); // starts the timer

    double total_number = 1;
    // creates the intital vector of recipies as well as the incrementors
    for (const auto& data : recipe_root) {
        // adds the first recipe of all items to recipe_list and creates the incrementors
        incrementor_max.push_back(1);
        incrementor.push_back(0);
        all_zeros.push_back(0);

        incrementor_map.insert({data.value("Category", ""), m});
        total_number *= data.value("Data", empty_array).size();

        recipe_input.set_recipe(data.value("Data", empty_array).at(0));
        recipe_map.insert({data.value("Category", ""), recipe_input});
        m += 1;
    }

    cout << total_number << endl;

    int count = 0; // the number of times the loop has run
    double total = 1; // this is double so that it can handle values in the quintilions.

    //
    // Every time it runs, replace every recipe for the item with a simplified version of the chain
    // 

    // The main function, runs until the incrementor vector has returned back to its starting value
    do {
        // clears the output storage vectors
        output_recipes.clear();
        chain_array.clear();

        // Use to inject an ITEM into the system
        m = incrementor_map[test_item];
        for (const auto& data : recipe_root) {
            if (data.value("Category", "") == test_item) {
                recipe_input.set_recipe(data.value("Data", empty_array).at(incrementor.at(m)));
            }
        }
        recipe_stack.push(recipe_input);

        // Use to inject a RECIPE into the system
        // recipe_stack.push(test_recipe);

        current_item = test_recipe.get_product(0).get_name();

        // Creates the recipe chain based on the provided recipes
        while (!recipe_stack.empty()) {
            if (recipe_stack.top().is_processed()) {
                already_added = false;
                // if the current recipe has already been processed, remove it from the stack
                for (int i = 0; i < output_recipes.size(); i++) {
                    if (output_recipes.at(i).same_name(recipe_stack.top())) {
                        already_added = true;
                        location = i;
                        break;
                    }
                }

                if (already_added) {
                    output_recipes.at(location) += recipe_stack.top();
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

                    auto terminal_location = terminal_map.find(ingredients.at(i).get_name()); // finds if the item is terminal
                    if (terminal_location != terminal_map.end()) {
                        // if the ingredient was terminal, adds it to the stack and moves on to the next one
                        /*
                        terminal_recipe.set_terminal_recipe(ingredients.at(i));
                        recipe_stack.push(terminal_recipe);
                        */
                        continue;
                    }

                    auto recipe_location = recipe_map.find(ingredients.at(i).get_name()); // finds the recipe in the map
                    if (recipe_location != recipe_map.end()) {
                        new_recipe = recipe_location->second; // sets new_recipe to the recipe found
                        new_recipe.set_to(ingredients.at(i).get_amount()); // raises the recipe product to match the ingredient it is for
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

        // Detects what products were created and adds their recipe to the incrementors
        for (int i = 0; i < output_recipes.size(); i++) {
            product_name = output_recipes.at(i).get_product(0).get_name();
            m = incrementor_map[product_name];
            for (const auto& data : recipe_root) {
                if (data.value("Category", "") == product_name) {
                    incrementor_max.at(m) = data.value("Data", empty_array).size();
                }
            }
        }
        
        // converts the output vector into uncompressed json
        /*
        for (int i = 0; i < output_recipes.size(); i++) {
            chain_object = output_recipes.at(i).to_json();
            chain_array.push_back(chain_object);
        }
        output_object = chain_array;
        */

        // converts the output vector into compressed json
        Recipe output;
        output.merge_recipes(output_recipes);
        output.set_name(current_item);
        output_object = output.to_json();
        
        // checks if the recipe combination has already been found
        found = false;
        for (int i = 0; i < output_array.size(); i++) {
            if (output_object == output_array.at(i)) {
                found = true;
            }
        }

        if (!found) {
            output_array.push_back(output_object);
        }

        // increments the incrementor vector
        vector<int> incrementor_values;
        for (int i = 0; i < output_recipes.size(); i++) {
            product_name = output_recipes.at(i).get_product(0).get_name();
            incrementor_values.push_back(incrementor_map[product_name]);
        }
        sort(incrementor_values.begin(), incrementor_values.end());
        bool increment = true;
        for (int j = 0; j < incrementor_values.size(); j++) {
            int i = incrementor_values.at(j);
            if (increment) {
                incrementor.at(i) += 1;
                increment = false;
            }
            if (incrementor.at(i) >= incrementor_max.at(i)) {
                incrementor.at(i) = 0;
                increment = true;
            }
        }
        if (increment) {
            for (int j = 0; j < incrementor.size(); j++) {
                incrementor.at(j) = 0;
            }
        }

        /*
        for (int i = 0; i < incrementor.size(); i++) {
            // if the incrementor at the current index has reached its max value, reset it to 0 and increment the next index
            if (incrementor.at(i) >= incrementor_max.at(i)) {
                incrementor.at(i) = 0;
                if ((i + 1) < incrementor.size()) {
                    incrementor.at(i + 1) += 1;
                }
                // if the last index has reached its max value, reset it to 0 to end the loop
                else {
                    incrementor.at(0) = 0;
                }
            }
        }
        */
        
        // updates recipe list for the next loop
        m = 0; // resets m
        for (const auto& data : recipe_root) {
            recipe_input.set_recipe(data.value("Data", empty_array).at(incrementor.at(m)));
            recipe_map.at(data.value("Category", "")) = recipe_input;
            m += 1;
        }

        // outputs the debugging data about the recipe list        
        count += 1;
        /*cout << output_array.size() << " combinations have been found.\n";
        cout << "The program has tested " << count << " combinations of recipes." << endl;*/

        total = 1;
        for (int i = 0; i < incrementor_max.size(); i++) {
            total *= incrementor_max.at(i);
        }

        if ((chrono::steady_clock::now() - start) >= (update_frequency * k)) {
            k += 1;
            
            auto end = chrono::steady_clock::now();
            chrono::duration<double> elapsed = end - start;

            cout << output_array.size() << " combinations have been found.\n";
            cout << "The program has tested " << count << " combinations of recipes." << endl;
            cout << "The program knows about " << total << " combinations of recipes." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            cout << endl;
        }

        // Use to terminate after a set amount of loops
        /*
        if (count >= max_loops) {
            loop_termination = true;
            break;
        }
        */

        // Use to terminate after a set amount of time
        if (chrono::steady_clock::now() - start >= max_time) {
            time_termination = true;
            break;
        }
    } while (incrementor != all_zeros);

    auto end = chrono::steady_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << output_array.size() << " combinations have been found.\n";
    cout << "The program has tested " << count << " combinations of recipes." << endl;
    cout << "The program knows about " << total << " combinations of recipes." << endl;
    cout << "Execution time: " << elapsed.count() << " seconds." << endl;

    // outputs the results to the file
    results << output_array.dump(4);

    recipe_in.close();
    test_recipe_in.close();
    terminal_recipe_in.close();
    results.close();

    if (loop_termination) {
        cout << "The program exceeded " << max_loops << " loops. The item is too complex." << endl;
    }
    else if (time_termination) {
        cout << "The program exceeded " << max_time.count() << " minutes. The item is too complex." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}