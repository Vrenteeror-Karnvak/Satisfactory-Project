#include "../lib/json.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include <chrono>

#include "../resources/recipe.h"
#include "../resources/resource.h"
#include "../resources/fraction.h"

#include "../resources/stats.h"

using namespace std;
using json = nlohmann::ordered_json;

bool merge_ids(vector<int>& candidate_ID, const vector<int>& current_ID, const size_t m);
void increment_incrementor(const vector<Recipe>& output_recipes, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log);
void increment_incrementor(const vector<Resource>& ingredients, const string& product, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log,
    vector<size_t>& incrementor_values);
void increment_incrementor(const vector<Resource>& ingredients, const string& product, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log,
    vector<size_t>& incrementor_values, const unordered_set<string>& nuclear_resources, const vector<char>& is_nuclear);
bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<string>& incrementor_products, const unordered_map<string, size_t>& incrementor_map, ofstream& status_log);

enum class Method {
    BASE,
    COMPRESSED,
    NUCLEAR
};

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    bool duplicate_found = false; // Triggers if a duplicate item is found in the incrementor
    bool base_not_valid = false; // Triggers if the base method takes too long

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    // ofstream results(exePath / "dat" / "test_results.json");
    ofstream status_log(exePath / "dat" / "test_status.log");

    // The json file containing all recipes as well as the variables needed to increment through them
    json recipe_root;
    recipe_in >> recipe_root;
    unordered_map<string, Recipe> recipe_map;
    vector<size_t> incrementor;
    vector<size_t> incrementor_max;
    vector<size_t> incrementor_values;
    vector<size_t> all_zeros(recipe_root.size(), 0);
    unordered_map<string, size_t> incrementor_map; // the location of the incrementor for a given product inside of the incrementor vector
    unordered_map<string, vector<Recipe>> base_recipes; // holds all the base recipes
    unordered_map<string, vector<Recipe>> compressed_recipes; // holds all the compressed recipes
    size_t m = 0;
    recipe_in.close();

    // The json file containing the recipe or item
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    Recipe test_recipe(test_recipe_root[0]); // Use to inject a RECIPE into the system
    string test_item = test_recipe_root[1].value("ItemClass", ""); // Use to inject an ITEM into the system

    // The auto terminate information
    const chrono::seconds update_frequency(stoi(test_recipe_root[3].value("update_frequency", "0"))); // the frequency the program updates its progress
    chrono::duration<double> failed_elapsed;

    // The filter information
    // unsigned int absolute_max_output = test_recipe_root[4].value("absolute_max_combinations", 10000);
    bool remake_filters = test_recipe_root[4].value("remake_filters", false); // whether or not to remake the filter
    test_recipe_in.close();

    unordered_set<string> terminal_resources;
    unordered_set<string> nuclear_resources;
    unordered_set<string> capstone_resources;
    vector<char> is_nuclear; // is the item nuclear?
    {
        // The json file containing the terminal resources
        ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
        json terminal_root;
        terminal_recipe_in >> terminal_root;
        terminal_recipe_in.close();
        // creates an unordered map of all terminal resources
        for (const auto& terminal : terminal_root) {
            terminal_resources.insert(terminal.value("ItemClass", ""));
        }

        ifstream nuclear_recipe_in(exePath / "dat" / "nuclear_resources.json");
        json nuclear_root;
        nuclear_recipe_in >> nuclear_root;
        nuclear_recipe_in.close();
        // creates an unordered map of all nuclear resources
        for (const auto& nuclear : nuclear_root) {
            nuclear_resources.insert(nuclear.value("ItemClass", ""));
        }

        ifstream capstone_recipe_in(exePath / "dat" / "capstone_resources.json");
        json capstone_root;
        capstone_recipe_in >> capstone_root;
        capstone_recipe_in.close();
        // creates an unordered map of all nuclear resources
        for (const auto& capstone : capstone_root) {
            capstone_resources.insert(capstone.value("ItemClass", ""));
        }
    }

    // The variables that the stack uses to increment through all nodes
    stack<Recipe> recipe_stack; // the stack of recipes in the chain

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain
    size_t location = 0; // the location of the identical recipe in the vector
    bool already_added = false; // marks if a recipe is already in the vector

    // The variables used to output the data
    json chain_object = json::object();
    json chain_array = json::array();
    json output_chain = json::object(); // the current recipe chain being processed
    json output_object = json::object(); // the json object containing all recipe chains being sent to the file
    vector<Recipe> output_vector;
    json output_array = json::array(); // the recipes being output into the file
    string product_name; // the name of the item being processed
    // bool first = true; // is this the first item being output in the given array?

    // Status tracking variables
    int total = 0; // the number of unique recipe chains found for the current item
    int true_total = 0; // the total number of unique recipe chains found across all items
    int unfiltered = 0; // the number of recipe chains filtered out for the current item
    int true_unfiltered = 0; // the number of recipe chains filtered out across all items
    int machine_filtered = 0; // the number of recipe chains filtered out for the current item due to the number of machines
    int true_machine_filtered = 0; // the number of recipe chains filtered out due to the number of machines across all items

    // an empty json array to make the .value() function work
    json empty_array = json::array();

    auto true_start = chrono::steady_clock::now();

    // creates a map of the filters
    ifstream filters_in(exePath / "dat" / "100_combination_filter.json");
    json filter_json;
    filters_in >> filter_json;
    unordered_map<string, int> filter_map;
    for (const auto& data : filter_json) {
        if (remake_filters) {
            filter_map.insert({data.value("ItemClass", "N/A"), 0});
        }
        else {
            filter_map.insert({data.value("ItemClass", "N/A"), data.value("Depth", 0)});
        }
    }
    filters_in.close();

    for (const auto& data : recipe_root) {
        // adds the first recipe of all items to recipe_list and creates the incrementors
        // also builds the recipe map
        output_recipes.clear();
        Recipe recipe_input;
        test_item = data["Category"];
        incrementor_max.push_back(data.value("Data", empty_array).size());
        incrementor.push_back(0);

        for (const auto& recipe : data["Data"]) {
            recipe_input.set_recipe(recipe);
            output_recipes.push_back(recipe_input);
        }

        incrementor_map.insert({test_item, m});
        base_recipes.insert({test_item, output_recipes});
        compressed_recipes.insert({test_item, output_recipes});

        recipe_input.set_recipe(data.value("Data", empty_array).at(0));
        recipe_map.insert({test_item, recipe_input});

        if (nuclear_resources.find(test_item) != nuclear_resources.end()) {
            is_nuclear.push_back(true);
        }
        else {
            is_nuclear.push_back(false);
        }
        
        m += 1;
    }

    size_t u = 0;

    //
    // Nothing currently
    //

    // results << "[" << endl;

    auto total_start = chrono::steady_clock::now();

    for (size_t k = 0; k < recipe_root.size(); k++) {
        // clears the output storage vectors
        output_vector.clear();

        // Sets the item being processed
        test_item = recipe_root[k]["Category"];

        Method method;
        if (test_item == "Ficsonium Fuel Rod") {
            method = Method::BASE;
        }
        else {
            continue;
        }
        const unordered_map<string, vector<Recipe>>* recipes_ptr = (method == Method::BASE) ? &base_recipes : &compressed_recipes;

        for (size_t i = 0; i <= k; i++) {
            product_name = recipe_root.at(i)["Category"];
            incrementor_max[i] = (*recipes_ptr).at(product_name).size();
            recipe_map.at(product_name) = (*recipes_ptr).at(product_name).at(incrementor[i]);
        }

        unfiltered = 0;
        machine_filtered = 0;
        total = 0;

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            // clears the output storage vectors
            output_recipes.clear();
            chain_array.clear();

            // Use to inject an item into the system
            m = incrementor_map[test_item];
            Stats::incrementor_map_lookups++;
            const Recipe& starting_recipe = (*recipes_ptr).at(test_item).at(incrementor[m]);
            Stats::recipes_map_lookups++;

            Stats::combinations_processed++;
            
            // Merge the ID's to detect any conflicts
            vector<int> candidate_ID(recipe_root.size(), -1);
            vector<Resource> ingredients;
            ingredients = starting_recipe.get_ingredients();

            Stats::chains_generated++;
            recipe_stack.push(starting_recipe);
            Stats::recipes_pushed_to_stack++;
            Stats::max_chain_depth = max(Stats::max_chain_depth, recipe_stack.size());
            
            // Creates the recipe chain based on the provided recipes
            auto chain_start = chrono::steady_clock::now();
            while (!recipe_stack.empty()) {
                if (recipe_stack.top().is_processed()) {
                    already_added = false;
                    // if the current recipe has already been processed, remove it from the stack
                    for (size_t i = 0; i < output_recipes.size(); i++) {
                        if (output_recipes[i].same_name(recipe_stack.top())) {
                            already_added = true;
                            location = i;
                            break;
                        }
                    }

                    if (already_added) {
                        output_recipes[location] += recipe_stack.top();
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
                    const vector<Resource>& stack_ingredients = recipe_stack.top().get_ingredients_ref(); // gets the ingredients of the current recipe
                    for (size_t i = 0; i < stack_ingredients.size(); i++) { // increments through all the ingredients
                        const Resource& ingredient = stack_ingredients[i];
                        product_name = ingredient.get_name();
                        Stats::terminal_map_searches++;
                        if (terminal_resources.find(product_name) != terminal_resources.end()) {
                            // if the ingredient was terminal, adds it to the stack and moves on to the next one
                            /*
                            Recipe terminal_recipe; // the recipe being added to the stack for terminal resources
                            terminal_recipe.set_terminal_recipe(stack_ingredients[i]);
                            recipe_stack.push(terminal_recipe);
                            */
                            Stats::terminal_hits++;
                            continue;
                        }

                        Stats::recipe_map_searches++;
                        auto recipe_location = recipe_map.find(product_name); // finds the recipe in the map
                        if (recipe_location != recipe_map.end()) {
                            Recipe new_recipe = recipe_location->second; // sets new_recipe to the recipe found
                            new_recipe.set_to(ingredient.get_amount()); // raises the recipe product to match the ingredient it is for
                            recipe_stack.push(new_recipe); // adds the new recipe to the stack
                            Stats::recipes_pushed_to_stack++;
                            Stats::max_chain_depth = max(Stats::max_chain_depth, recipe_stack.size());
                        }
                        else {
                            // if now recipe was found, outputs the fact as there may be missing data somewhere
                            // the program otherwise continues as if the resource was terminal
                            cout << "No recipe found for " << product_name << "." << endl;
                            status_log << "No recipe found for " << product_name << "." << endl;
                        }
                    }
                }
            }
            Stats::output_recipe_samples = output_recipes.size();
            Stats::max_output_recipes = max(Stats::max_output_recipes, Stats::output_recipe_samples);
            Stats::total_output_recipes += Stats::output_recipe_samples;
            auto chain_end = chrono::steady_clock::now();
            Stats::chain_generation_time += (chain_end - chain_start);






            u += 1;
            if (u % 1000000 == 0) {
                auto total_end = chrono::steady_clock::now();
                chrono::duration<double> total_elapsed = (total_end - total_start);
                Stats::total_generation_time = total_elapsed;
                cout << u << " -> " << total_elapsed.count() << endl;
            }
            increment_incrementor(output_recipes, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log);

        } while (!duplicate_found && incrementor != all_zeros);

        if (duplicate_found) {
            break;
        }

        cout << test_item << " has been proccessed." << endl;
        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been processed." << endl;
        status_log << unfiltered << " recipes were output." << endl;
        status_log << machine_filtered << " recipes were filtered due to number of machines." << endl;
        status_log << endl;
    }
    auto total_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = (total_end - total_start);
    Stats::total_generation_time = total_elapsed;

    auto true_end = chrono::steady_clock::now();
    chrono::duration<double> true_elapsed = true_end - true_start;

    cout << true_total << " combinations were processed." << endl;
    cout << true_unfiltered << " recipes were output." << endl;
    cout << true_machine_filtered << " recipes were filtered due to number of machines." << endl;
    cout << "Execution time: " << true_elapsed.count() << " seconds." << endl;

    status_log << true_total << " combinations were processed." << endl;
    status_log << true_unfiltered << " recipes were output." << endl;
    status_log << true_machine_filtered << " recipes were filtered due to number of machines." << endl;
    status_log << "Execution time: " << true_elapsed.count() << " seconds." << endl;
    
    Stats::print(cout);
    Stats::print(status_log);

    status_log.close();

    if (base_not_valid) {
        cout << "The base method is likely not valid for this item." << endl;
        cout << "Program terminated while processing " << recipe_root.at(m)["Category"] << "(k = " << m << ")." << endl;
        cout << total << " combinations had been processed before terminating." << endl;
        cout << "Program ran for " << failed_elapsed.count() << " seconds before terminating." << endl;
    }
    else if (duplicate_found) {
        cout << "A duplicate was found in the incrementor. Program terminated while processing " << test_item << "." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}



