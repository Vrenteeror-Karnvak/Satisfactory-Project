#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
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

class Incrementor {
    vector<size_t> selected_alternate;
    vector<size_t> alternate_count;
    unordered_map<string,size_t> name_to_index;
    public:
        void push_back(string name, size_t alternate, size_t alternate_count) {
            name_to_index.insert({name,selected_alternate.size()});
            selected_alternate.push_back(alternate);
            this->alternate_count.push_back(alternate_count);
        }
        size_t get_index(string name) const {
            return name_to_index.at(name);
        }
        size_t& alternate(size_t index) {
            return selected_alternate[index];
        }
        size_t& alternate_max(size_t index) {
            return alternate_count[index];
        }
        size_t size() const {
            return selected_alternate.size();
        }
        bool all_zeros() const {
            for (size_t i = 0; i<selected_alternate.size(); i++) {
                if (selected_alternate[i]!=0) {
                    return false;
                }
            }
            return true;
        }
};

bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<string>& incrementor_products, const Incrementor& incrementor, ofstream& status_log);

int main(int argc, char* argv[]) {

    //initalization start
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    bool loop_termination = false; // Triggers if the loop runs a set number of times
    bool total_loop_termination = false; // Triggers if the loop runs a set number of times, doesn't clear
    int loop_termination_count = 0; // Counts how many times loop_termination was triggered
    bool time_termination = false; // Triggers if the loops runs a set amount of time
    bool total_time_termination = false; // Triggers if the loops runs a set amount of time, doesn't clear
    int time_termination_count = 0; // Counts how many times time_termination was triggered
    bool duplicate_found = false; // Triggers if a duplicate item is found in the incrementor

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
    ofstream results(exePath / "dat" / "test_results.json");
    ofstream status_log(exePath / "dat" / "test_status.log");

    // The json file containing all recipes as well as the variables needed to increment through them
    json recipe_root;
    recipe_in >> recipe_root;
    recipe_in.close();
    Recipe recipe_input;
    unordered_map<string, Recipe> recipe_map;   //recipe name -> recipe object
    Incrementor incrementor;    //used for handling selection of alternate recipes
    size_t m = 0; //?

    // The json file containing the recipe or item
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    test_recipe_in.close();
    Recipe test_recipe(test_recipe_root.at(0)); // Use to inject a RECIPE into the system
    string test_item = test_recipe_root.at(1).value("ItemClass", ""); // Use to inject an ITEM into the system

    // The auto terminate information
    const int max_loops = stoi(test_recipe_root.at(2).value("max_loops", "0")); // the maximum number of loops the program is allowed to run
    const chrono::minutes max_time(stoi(test_recipe_root.at(2).value("max_time", "0"))); // the max time the program is allowed to run
    size_t num_to_test = static_cast<size_t>(stoi(test_recipe_root.at(2).value("number_items_to_test", "0")) - 1); // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds update_frequency(stoi(test_recipe_root.at(3).value("update_frequency", "0"))); // the frequency the program updates its progress
    int u = 1; // the number of updates
    int max_product = stoi(test_recipe_root.at(4).value("max_product", "0")); // the maximum amount of product a recipe chain is allowed to have

    // The json file containing the terminal resources
    json terminal_root;
    terminal_recipe_in >> terminal_root;
    terminal_recipe_in.close();
    Resource terminal_resource;
    unordered_map<string, Resource> terminal_map;

    // The variables that the stack uses to increment through all nodes
    stack<Recipe> recipe_stack; // the stack of recipes in the chain
    vector<Resource> ingredients; // the ingredients being added
    Resource ingredient; // the current ingredient being added
    Recipe new_recipe; // the recipe being added to the stack for non terminal resources
    Recipe terminal_recipe; // the recipe being added to the stack for terminal resources
    bool is_terminal = false; // marks if a resource is terminal

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain
    size_t location = 0; // the location of the identical recipe in the vector
    bool already_added = false; // marks if a recipe is already in the vector

    // The variables used to output the data
    json chain_object = json::object();
    json chain_array = json::array();
    json output_object = json::object(); // the current output object being processed
    json output_array = json::array(); // the recipes being output into the file
    json filtered_array = json::array(); // the recipes filtered out, not added to output file
    string product_name; // the name of the item being processed
    bool found; // determines if the chain has already been found

    // Status tracking variables
    int total = 0; // the number of unique recipe chains found for the current item
    int true_total = 0; // the total number of unique recipe chains found across all items
    int unfiltered = 0; // the number of recipe chains filtered out for the current item
    int true_unfiltered = 0; // the number of recipe chains filtered out across all items
    int filtered = 0; // the number of recipe chains filtered out for the current item
    int true_filtered = 0; // the number of recipe chains filtered out across all items
    int count = 0; // the number of times the loop has run for the current item
    int true_count = 0; // the total number of times the loop has run across all items

    // an empty json array to make the .value() function work
    json empty_array = json::array();

    auto true_start = chrono::steady_clock::now();

    // creates a vector of all terminal resources
    for (const auto& terminal : terminal_root) {
        terminal_resource.set_resource(terminal);
        terminal_map.insert({terminal_resource.get_name(), terminal_resource});
    }

    for (const auto& data : recipe_root) {
        // adds the first recipe of all items to recipe_list and creates the incrementors
        string category = data.value("Category", "");
        size_t selected_recipe = 0,
            recipe_count = data.value("Data", empty_array).size();

        incrementor.push_back(category, selected_recipe, recipe_count);

        recipe_input.set_recipe(data.value("Data", empty_array).at(0));

        recipe_map.insert({data.value("Category", ""), recipe_input});
    }



    for (size_t k = 0; k < recipe_root.size(); k++) {
        auto start = chrono::steady_clock::now(); // starts the timer

        // Clears termination flags and debug variables
        loop_termination = false;
        time_termination = false;
        u = 1;

        // clears the output storage vectors
        output_array.clear();
        filtered_array.clear();

        // Sets the item being processed
        test_item = recipe_root.at(k).value("Category", ""); //overwrite test_item to run like main mega

        count = 0;
        unfiltered = 0;
        filtered = 0;
        total = 0;

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            // clears the output storage vectors
            output_recipes.clear();
            chain_array.clear();

            // Use to inject an item into the system
            m = incrementor.get_index(test_item);
            recipe_input.set_recipe(recipe_root.at(m).value("Data", empty_array).at(incrementor.alternate(m)));
            recipe_stack.push(recipe_input);
            
            // Creates the recipe chain based on the provided recipes
            while (!recipe_stack.empty()) {
                if (recipe_stack.top().is_processed()) {
                    already_added = false;
                    // if the current recipe has already been processed, remove it from the stack
                    for (size_t i = 0; i < output_recipes.size(); i++) {
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
                    for (size_t i = 0; i < ingredients.size(); i++) { // increments through all the ingredients
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
            
            // converts the output vector into uncompressed json
            /*
            for (size_t i = 0; i < output_recipes.size(); i++) {
                chain_object = output_recipes.at(i).to_json();
                chain_array.push_back(chain_object);
            }
            output_object = chain_array;
            */

            // converts the output vector into compressed json
            Recipe output;
            int lm = 1; // the least common multiple of the denominators
            string incrementor_ID = ""; // The ID that identifies what recipes were used to make the chain
            output.merge_recipes(output_recipes);
            output.set_primary_product(test_item);
            for (size_t i = 0; i < incrementor.size(); i++) {
                incrementor_ID.append(to_string(incrementor.alternate(i)));
                if ((i + 1) != incrementor.size()) {
                    incrementor_ID.append("|");
                }
            }
            output.set_ID(incrementor_ID);
            output.set_name(test_item);
            for (size_t i = 0; i < output.get_ingredients().size(); i++) {
                lm = lcm(lm, output.get_ingredient(i).get_amount().get_denominator());
            }
            for (size_t i = 0; i < output.get_products().size(); i++) {
                lm = lcm(lm, output.get_product(i).get_amount().get_denominator());
            }
            output *= lm;
            output_object = output.to_compressed_json();

            // Checks if the total output is more than 100 and doesn't add it if it is
            if (output.get_product(0).get_amount() <= max_product) {
                // if the recipe is valid, adds it to the output
                output_array.push_back(output_object);
                unfiltered += 1;
                true_unfiltered += 1;
                total += 1;
                true_total += 1;
            }
            else {
                // if the recipe is not valid, removes it
                filtered += 1;
                true_filtered += 1;
                total += 1;
                true_total += 1;
            }

            // increments the incrementor vector
            vector<size_t> incrementor_values;
            vector<string> incrementor_products;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                product_name = output_recipes.at(i).get_product(0).get_name();
                incrementor_values.push_back(incrementor.get_index(product_name));
                incrementor_products.push_back(product_name);
            }
            sort(incrementor_values.begin(), incrementor_values.end());
            duplicate_found = check_duplicate_incrementor_values(incrementor_values, incrementor_products, incrementor, status_log);
            bool increment = true; // Determines if the value should be incremented
            for (size_t j = 0; j < incrementor_values.size(); j++) {
                int index = incrementor_values.at(j);
                // if the value needs to be incremented, add one to it
                if (increment) {
                    incrementor.alternate(index) += 1;
                    increment = false;
                }
                // if the value has reached its maximum, set it to zero and set to increment the next value
                if (incrementor.alternate(index) >= incrementor.alternate_max(index)) {
                    incrementor.alternate(index) = 0;
                    increment = true; //carry the addition
                }

                recipe_input.set_recipe(recipe_root.at(index).value("Data", empty_array).at(incrementor.alternate(index)));
                recipe_map.at(recipe_root.at(index).value("Category", "")) = recipe_input;
            }
            
            // If the last value reached its maximum
            // Set all incrementor values to 0 to end the while loop
            if (increment) {
                for (size_t j = 0; j < incrementor.size(); j++) {
                    incrementor.alternate(j) = 0;
                    recipe_input.set_recipe(recipe_root.at(j).value("Data", empty_array).at(0));
                    recipe_map[recipe_root.at(j).value("Category", "")] = recipe_input;
                }
            }

            // Updates the loop counter     
            count += 1;
            true_count += 1;

            // Provides updates on the current status of the program
            if ((chrono::steady_clock::now() - start) >= (update_frequency * u)) {
                u += 1;
                
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;

                cout << test_item << " is being proccessed." << endl;
                cout << total << " combinations have been found." << endl;
                cout << unfiltered << " recipes had a product amount less than or equal to " << max_product << "." << endl;
                cout << filtered << " recipes had a product amount greater than " << max_product << "." << endl;
                cout << "The program has tested " << count << " combinations of recipes." << endl;
                cout << "Execution time: " << elapsed.count() << " seconds." << endl;
                cout << endl;
            }

            // Use to terminate after a set amount of loops
            if (count >= max_loops) {
                total_loop_termination = true;
                loop_termination = true;
                break;
            }

            // Use to terminate after a set amount of time
            if (chrono::steady_clock::now() - start >= max_time) {
                total_time_termination = true;
                time_termination = true;
                break;
            }
        } while (!duplicate_found && !incrementor.all_zeros());

        if (duplicate_found) {
            break;
        }

        m = incrementor.get_index(test_item);
        recipe_root.at(m)["Data"] = output_array;
        incrementor.alternate_max(m) = output_array.size();

        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;

        cout << test_item << " has been proccessed." << endl;
        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been found." << endl;
        status_log << unfiltered << " recipes had a product amount less than or equal to " << max_product << "." << endl;
        status_log << filtered << " recipes had a product amount greater than " << max_product << "." << endl;
        status_log << "The program has tested " << count << " combinations of recipes." << endl;
        status_log << "Execution time: " << elapsed.count() << " seconds." << endl;
        if (loop_termination) {
            status_log << "The program exceeded " << max_loops << " loops. The item is too complex." << endl;
            loop_termination_count++;
        }
        else if (time_termination) {
            status_log << "The program exceeded " << max_time.count() << " minutes. The item is too complex." << endl;
            time_termination_count++;
        }
        status_log << endl;

        if (elapsed >= update_frequency || loop_termination || time_termination) {
            cout << total << " combinations have been found." << endl;
            cout << unfiltered << " recipes had a product amount less than or equal to " << max_product << "." << endl;
            cout << filtered << " recipes had a product amount greater than " << max_product << "." << endl;
            cout << "The program has tested " << count << " combinations of recipes." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            if (loop_termination) {
                cout << "The program exceeded " << max_loops << " loops. The item is too complex." << endl;
            }
            else if (time_termination) {
                cout << "The program exceeded " << max_time.count() << " minutes. The item is too complex." << endl;
            }
            cout << endl;
        }

        if (k == num_to_test) {
            break;
        }
    }

    auto true_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = true_end - true_start;
    cout << true_total << " combinations have been found across all items." << endl;
    cout << true_unfiltered << " recipes had a product amount less than or equal to " << max_product << " across all items." << endl;
    cout << true_filtered << " recipes had a product amount greater than " << max_product << " across all items." << endl;
    cout << "The program has tested " << true_count << " combinations of recipes across all items." << endl;
    cout << "Execution time: " << total_elapsed.count() << " seconds." << endl;

    status_log << true_total << " combinations have been found across all items." << endl;
    status_log << true_unfiltered << " recipes had a product amount less than or equal to " << max_product << " across all items." << endl;
    status_log << true_filtered << " recipes had a product amount greater than " << max_product << " across all items." << endl;
    status_log << "The program has tested " << true_count << " combinations of recipes across all items." << endl;
    status_log << "Execution time: " << total_elapsed.count() << " seconds." << endl;

    // outputs the results to the file
    results << recipe_root.dump(4);
    
    results.close();
    status_log.close();

    if (total_loop_termination) {
        cout << loop_termination_count << " items exceeded " << max_loops << " loops." << endl;
    }
    else if (total_time_termination) {
        cout << time_termination_count << " items exceeded " << max_time.count() << " minutes." << endl;
    }
    else if (duplicate_found) {
        cout << "A duplicate was found in the incrementor. Program terminated while processing " << test_item << "." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}



bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<string>& incrementor_products, const Incrementor& incrementor, ofstream& status_log) {
    bool duplicate_found = false;
    for (size_t d = 0; d < incrementor_values.size(); d++) {
        for (size_t f = d + 1; f < incrementor_values.size(); f++) {
            if (incrementor_values.at(d) == incrementor_values.at(f)) {
                duplicate_found = true;
                break;
            }
        }
        if (duplicate_found) {
            break;
        }
    }

    if (duplicate_found) {
        // outputs the error into the terminal
        cerr << "ERROR: duplicate incrementor index detected; This should not occur." << endl;
        cerr << "   output_recipes.size() = " << incrementor_values.size() << endl;
        cerr << "   incrementor_values = [";
        for (size_t j = 0; j < incrementor_values.size(); j++) {
            cerr << incrementor_values.at(j);
            if (j + 1 < incrementor_values.size()) {
                cerr << ", ";
            }
        }
        cerr << "]" << endl;
        for (size_t j = 0; j < incrementor_products.size(); j++) {
            cerr << "   Recipe " << j << ": product(0) = '" << incrementor_products.at(j) << "' index = " << incrementor.get_index(incrementor_products.at(j)) << endl;
        }

        // outputs the error into the log file
        status_log << "ERROR: duplicate incrementor index detected; This should not occur." << endl;
        status_log << "   output_recipes.size() = " << incrementor_values.size() << endl;
        status_log << "   incrementor_values = [";
        for (size_t j = 0; j < incrementor_values.size(); j++) {
            status_log << incrementor_values.at(j);
            if (j + 1 < incrementor_values.size()) {
                status_log << ", ";
            }
        }
        status_log << "]" << endl;
        for (size_t j = 0; j < incrementor_products.size(); j++) {
            status_log << "   Recipe " << j << ": product(0) = '" << incrementor_products.at(j) << "' index = " << incrementor.get_index(incrementor_products.at(j)) << endl;
        }
        status_log << endl;
    }

    return duplicate_found;
}