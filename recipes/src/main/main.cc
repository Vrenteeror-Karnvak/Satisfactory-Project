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
void increment_incrementor(const vector<Resource>& ingredients, const string& product, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log,
    vector<size_t>& incrementor_values);
void increment_incrementor(const vector<Recipe>& output_recipes, const json& recipe_root, unordered_map<string, Recipe>& recipe_map, const unordered_map<string, vector<Recipe>>& recipes,
    const unordered_set<string>& terminal_resources, const unordered_map<string, size_t>& incrementor_map, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log);
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
    ofstream status_log(exePath / "dat" / "backup_test_status.log");

    // The json file containing all recipes as well as the variables needed to increment through them
    json recipe_root;
    recipe_in >> recipe_root;
    unordered_map<string, Recipe> recipe_map;
    vector<size_t> incrementor;
    vector<size_t> incrementor_max;
    vector<size_t> incrementor_values;
    bool first_creation = true; // is this the first time incrementor_values has been made?
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
    size_t num_to_test = static_cast<size_t>(stoi(test_recipe_root[2].value("number_items_to_test", "1")) - 1); // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds update_frequency(stoi(test_recipe_root[3].value("update_frequency", "0"))); // the frequency the program updates its progress
    int u = 1; // the number of updates
    chrono::duration<double> failed_elapsed;

    // The filter information
    int max_num_machines = test_recipe_root[4].value("max_num_machines", 200); // the maximum amount of product a recipe chain is allowed to have
    int max_filter = test_recipe_root[4].value("max_filter", 200); // the maximum number of machines the filter is allowed to have under normal circumstances
    unsigned int max_output = test_recipe_root[4].value("max_combinations", 1000); // the maximum number of combinations being output
    // unsigned int absolute_max_output = test_recipe_root[4].value("absolute_max_combinations", 10000);
    const unsigned int base_max_output = max_output;
    bool remake_filters = test_recipe_root[4].value("remake_filters", false); // whether or not to remake the filter
    bool filter_made = false; // has the filter for the item already been made?
    bool redo_filter = false; // does the filter need to be redone to futher reduce the output?
    int number_of_machines = 0;
    test_recipe_in.close();

    unordered_set<string> terminal_resources;
    unordered_set<string> nuclear_resources;
    unordered_set<string> capstone_resources;
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
    int count = 0; // the number of times the loop has run for the current item

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
        m += 1;
    }

    //
    // Nothing currently
    //

    // results << "[" << endl;

    auto total_start = chrono::steady_clock::now();

    for (size_t k = 0; k < recipe_root.size(); k++) {
        auto start = chrono::steady_clock::now(); // starts the timer

        // Clears termination flags and debug variables
        first_creation = true;
        u = 1;

        // clears the output storage vectors
        output_vector.clear();

        // Sets the item being processed
        test_item = recipe_root[k]["Category"];

        Method method;
        if (test_item == "Ficsonium Fuel Rod") {
            // method = Method::NUCLEAR;
            method = Method::BASE;
        }
        else if (nuclear_resources.find(test_item) != nuclear_resources.end()) {
            cout << "Skipping " << test_item << ".\n";
            status_log  << "Skipping " << test_item << ".\n\n";
            if ((k - 1) == num_to_test) {
                break;
            }
            else {
                continue;
            }
        }
        else if (k <= 69 || k == 71 || k == 78 || k == 82 || k == 85 || k == 87) {
            method = Method::BASE;
        }
        else {
            method = Method::COMPRESSED;
        }
        const unordered_map<string, vector<Recipe>>* recipes_ptr = (method == Method::BASE) ? &base_recipes : &compressed_recipes;

        if (!remake_filters && filter_map.at(test_item) != 0) {
            filter_made = true;
        }
        
        if (redo_filter) {
            filter_made = false;
            redo_filter = false;
        }

        for (size_t i = 0; i <= k; i++) {
            product_name = recipe_root.at(i)["Category"];
            incrementor_max[i] = (*recipes_ptr).at(product_name).size();
            recipe_map.at(product_name) = (*recipes_ptr).at(product_name).at(incrementor[i]);
        }

        count = 0;
        unfiltered = 0;
        machine_filtered = 0;
        total = 0;

        auto count_start = chrono::steady_clock::now();
        for (size_t i = 0; i < (*recipes_ptr).at(test_item).size(); i++) {
            const vector<Resource>& recipe_vector = (*recipes_ptr).at(test_item)[i].get_ingredients_ref();
            int temp_count = 1;

            for (size_t j = 0; j < recipe_vector.size(); j++) {
                string temp_item = recipe_vector[j].get_name();
                if (terminal_resources.find(temp_item) == terminal_resources.end()) {
                    temp_count *= (*recipes_ptr).at(temp_item).size();
                }
            }
            count += temp_count;
        }
        auto count_end = chrono::steady_clock::now();
        Stats::current_method_stats().count_time += (count_end - count_start);

        status_log << test_item << " about to be processed." << endl;
        if (method == Method::COMPRESSED) {
            status_log << "Estimated to need to process " << count << " combinations to complete." << endl;
        }
        else {
            status_log << "Can not estimate number of combinations." << endl;
        }

        if ((k - 1) == num_to_test) {
            status_log << endl;
            break;
        }

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            // clears the output storage vectors
            output_recipes.clear();
            chain_array.clear();

            // Use to inject an item into the system
            m = incrementor_map[test_item];
            const Recipe& starting_recipe = (*recipes_ptr).at(test_item).at(incrementor[m]);

            Stats::current_method_stats().combinations_processed++;
            
            // Merge the ID's to detect any conflicts
            vector<int> candidate_ID(recipe_root.size(), -1);
            const vector<Resource>& ingredients = starting_recipe.get_ingredients_ref();
            if (method == Method::COMPRESSED) {
                auto ID_start = chrono::steady_clock::now();
                bool valid = true;

                auto ID_end = chrono::steady_clock::now();
                Stats::current_method_stats().ID_total_time += (ID_end - ID_start);

                if (first_creation) {
                    Stats::current_method_stats().incrementor_rebuild_count++;
                    auto incrementor_rebuild_start = chrono::steady_clock::now();
                    incrementor_values.clear();
                    for (size_t i = 0; i < ingredients.size(); i++) {
                        string item_name = ingredients[i].get_name();
                        if (terminal_resources.find(item_name) == terminal_resources.end()) {
                            incrementor_values.push_back(incrementor_map.at(item_name));
                        }
                    }
                    incrementor_values.push_back(incrementor_map.at(test_item));
                    sort(incrementor_values.begin(), incrementor_values.end());
                    first_creation = false;
                    auto incrementor_rebuild_end = chrono::steady_clock::now();
                    Stats::current_method_stats().incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
                }

                ID_start = chrono::steady_clock::now();
                auto ID_build_start = chrono::steady_clock::now();

                for (size_t i = 0; i < ingredients.size(); i++) {
                    valid = true;
                    const Resource& item = ingredients[i];
                    product_name = item.get_name();
                    if (terminal_resources.find(product_name) == terminal_resources.end()) {
                        location = incrementor_map[product_name];
                        const Recipe& current_recipe = (*recipes_ptr).at(product_name).at(incrementor[location]);
                        if (!current_recipe.get_ID_ref().empty()) {
                            if (i == 0) {
                                candidate_ID = current_recipe.get_ID();
                            }
                            else if (!merge_ids(candidate_ID, current_recipe.get_ID(), m)) {
                                Stats::current_method_stats().merge_ID_calls++;
                                valid = false;
                                break;
                            }
                        }
                    }
                }
                candidate_ID[m] = incrementor[m];

                uint64_t current_ID_entries = 0;
                for (size_t i = 0; i < candidate_ID.size(); i++) {
                    if (candidate_ID[i] != -1) {
                        current_ID_entries++;
                    }
                }
                Stats::current_method_stats().total_ID_entries_used += current_ID_entries;
                Stats::current_method_stats().max_ID_entries_used = max(Stats::current_method_stats().max_ID_entries_used, current_ID_entries);

                auto ID_build_end = chrono::steady_clock::now();
                Stats::current_method_stats().ID_build_time += (ID_build_end - ID_build_start);

                if (!valid) {
                    Stats::current_method_stats().ID_filtered++;
                    machine_filtered += 1;
                    true_machine_filtered += 1;
                    total += 1;
                    true_total += 1;

                    auto ID_end = chrono::steady_clock::now();
                    Stats::current_method_stats().ID_total_time += (ID_end - ID_start);
                    increment_incrementor(ingredients, test_item, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log, incrementor_values);
                    continue;
                }
                ID_end = chrono::steady_clock::now();
                Stats::current_method_stats().ID_total_time += (ID_end - ID_start);
            }

            Stats::current_method_stats().chains_generated++;
            recipe_stack.push(starting_recipe);
            
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
                        if (terminal_resources.find(product_name) != terminal_resources.end()) {
                            // if the ingredient was terminal, adds it to the stack and moves on to the next one
                            /*
                            Recipe terminal_recipe; // the recipe being added to the stack for terminal resources
                            terminal_recipe.set_terminal_recipe(stack_ingredients[i]);
                            recipe_stack.push(terminal_recipe);
                            */
                            continue;
                        }
                        auto recipe_location = recipe_map.find(product_name); // finds the recipe in the map
                        if (recipe_location != recipe_map.end()) {
                            Recipe new_recipe = recipe_location->second; // sets new_recipe to the recipe found
                            new_recipe.set_to(ingredient.get_amount()); // raises the recipe product to match the ingredient it is for
                            recipe_stack.push(new_recipe); // adds the new recipe to the stack
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
            auto chain_end = chrono::steady_clock::now();
            Stats::current_method_stats().chain_generation_time += (chain_end - chain_start);
            
            // converts the output vector into uncompressed json
            /*
            for (size_t i = 0; i < output_recipes.size(); i++) {
                chain_object = output_recipes[i].to_json();
                chain_array.push_back(chain_object);
            }
            output_chain = chain_array;
            output_array.push_back(output_chain);
            */

            auto lcm_start = chrono::steady_clock::now();
            int speed_lm = 1;
            Fraction rate;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                Resource product = output_recipes[i].get_product(0);
                product_name = product.get_name();
                rate = (product.get_amount() / recipe_map.at(product_name).get_product(0).get_amount());
                rate *= recipe_map.at(product_name).get_machine_speed();
                rate /= 60;
                speed_lm = lcm(speed_lm, rate.get_denominator());
                Stats::current_method_stats().lcm_calls++;
            }

            rate *= speed_lm;
            Stats::current_method_stats().max_machine_count = max(Stats::current_method_stats().max_machine_count, (static_cast<double>(rate.get_numerator()) / rate.get_denominator()));
            if (rate > max_num_machines) {
                Stats::current_method_stats().speed_filtered++;
                machine_filtered += 1;
                true_machine_filtered += 1;
                total += 1;
                true_total += 1;

                auto lcm_end = chrono::steady_clock::now();
                Stats::current_method_stats().lcm_time += (lcm_end - lcm_start);
                if (method == Method::COMPRESSED) {
                    increment_incrementor(ingredients, test_item, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log, incrementor_values);
                }
                else if (method == Method::BASE) {
                    increment_incrementor(output_recipes, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log);
                }
                continue;
            }

            auto lcm_end = chrono::steady_clock::now();
            Stats::current_method_stats().lcm_time += (lcm_end - lcm_start);

            // converts the output vector into compressed json
            Recipe output;
            auto merge_start = chrono::steady_clock::now();
            if (method == Method::BASE) {
                for (size_t i = 0; i < output_recipes.size(); i++) {
                    size_t j = incrementor_map.at(output_recipes[i].get_product(0).get_name());
                    candidate_ID[j] = incrementor[j];
                }
            }
            output.merge_recipes(output_recipes);
            output.set_ID(candidate_ID);
            output.set_name(test_item);
            output *= speed_lm;

            Stats::current_method_stats().max_speed_lm = max(Stats::current_method_stats().max_speed_lm, static_cast<uint64_t>(speed_lm));
            auto merge_end = chrono::steady_clock::now();
            Stats::current_method_stats().merge_time += (merge_end - merge_start);
            // output_chain = output.to_compressed_json();

            rate = (output.get_product(0).get_amount() / recipe_map.at(test_item).get_product(0).get_amount());
            rate /= 60;
            rate *= recipe_map.at(test_item).get_machine_speed();
            number_of_machines = rate.get_numerator();
            if (number_of_machines < 0 || number_of_machines > 10000) {
                number_of_machines = 2147483647;
            }
            output.set_machine_speed(number_of_machines);

            // Checks if the total number of machines is more than the maximum and doesn't add it if it is
            if ((number_of_machines > 0 && number_of_machines <= max_num_machines) && (number_of_machines <= filter_map.at(test_item) || filter_map.at(test_item) == 0)) {
                // if the recipe is valid, adds it to the output
                output_vector.push_back(output);

                unfiltered += 1;
                true_unfiltered += 1;
                total += 1;
                true_total += 1;
            }
            else {
                // if the recipe is not valid, removes it
                Stats::current_method_stats().merge_filtered++;
                machine_filtered += 1;
                true_machine_filtered += 1;
                total += 1;
                true_total += 1;
            }

            if (method == Method::COMPRESSED) {
                increment_incrementor(ingredients, test_item, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log, incrementor_values);
            }
            else if (method == Method::BASE) {
                increment_incrementor(output_recipes, recipe_root, recipe_map, (*recipes_ptr), terminal_resources, incrementor_map, incrementor, incrementor_max, status_log);
            }

            // Provides updates on the current status of the program
            if ((chrono::steady_clock::now() - start) >= (update_frequency * u)) {
                u += 1;
                
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;

                cout << test_item << " is being proccessed." << endl;
                cout << total << " combinations have been processed." << endl;
                cout << unfiltered << " recipes were output." << endl;
                cout << machine_filtered << " recipes were filtered due to number of machines." << endl;
                cout << "Execution time: " << elapsed.count() << " seconds." << endl;
                cout << endl;
            }
            if (total >= 10000000) {
                if (method == Method::BASE && test_item != "Ficsonium Fuel Rod") {
                    auto end = chrono::steady_clock::now();
                    failed_elapsed = end - start;
                    
                    base_not_valid = true;
                    duplicate_found = true;
                }
            }
        } while (!duplicate_found && incrementor != all_zeros);

        if (duplicate_found) {
            break;
        }

        if ((chrono::steady_clock::now() - start) >= update_frequency) {  
            auto creation_end = chrono::steady_clock::now();
            chrono::duration<double> creation_elapsed = creation_end - start;
            cout << test_item << " was proccessed." << endl;
            cout << total << " combinations were found." << endl;
            cout << "Execution time: " << creation_elapsed.count() << " seconds." << endl;
            cout << endl;
        }
        
        auto minimum = min_element(output_vector.begin(), output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
        if (!filter_made) {
            auto result = max_element(output_vector.begin(), output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
            // checks if the maximum is less than 10. Skips the creation process if it is.
            if ((*result).get_machine_speed() <= 10) {
                filter_map.at(test_item) = 10;
            }
            else {
                if (output_vector.size() > base_max_output) {
                    partial_sort(output_vector.begin(), output_vector.begin() + base_max_output, output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
                    // filter_map.at(test_item) = (ceil(output_vector.at(max_output).get_machine_speed() / 10) * 10);
                    filter_map.at(test_item) = output_vector.at(max_output - 1).get_machine_speed();
                }
                else {
                    filter_map.at(test_item) = (ceil((*result).get_machine_speed() / 10) * 10);
                }
            }
            if (output_vector.at(0).get_machine_speed() > max_filter) {
                filter_map.at(test_item) = (*minimum).get_machine_speed();
            }
            else if (filter_map.at(test_item) > max_filter) {
                filter_map.at(test_item) = max_filter;
            }

            cout << test_item << " filter has been created." << endl;
            status_log << test_item << " filter has been created." << endl;

            filter_made = true;
            true_unfiltered -= unfiltered;
            true_machine_filtered -= machine_filtered;
            k--;
            continue;
        }
        else {
            filter_made = false;
        }

        // If the output_vector is still too large, halve the max output allowed to reduce combinations
        /*
        if (output_vector.size() > absolute_max_output) {
            if ((*minimum).get_machine_speed() == filter_map.at(test_item) || ((*minimum).get_machine_speed() < 10 && filter_map.at(test_item) == 10)) {
                cout << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
                cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                status_log << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
                status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                max_output = base_max_output;
            }
            else {
                cout << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
                cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                status_log << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
                status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                redo_filter = true;
                filter_map.at(test_item) = 0;
                max_output /= 2;
                true_unfiltered -= unfiltered;
                true_machine_filtered -= machine_filtered;
                k--;
                continue;
            }
        }
        else {
            max_output = base_max_output;
        }
        */

        
        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;

        auto pre_output = chrono::steady_clock::now();
        string filter_item;
        for (auto& item : filter_json) {
            filter_item = item.value("ItemClass", "N/A");
            item["Depth"] = filter_map.at(filter_item);
        }

        ofstream filter_out(exePath / "dat" / "100_combination_filter.json");
        filter_out << filter_json.dump(4);
        filter_out.close();

        // preps the array to be output
        output_array.clear();
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_vector[i].set_machine_speed(60.0);
            // output_chain = output_vector[i].to_compressed_json();
            // output_array.push_back(output_chain);
        }
        // output_object["Category"] = test_item;
        // output_object["Data"] = output_array;
        
        if (capstone_resources.find(test_item) == capstone_resources.end()) {
            m = incrementor_map.at(test_item);
            compressed_recipes.at(test_item) = output_vector;
        }

        // outputs the string to the file
        string file_name = "results/" + test_item + ".txt";
        ofstream output_file(exePath / file_name);

        if (!output_file.is_open()) {
            cerr << "Failed to open " << file_name << endl;
            return 0;
        }
        for (const Recipe& recipe : output_vector) {
            output_file << recipe.to_string();
        }
        output_file.close();

        // outputs the array to the file
        /*
        if (!first) {
            results << "," << endl;
        }
        first = false;

        results << output_object.dump(4);
        results.flush();
        */

        /*
        string output_string = output_object.dump(4); // output_object dumped into a string for line by line writing
        string buffer = "    ";
        for (char c : output_string) {
            buffer += c;
            if (c == '\n') buffer += "    ";
        }
        results << buffer;
        buffer.clear();
        */

        // Outputs all at once
        /*
        output_array.clear();
        output_vector = (*recipes_ptr).at(test_item);
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_chain = output_vector[i].to_compressed_json();
            output_array.push_back(output_chain);
        }
        recipe_root[m]["Data"] = output_array;
        */

        auto post_output = chrono::steady_clock::now();
        chrono::duration<double> output_duration = post_output - pre_output;
        Stats::current_method_stats().output_time += output_duration;

        cout << test_item << " has been proccessed." << endl;
        if (count != total && method == Method::COMPRESSED) {
            cout << "Estimated total doesn't equal calculated total. " << count << " != " << total << endl;
            status_log << "Estimated total doesn't equal calculated total. " << count << " != " << total << endl;
        }
        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been processed." << endl;
        status_log << unfiltered << " recipes were output." << endl;
        status_log << machine_filtered << " recipes were filtered due to number of machines." << endl;
        status_log << "Execution time: " << elapsed.count() << " seconds." << endl;
        status_log << "Output time: " << output_duration.count() << " seconds." << endl;
        status_log << endl;

        if (elapsed >= update_frequency) {
            cout << total << " combinations have been processed." << endl;
            cout << unfiltered << " recipes were output." << endl;
            cout << machine_filtered << " recipes were filtered due to number of machines." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            cout << "Output time: " << output_duration.count() << " seconds." << endl;
            cout << endl;
        }
    }
    auto total_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = (total_end - total_start);
    Stats::current_method_stats().total_time += total_elapsed;

    // results << "]" << endl;
    // results << recipe_root.dump(4);

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

    for (auto& item : filter_json) {
        test_item = item.value("ItemClass", "N/A");
        item["Depth"] = filter_map.at(test_item);
    }

    ofstream filter_out(exePath / "dat" / "100_combination_filter.json");
    filter_out << filter_json.dump(4);
    filter_out.close();

    // results.close();
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
            Stats::current_method_stats().merge_recipe_calls++;
            
            if(i == incrementor_values.back()) {
                rebuild_needed = true;
            }
        }
    }
    if (rebuild_needed) {
        Stats::current_method_stats().incrementor_rebuild_count++;
        auto incrementor_rebuild_start = chrono::steady_clock::now();
        size_t j = incrementor_values.back();
        incrementor_values.clear();
        const vector<Resource>& new_ingredients = recipes.at(item_name).at(incrementor.at(j)).get_ingredients_ref();
        for (size_t i = 0; i < new_ingredients.size(); i++) {
            item_name = new_ingredients[i].get_name();
            if (terminal_resources.find(item_name) == terminal_resources.end()) {
                incrementor_values.push_back(incrementor_map.at(item_name));
            }
        }
        incrementor_values.push_back(incrementor_map.at(product));
        sort(incrementor_values.begin(), incrementor_values.end());
        auto incrementor_rebuild_end = chrono::steady_clock::now();
        Stats::current_method_stats().incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    }
    auto incrementor_end = chrono::steady_clock::now();
    Stats::current_method_stats().incrementor_time += (incrementor_end - incrementor_start);
}



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
    Stats::current_method_stats().incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
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
        }
    }
    auto incrementor_end = chrono::steady_clock::now();
    Stats::current_method_stats().incrementor_time += (incrementor_end - incrementor_start);
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