bool merge_ids(vector<int>& a, const vector<int>& b, const size_t m) {
    for (size_t i = 0; i < m; i++) {
        if (b[i] == -1) {
            continue;
        }
        else if (a[i] == -1) {
            a[i] = b[i];
        }
        else if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}



// Base Method Incrementor
void increment_incrementor(const vector<Recipe>& output_recipes, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log) {
    auto incrementor_start = chrono::steady_clock::now();
    // increments the incrementor vector 
    string item_name;
    vector<size_t> incrementor_values;
    auto incrementor_rebuild_start = chrono::steady_clock::now();
    for (size_t i = 0; i < output_recipes.size(); i++) {
        item_name = output_recipes.at(i).get_product(0).get_name();
        incrementor_values.push_back(incrementor_map.at(item_name));
    }
    sort(incrementor_values.begin(), incrementor_values.end());
    auto incrementor_rebuild_end = chrono::steady_clock::now();
    Stats::incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, incrementor_products, incrementor_map, status_log);
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor.at(i) += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            item_name = recipe_root[i]["Category"];
            recipe_map[item_name] = recipes.at(item_name).at(incrementor[i]);
            Stats::recipes_map_lookups++;
            Stats::recipe_map_updates++;
        }
    }
    auto incrementor_end = chrono::steady_clock::now();
    Stats::incrementor_time += (incrementor_end - incrementor_start);
}



// Compressed Method Incrementor
void increment_incrementor(const vector<Resource>& ingredients, const string& product, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log,
    vector<size_t>& incrementor_values) {
    auto incrementor_start = chrono::steady_clock::now();
    // increments the incrementor vector
    string item_name;
    bool rebuild_needed = false;
    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, incrementor_products, incrementor_map, status_log);
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor.at(i) += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            item_name = recipe_root[i]["Category"];
            recipe_map[item_name] = recipes.at(item_name).at(incrementor[i]);
            Stats::recipes_map_lookups++;
            Stats::recipe_map_updates++;
            
            if(i == incrementor_values.back()) {
                rebuild_needed = true;
            }
        }
    }
    if (rebuild_needed) {
        Stats::incrementor_rebuild_count++;
        auto incrementor_rebuild_start = chrono::steady_clock::now();
        size_t j = incrementor_values.back();
        incrementor_values.clear();
        const vector<Resource>& new_ingredients = recipes.at(item_name).at(incrementor.at(j)).get_ingredients_ref();
        Stats::recipes_map_lookups++;
        for (size_t i = 0; i < new_ingredients.size(); i++) {
            item_name = new_ingredients[i].get_name();
            Stats::terminal_map_searches++;
            if (terminal_resources.find(item_name) == terminal_resources.end()) {
                incrementor_values.push_back(incrementor_map.at(item_name));
                Stats::incrementor_map_lookups++;
            }
        }
        incrementor_values.push_back(incrementor_map.at(product));
        Stats::incrementor_map_lookups++;
        sort(incrementor_values.begin(), incrementor_values.end());
        auto incrementor_rebuild_end = chrono::steady_clock::now();
        Stats::incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    }
    auto incrementor_end = chrono::steady_clock::now();
    Stats::incrementor_time += (incrementor_end - incrementor_start);
}



// Nuclear Method Incrementor
void increment_incrementor(const vector<Resource>& ingredients, const string& product, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log,
    vector<size_t>& incrementor_values, const unordered_set<string>& nuclear_resources, const vector<char>& is_nuclear) {
    auto incrementor_start = chrono::steady_clock::now();
    // increments the incrementor vector
    string item_name;
    bool rebuild_needed = false;
    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, incrementor_products, incrementor_map, status_log);
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor.at(i) += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            item_name = recipe_root[i]["Category"];
            recipe_map[item_name] = recipes.at(item_name).at(incrementor[i]);
            Stats::recipes_map_lookups++;
            Stats::recipe_map_updates++;
            

            if (is_nuclear[i]) {
                rebuild_needed = true;
            }
        }
    }
    if (rebuild_needed) {
        Stats::incrementor_rebuild_count++;
        auto incrementor_rebuild_start = chrono::steady_clock::now();
        incrementor_values.clear();
        vector<Resource> new_ingredients;
        unordered_set<string> names;
        for (auto& name : nuclear_resources) {
            for (const Resource& ingredient : recipe_map.at(name).get_ingredients_ref()) {
                if (names.insert(ingredient.get_name()).second) {
                    new_ingredients.push_back(ingredient);
                }
            }
        }
        Stats::recipes_map_lookups++;
        for (size_t i = 0; i < new_ingredients.size(); i++) {
            item_name = new_ingredients[i].get_name();
            Stats::terminal_map_searches++;
            if (terminal_resources.find(item_name) == terminal_resources.end()) {
                incrementor_values.push_back(incrementor_map.at(item_name));
                Stats::incrementor_map_lookups++;
            }
        }
        incrementor_values.push_back(incrementor_map.at(product));
        Stats::incrementor_map_lookups++;
        sort(incrementor_values.begin(), incrementor_values.end());
        auto incrementor_rebuild_end = chrono::steady_clock::now();
        Stats::incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    }
    auto incrementor_end = chrono::steady_clock::now();
    Stats::incrementor_time += (incrementor_end - incrementor_start);
}



bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<string>& incrementor_products, const unordered_map<string, size_t>& incrementor_map, ofstream& status_log) {
    bool duplicate_found = false;
    for (size_t d = 0; d < incrementor_values.size(); d++) {
        for (size_t f = d + 1; f < incrementor_values.size(); f++) {
            if (incrementor_values[d] == incrementor_values[f]) {
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
            cerr << incrementor_values[j];
            if (j + 1 < incrementor_values.size()) {
                cerr << ", ";
            }
        }
        cerr << "]" << endl;
        for (size_t j = 0; j < incrementor_products.size(); j++) {
            cerr << "   Recipe " << j << ": product(0) = '" << incrementor_products[j] << "' index = " << incrementor_map.at(incrementor_products[j]) << endl;
        }

        // outputs the error into the log file
        status_log << "ERROR: duplicate incrementor index detected; This should not occur." << endl;
        status_log << "   output_recipes.size() = " << incrementor_values.size() << endl;
        status_log << "   incrementor_values = [";
        for (size_t j = 0; j < incrementor_values.size(); j++) {
            status_log << incrementor_values[j];
            if (j + 1 < incrementor_values.size()) {
                status_log << ", ";
            }
        }
        status_log << "]" << endl;
        for (size_t j = 0; j < incrementor_products.size(); j++) {
            status_log << "   Recipe " << j << ": product(0) = '" << incrementor_products[j] << "' index = " << incrementor_map.at(incrementor_products[j]) << endl;
        }
        status_log << endl;
    }

    return duplicate_found;
}