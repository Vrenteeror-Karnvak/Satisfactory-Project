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

bool merge_ids(vector<int>& candidate_ID, const vector<int>& current_ID, const size_t m, MethodStats& stats);
void increment_incrementor(const vector<Recipe>& output_recipes, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes,
    vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log);
void increment_incrementor(const vector<Resource>& ingredients, const size_t product_ID, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes,
    const vector<char>& is_terminal, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log, vector<size_t>& incrementor_values);
void increment_incrementor(const vector<Resource>& ingredients, const size_t product_ID, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes, const vector<char>& is_terminal,
    vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log, vector<size_t>& incrementor_values, const vector<char>& is_nuclear);
bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<Recipe>& recipe_map, ofstream& status_log);

enum class Method {
    BASE,
    COMPRESSED,
    NUCLEAR
};

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    bool duplicate_found = false; // Triggers if a duplicate item is found in the incrementor

    auto true_start = chrono::steady_clock::now();

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    ofstream status_log(exePath / "dat" / "test_status.log");

    // The json file containing all recipes
    json recipe_root;
    recipe_in >> recipe_root;
    recipe_in.close();
    size_t m = 0; // index in recipe_root of the item

    // The json file containing the testing data
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    string test_item; // The item being processed
    size_t test_ID; // The ID of the item being processed

    // The auto terminate information
    size_t num_to_test = static_cast<size_t>(stoi(test_recipe_root[2].value("number_items_to_test", "1")) - 1); // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds update_frequency(stoi(test_recipe_root[3].value("update_frequency", "0"))); // the frequency the program updates its progress
    uint u = 1; // the number of updates

    // The filter information
    uint max_num_machines = test_recipe_root[4].value("max_num_machines", 200); // the maximum amount of product a recipe chain is allowed to have
    uint max_filter = test_recipe_root[4].value("max_filter", 200); // the maximum number of machines the filter is allowed to have under normal circumstances
    uint max_output = test_recipe_root[4].value("max_combinations", 1000); // the maximum number of combinations being output
    bool remake_filters = test_recipe_root[4].value("remake_filters", false); // whether or not to remake the filter
    bool filter_made = false; // has the filter for the item already been made?
    uint64_t number_of_machines = 0;
    test_recipe_in.close();


    // Sets up the ID system
    m = 0;
    size_t number_unique_resources;
    size_t number_of_items;
    unordered_map<string, size_t> resource_ID_map; // Translates a resource into an ID
    {
        ifstream base_recipe_in(exePath / "dat" / "base_resources.json");
        json base_root;
        base_recipe_in >> base_root;
        base_recipe_in.close();
        for (const auto& item : recipe_root) {
            resource_ID_map.insert({item["Category"], m});
            m += 1;
        }
        for (const auto& item : base_root) {
            resource_ID_map.insert({item["ItemClass"], m});
            m += 1;
        }
        number_unique_resources = resource_ID_map.size();
        number_of_items = recipe_root.size();
    }

    // The variables needed to increment through all the recipe combinations
    vector<Recipe> recipe_map(number_of_items);
    vector<size_t> incrementor(number_of_items, 0);
    vector<size_t> incrementor_max(number_of_items, 0);
    vector<size_t> incrementor_values;
    incrementor_values.reserve(20);
    vector<size_t> all_zeros(number_of_items, 0);
    vector<vector<Recipe>> base_recipes(number_of_items); // holds all the base recipes
    vector<vector<Recipe>> compressed_recipes(number_of_items); // holds all the compressed recipes
    bool first_creation = true; // is this the first time incrementor_values has been made?



    vector<char> is_terminal(number_unique_resources, false); // is the item terminal?
    vector<char> is_nuclear(number_of_items, false); // is the item nuclear?
    vector<char> is_capstone(number_of_items, false); // is the item a capstone?
    {
        // The json file containing the terminal resources
        ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
        json terminal_root;
        terminal_recipe_in >> terminal_root;
        terminal_recipe_in.close();
        // creates an unordered map of all terminal resources
        for (const auto& terminal : terminal_root) {
            is_terminal[resource_ID_map.at(terminal["ItemClass"])] = true;
        }

        ifstream nuclear_recipe_in(exePath / "dat" / "nuclear_resources.json");
        json nuclear_root;
        nuclear_recipe_in >> nuclear_root;
        nuclear_recipe_in.close();
        // creates an unordered map of all nuclear resources
        for (const auto& nuclear : nuclear_root) {
            is_nuclear[resource_ID_map.at(nuclear["ItemClass"])] = true;
        }

        ifstream capstone_recipe_in(exePath / "dat" / "capstone_resources.json");
        json capstone_root;
        capstone_recipe_in >> capstone_root;
        capstone_recipe_in.close();
        // creates an unordered map of all nuclear resources
        for (const auto& capstone : capstone_root) {
            is_capstone[resource_ID_map.at(capstone["ItemClass"])] = true;
        }
    }

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain

    // The variables used to output the data
    vector<Recipe> output_vector;
    output_vector.reserve(20);
    string product_name; // the name of the item being processed

    // Status tracking variables
    uint64_t total = 0; // the number of unique recipe chains found for the current item
    uint64_t true_total = 0; // the total number of unique recipe chains found across all items
    uint64_t unfiltered = 0; // the number of recipe chains filtered out for the current item
    uint64_t true_unfiltered = 0; // the number of recipe chains filtered out across all items
    uint64_t machine_filtered = 0; // the number of recipe chains filtered out for the current item due to the number of machines
    uint64_t true_machine_filtered = 0; // the number of recipe chains filtered out due to the number of machines across all items

    // creates a map of the filters
    ifstream filters_in(exePath / "dat" / "item_filters.json");
    json filter_json;
    filters_in >> filter_json;
    vector<uint> filters(number_of_items, 0);
    if (!remake_filters) {
        for (size_t i = 0; i < number_of_items; i++) {
            filters[i] = filter_json[i].value("Amount", 0);
        }
    }
    filters_in.close();

    m = 0;
    for (const auto& data : recipe_root) {
        // adds the first recipe of all items to recipe_list and creates the incrementors
        // also builds the recipe map and other things
        output_recipes.clear();
        Recipe recipe_input;
        incrementor_max[m] = data["Data"].size();

        for (const auto& recipe : data["Data"]) {
            recipe_input.set_recipe(recipe, m);
            for (Resource& ingredient : recipe_input.modify_ingredients()) {
                ingredient.set_product_ID(resource_ID_map.at(ingredient.get_name()));
            }
            for (Resource& product : recipe_input.modify_products()) {
                product.set_product_ID(resource_ID_map.at(product.get_name()));
            }
            output_recipes.push_back(recipe_input);
        }

        base_recipes[m] = output_recipes;
        compressed_recipes[m] = output_recipes;

        recipe_input.set_recipe(data["Data"].at(0), m);
        for (Resource& ingredient : recipe_input.modify_ingredients()) {
            ingredient.set_product_ID(resource_ID_map.at(ingredient.get_name()));
        }
        for (Resource& product : recipe_input.modify_products()) {
            product.set_product_ID(resource_ID_map.at(product.get_name()));
        }
        recipe_map[m] = recipe_input;

        m += 1;
    }

    //
    // Update the variable names to match the new system
    // Set up a seperate program to test various speeds on this computer
    //

    for (size_t k = 0; k < number_of_items; k++) {
        auto start = chrono::steady_clock::now(); // starts the timer

        // Clears termination flags and debug variables
        first_creation = true;
        u = 1;

        // clears the output storage vectors
        output_vector.clear();

        // Sets the item being processed
        test_item = recipe_root[k]["Category"];
        test_ID = k;

        Method method;
        if (k == (number_of_items - 1)) {
            method = Method::NUCLEAR;
        }
        else if (is_nuclear[k]) {
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
        const vector<vector<Recipe>>* recipes_ptr = (method == Method::BASE) ? &base_recipes : &compressed_recipes;
        Stats::active_method_stats = (method == Method::COMPRESSED) ? &Stats::compressed : (method == Method::BASE) ? &Stats::base : &Stats::nuclear;
        MethodStats& stats = Stats::current_method_stats();

        if (!remake_filters && filters[test_ID] != 0) {
            filter_made = true;
        }

        for (size_t i = 0; i < k; i++) {
            incrementor_max[i] = (*recipes_ptr)[i].size();
            recipe_map[i] = (*recipes_ptr)[i][incrementor[i]];
        }

        unfiltered = 0;
        machine_filtered = 0;
        total = 0;

        auto count_start = chrono::steady_clock::now();
        uint64_t count = 0; // The estimated number of combinations needing to be processed for the current item
        status_log << test_item << " about to be processed." << endl;
        if (method == Method::COMPRESSED) {
            for (size_t i = 0; i < (*recipes_ptr)[test_ID].size(); i++) {
                const vector<Resource>& ingredient_vector = (*recipes_ptr)[test_ID][i].get_ingredients_ref();
                uint64_t temp_count = 1;
                for (size_t j = 0; j < ingredient_vector.size(); j++) {
                    size_t temp_ID = ingredient_vector[j].get_product_ID();
                    if (!is_terminal[temp_ID]) {
                        temp_count *= (*recipes_ptr)[temp_ID].size();
                    }
                }
                count += temp_count;
            }
            status_log << "Estimated to need to process " << count << " combinations to complete." << endl;
        }
        else if (method == Method::NUCLEAR) {
            vector<uint64_t> theoretical_count; // the number of times the loop needs to run for an item
            for (const Recipe& data : recipe_map) {
                size_t current_ID = data.get_product_ID();
                if (!is_nuclear[current_ID]) {
                    theoretical_count.push_back((*recipes_ptr)[current_ID].size());
                }
                else {
                    count = 0;
                    const vector<Recipe>& current_recipes = (*recipes_ptr)[current_ID];
                    for (size_t i = 0; i < current_recipes.size(); i++) {
                        const vector<Resource>& ingredient_vector = current_recipes[i].get_ingredients_ref();
                        uint64_t temp_count = 1;
                        for (size_t j = 0; j < ingredient_vector.size(); j++) {
                            size_t temp_ID = ingredient_vector[j].get_product_ID();
                            if (temp_ID == resource_ID_map.at("Uranium Waste") && current_ID == resource_ID_map.at("Plutonium Pellet")) {
                                // Uranium waste always appears twice here, so one is skipped
                                continue;
                            }
                            else if (!is_terminal[temp_ID]) {
                                temp_count *= theoretical_count[ingredient_vector[j].get_product_ID()];
                            }
                        }
                        count += temp_count;
                    }
                    theoretical_count.push_back(count);
                }
            }
            count = theoretical_count.back();
            status_log << "Estimated to need to process " << count << " combinations to complete." << endl;
        }
        else {
            status_log << "Can not estimate number of combinations." << endl;
        }
        auto count_end = chrono::steady_clock::now();
        stats.count_time += (count_end - count_start);

        stats.item_count += 1;

        if ((k - 1) == num_to_test) {
            status_log << endl;
            break;
        }

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            // clears the output storage vectors
            output_recipes.clear();

            // The starting recipe in the chain
            const Recipe& starting_recipe = (*recipes_ptr)[test_ID][incrementor[test_ID]];

            stats.combinations_processed++;
            // Merge the ID's to detect any conflicts
            vector<int> candidate_ID(number_of_items, -1);
            vector<Resource> ingredients;
            if (method == Method::NUCLEAR) {
                unordered_set<size_t> item_IDs;
                for (size_t i = 0; i < is_nuclear.size(); i++) {
                    if (is_nuclear[i]) {
                        for (const Resource& ingredient : recipe_map[i].get_ingredients_ref()) {
                            if (item_IDs.insert(ingredient.get_product_ID()).second) {
                                ingredients.push_back(ingredient);
                            }
                        }
                    }
                }
            }
            else {
                ingredients = starting_recipe.get_ingredients();
            }

            if (method != Method::BASE) {
                bool valid = true;

                if (first_creation) {
                    auto incrementor_rebuild_start = chrono::steady_clock::now();
                    stats.incrementor_rebuild_count++;
                    incrementor_values.clear();
                    for (size_t i = 0; i < ingredients.size(); i++) {
                        size_t item_ID = ingredients[i].get_product_ID();
                        if (!is_terminal[item_ID]) {
                            incrementor_values.push_back(item_ID);
                        }
                    }
                    incrementor_values.push_back(test_ID);
                    sort(incrementor_values.begin(), incrementor_values.end());
                    first_creation = false;
                    auto incrementor_rebuild_end = chrono::steady_clock::now();
                    stats.incrementor_time += (incrementor_rebuild_end - incrementor_rebuild_start);
                    stats.incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
                }

                auto ID_start = chrono::steady_clock::now();
                auto ID_build_start = chrono::steady_clock::now();

                for (size_t i = 0; i < ingredients.size(); i++) {
                    valid = true;
                    const Resource& item = ingredients[i];
                    size_t product_ID = item.get_product_ID();
                    if (!is_terminal[product_ID]) {
                        const Recipe& current_recipe = (*recipes_ptr)[product_ID][incrementor[product_ID]];
                        if (!current_recipe.get_ID_ref().empty()) {
                            if (i == 0) {
                                candidate_ID = current_recipe.get_ID();
                            }
                            else if (!merge_ids(candidate_ID, current_recipe.get_ID_ref(), product_ID, stats)) {
                                valid = false;
                                break;
                            }
                        }
                        else if (is_nuclear[product_ID]) {
                            continue;
                        }
                        else {
                            status_log << current_recipe.get_name() << " has no ID." << endl;
                        }
                    }
                }
                if (valid) {
                    if (method == Method::NUCLEAR) {
                        for (size_t i = 0; i < is_nuclear.size(); i++) {
                            if (is_nuclear[i]) {
                                candidate_ID[i] = incrementor[i];
                            }
                        }
                    }
                    candidate_ID[test_ID] = incrementor[test_ID];
                }

                auto ID_build_end = chrono::steady_clock::now();
                stats.ID_build_time += (ID_build_end - ID_build_start);

                if (!valid) {
                    stats.ID_filtered++;
                    machine_filtered += 1;
                    total += 1;

                    auto ID_end = chrono::steady_clock::now();
                    stats.ID_total_time += (ID_end - ID_start);
                    if (method == Method::COMPRESSED) {     // Runs the compressed incrementor
                        increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values);
                    }
                    else {                                  // Runs the nuclear incrementor
                        increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values, is_nuclear);
                    }
                    continue;
                }

                // Collects data on the IDs if they pass the test
                uint64_t current_ID_entries = 0;
                for (size_t i = 0; i < candidate_ID.size(); i++) {
                    if (candidate_ID[i] != -1) {
                        current_ID_entries++;
                    }
                }
                stats.total_ID_entries_used += current_ID_entries;
                stats.max_ID_entries_used = max(stats.max_ID_entries_used, current_ID_entries);

                auto ID_end = chrono::steady_clock::now();
                stats.ID_total_time += (ID_end - ID_start);
            }

            stack<Recipe> recipe_stack; // the stack of recipes in the chain
            stats.chains_generated++;
            recipe_stack.push(starting_recipe);
            
            // Creates the recipe chain based on the provided recipes
            auto chain_start = chrono::steady_clock::now();
            while (!recipe_stack.empty()) {
                Recipe& current_recipe = recipe_stack.top();
                if (current_recipe.is_processed()) {
                    // if the current recipe has already been processed, remove it from the stack
                    bool already_added = false; // marks if a recipe is already in the vector
                    size_t location = 0; // the location of the identical recipe in the vector

                    for (size_t i = 0; i < output_recipes.size(); i++) {
                        if (output_recipes[i].same_product_ID(current_recipe)) {
                            already_added = true;
                            location = i;
                            break;
                        }
                    }

                    if (already_added) {
                        output_recipes[location] += current_recipe;
                    }
                    else {
                        output_recipes.push_back(current_recipe);
                    }
                    recipe_stack.pop();
                    continue;
                }
                else {
                    // if the current recipe has not been processed, process it
                    current_recipe.set_processed(); // sets the processed flag to true
                    const vector<Resource>& stack_ingredients = current_recipe.get_ingredients_ref(); // gets the ingredients of the current recipe
                    for (size_t i = 0; i < stack_ingredients.size(); i++) { // increments through all the ingredients
                        const Resource& ingredient = stack_ingredients[i];
                        size_t product_ID = ingredient.get_product_ID();
                        if (is_terminal[product_ID]) {
                            // if the ingredient was terminal moves on to the next one
                            continue;
                        }

                        Recipe new_recipe = recipe_map[product_ID]; // sets new_recipe to the recipe found
                        new_recipe.set_to(ingredient.get_amount()); // changes the recipe product to match the ingredient it is for
                        recipe_stack.push(new_recipe); // adds the new recipe to the stack
                    }
                }
            }
            auto chain_end = chrono::steady_clock::now();
            stats.chain_generation_time += (chain_end - chain_start);

            auto lcm_start = chrono::steady_clock::now();
            Fraction rate;
            rate = (output_recipes.back().get_product_ref(0).get_amount() / recipe_map[test_ID].get_product_ref(0).get_amount());
            rate /= 60;
            rate *= recipe_map[test_ID].get_machine_speed();
            Fraction test_number_of_machines = rate;
            bool invalid = false; // Is the combination already invalid?

            uint64_t speed_lm = 1;
            size_t product_ID = 0;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                const Resource& product = output_recipes[i].get_product_ref(0);
                // size_t product_ID = product.get_product_ID();
                product_ID = product.get_product_ID();
                rate = (product.get_amount() / recipe_map[product_ID].get_product_ref(0).get_amount());
                rate /= 60;
                rate *= recipe_map[product_ID].get_machine_speed();
                speed_lm = lcm(speed_lm, rate.get_denominator());
                stats.lcm_calls++;

                Fraction temp_number_of_machines = test_number_of_machines * speed_lm;
                if (temp_number_of_machines > max_num_machines || (temp_number_of_machines > filters[test_ID] && filters[test_ID] != 0)) {
                    invalid = true;
                    break;
                }
            }

            test_number_of_machines *= speed_lm;
            stats.max_machine_count = max(stats.max_machine_count, (static_cast<double>(test_number_of_machines.get_numerator()) / test_number_of_machines.get_denominator()));

            if (invalid || test_number_of_machines > max_num_machines || (test_number_of_machines > filters[test_ID] && filters[test_ID] != 0)) {
                // (test_number_of_machines > max_num_machines) || (test_number_of_machines > filters[test_ID] && filters[test_ID] != 0)
                stats.speed_filtered++;
                machine_filtered += 1;
                total += 1;

                auto lcm_end = chrono::steady_clock::now();
                stats.lcm_time += (lcm_end - lcm_start);
                if (method == Method::COMPRESSED) {     // Runs the compressed incrementor
                    increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values);
                }
                else if (method == Method::BASE) {      // Runs the base incrementor
                    increment_incrementor(output_recipes, recipe_map, (*recipes_ptr), incrementor, incrementor_max, status_log);
                }
                else {                                  // Runs the nuclear incrementor
                    increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values, is_nuclear);
                }
                continue;
            }
            stats.max_speed_lm = max(stats.max_speed_lm, speed_lm);

            auto lcm_end = chrono::steady_clock::now();
            stats.lcm_time += (lcm_end - lcm_start);

            // converts the output vector into compressed json
            Recipe output;
            auto merge_start = chrono::steady_clock::now();
            if (method == Method::BASE) {
                for (size_t i = 0; i < output_recipes.size(); i++) {
                    size_t j = output_recipes[i].get_product_ID();
                    candidate_ID[j] = incrementor[j];
                }
            }
            output.merge_recipes(output_recipes);
            output.set_product_ID(test_ID);
            output.set_ID(candidate_ID);
            output.set_name(test_item);
            output *= speed_lm;
            auto merge_end = chrono::steady_clock::now();
            stats.merge_time += (merge_end - merge_start);

            int64_t item_lm = 1;
            for (size_t i = 0; i < output.get_ingredients().size(); i++) {
                item_lm = lcm(item_lm, output.get_ingredient(i).get_amount().get_denominator());
                stats.lcm_calls++;
            }
            for (size_t i = 0; i < output.get_products().size(); i++) {
                item_lm = lcm(item_lm, output.get_product(i).get_amount().get_denominator());
                stats.lcm_calls++;
            }

            if (item_lm == 1) {
                stats.item_lm_1++;
            }
            else {
                stats.item_lm_over_1++;
            }
            output *= item_lm;

            rate = (output.get_product_ref(0).get_amount() / recipe_map[test_ID].get_product_ref(0).get_amount());
            rate /= 60;
            rate *= recipe_map[test_ID].get_machine_speed();
            number_of_machines = rate.get_numerator();
            output.set_machine_speed(number_of_machines);

            // Checks if the total number of machines is more than the maximum and doesn't add it if it is
            if ((number_of_machines <= max_num_machines) && (number_of_machines <= filters[test_ID] || filters[test_ID] == 0)) {
                // if the recipe is valid, adds it to the output
                output_vector.push_back(output);

                unfiltered += 1;
                total += 1;
            }
            else {
                // if the recipe is not valid, removes it
                stats.merge_filtered++;
                machine_filtered += 1;
                total += 1;
            }

            if (method == Method::COMPRESSED) {     // Runs the compressed incrementor
                increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values);
            }
            else if (method == Method::BASE) {      // Runs the base incrementor
                increment_incrementor(output_recipes, recipe_map, (*recipes_ptr), incrementor, incrementor_max, status_log);
            }
            else {                                  // Runs the nuclear incrementor
                increment_incrementor(ingredients, test_ID, recipe_map, (*recipes_ptr), is_terminal, incrementor, incrementor_max, status_log, incrementor_values, is_nuclear);
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
            if (total >= 10000000 && test_item == "Ficsonium Fuel Rod") {
                break;
            }
        } while (!duplicate_found && incrementor != all_zeros);

        if (duplicate_found) {
            break;
        }

        if ((chrono::steady_clock::now() - start) >= update_frequency) {
            auto end = chrono::steady_clock::now();
            chrono::duration<double> elapsed = end - start;
            cout << test_item << " was proccessed." << endl;
            cout << total << " combinations were found." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            cout << endl;
        }
        
        if (!filter_made) {
            Recipe maximum;
            if (output_vector.size() > max_output) {
                partial_sort(output_vector.begin(), output_vector.begin() + max_output, output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
                maximum = output_vector[max_output - 1];
            }
            else {
                sort(output_vector.begin(), output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
                maximum = output_vector.back();
            }
            
            // checks if the maximum is less than 10. Skips the creation process if it is.
            if (maximum.get_machine_speed() <= 10) {
                filters[test_ID] = 10;
            }
            else if (output_vector.at(0).get_machine_speed() > max_filter) {
                filters[test_ID] = output_vector.at(0).get_machine_speed();
            }
            else if (filters[test_ID] > max_filter) {
                filters[test_ID] = max_filter;
            }
            else {
                filters[test_ID] = maximum.get_machine_speed();
            }

            cout << test_item << " filter has been created." << endl;
            status_log << test_item << " filter has been created." << endl;

            // Outputs the new filter
            m = 0;
            for (auto& item : filter_json) {
                item["Amount"] = filters[m];
                m += 1;
            }

            ofstream filter_out(exePath / "dat" / "item_filters.json");
            filter_out << filter_json.dump(4);
            filter_out.close();

            filter_made = true;
            k--;
            continue;
        }
        else {
            filter_made = false;
        }
        
        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;

        auto pre_output = chrono::steady_clock::now();

        // preps the array to be output
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_vector[i].set_machine_speed(60.0);
        }
        
        if (!is_capstone[test_ID]) {
            compressed_recipes[test_ID] = output_vector;
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

        auto post_output = chrono::steady_clock::now();
        chrono::duration<double> output_duration = post_output - pre_output;
        stats.output_time += output_duration;

        cout << test_item << " has been proccessed." << endl;
        if (count != total && method != Method::BASE) {
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

        true_machine_filtered += machine_filtered;
        true_unfiltered += unfiltered;
        true_total += total;

        end = chrono::steady_clock::now();
        stats.total_time += end - start;
    }

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

    cout << endl << endl << endl;
    cout << true_total << " combinations were processed." << endl;
    cout << "Execution time: " << true_elapsed.count() << " seconds." << endl;
    status_log << endl << endl << endl;
    status_log << true_total << " combinations were processed." << endl;
    status_log << "Execution time: " << true_elapsed.count() << " seconds." << endl;

    m = 0;
    for (auto& item : filter_json) {
        item["Amount"] = filters[m];
        m += 1;
    }

    ofstream filter_out(exePath / "dat" / "item_filters.json");
    filter_out << filter_json.dump(4);
    filter_out.close();

    status_log.close();

    if (duplicate_found) {
        cout << "A duplicate was found in the incrementor. Program terminated while processing " << test_item << "." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}



bool merge_ids(vector<int>& a, const vector<int>& b, const size_t m, MethodStats& stats) {
    stats.merge_ID_calls++;
    // Only runs from index 0 to the index of the item that b represents
    // All values in b after that index are guarenteed -1
    for (size_t i = 0; i <= m; i++) {
        if (b[i] == -1) {
            // If b = -1, do nothing as a holds the correct value.
            continue;
        }
        else if (a[i] == -1) {
            // If a = -1 replace a with b as b holds the correct value
            a[i] = b[i];
        }
        else if (a[i] != b[i]) {
            // If a and b do not equal each other and don't equal -1, the ID's can not be merged and the merging was unsuccessful
            return false;
        }
    }
    // If the for loop completes, a holds the merged ID and the merging was successful
    return true;
}



// Base Method Incrementor
void increment_incrementor(const vector<Recipe>& output_recipes, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes,
    vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log) {
    MethodStats& stats = Stats::current_method_stats();
    auto incrementor_start = chrono::steady_clock::now();
    // increments the incrementor vector 
    string item_name;
    vector<size_t> incrementor_values;
    stats.incrementor_rebuild_count++;
    auto incrementor_rebuild_start = chrono::steady_clock::now();
    for (size_t i = 0; i < output_recipes.size(); i++) {
        incrementor_values.push_back(output_recipes[i].get_product_ID());
    }
    sort(incrementor_values.begin(), incrementor_values.end());
    auto incrementor_rebuild_end = chrono::steady_clock::now();
    stats.incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);

    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, recipe_map, status_log);
    
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor[i] += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            recipe_map[i] = recipes[i][incrementor[i]];
        }
        else {
            break;
        }
    }
    auto incrementor_end = chrono::steady_clock::now();
    stats.incrementor_time += (incrementor_end - incrementor_start);
}



// Compressed Method Incrementor
void increment_incrementor(const vector<Resource>& ingredients, const size_t product_ID, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes,
    const vector<char>& is_terminal, vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log, vector<size_t>& incrementor_values) {
    MethodStats& stats = Stats::current_method_stats();
    auto incrementor_start = chrono::steady_clock::now();

    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, recipe_map, status_log);
    
    // increments the incrementor vector
    bool rebuild_needed = false;
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor[i] += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            recipe_map[i] = recipes[i][incrementor[i]];
            
            if (i == incrementor_values.back()) {
                rebuild_needed = true;
            }
        }
    }
    if (rebuild_needed) {
        stats.incrementor_rebuild_count++;
        auto incrementor_rebuild_start = chrono::steady_clock::now();
        size_t j = incrementor_values.back();
        size_t item_ID = 0;
        incrementor_values.clear();

        const vector<Resource>& new_ingredients = recipes[j][incrementor[j]].get_ingredients_ref();
        for (size_t i = 0; i < new_ingredients.size(); i++) {
            item_ID = new_ingredients[i].get_product_ID();
            if (!is_terminal[item_ID]) {
                incrementor_values.push_back(item_ID);
            }
        }
        incrementor_values.push_back(product_ID);
        sort(incrementor_values.begin(), incrementor_values.end());
        auto incrementor_rebuild_end = chrono::steady_clock::now();
        stats.incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    }
    auto incrementor_end = chrono::steady_clock::now();
    stats.incrementor_time += (incrementor_end - incrementor_start);
}



// Nuclear Method Incrementor
void increment_incrementor(const vector<Resource>& ingredients, const size_t product_ID, vector<Recipe>& recipe_map, const vector<vector<Recipe>>& recipes, const vector<char>& is_terminal,
    vector<size_t>& incrementor, const vector<size_t>& incrementor_max, ofstream& status_log, vector<size_t>& incrementor_values, const vector<char>& is_nuclear) {
    MethodStats& stats = Stats::current_method_stats();
    auto incrementor_start = chrono::steady_clock::now();

    // duplicate_found = check_duplicate_incrementor_values(incrementor_values, recipe_map, status_log);

    // increments the incrementor vector
    bool rebuild_needed = false;
    bool increment = true; // Determines if the value should be incremented
    for (size_t j = 0; j < incrementor_values.size(); j++) {
        size_t i = incrementor_values[j];
        // if the value needs to be incremented, add one to it
        if (increment) {
            incrementor[i] += 1;
            increment = false;
            // if the value has reached its maximum, set it to zero and set to increment the next value
            if (incrementor[i] >= incrementor_max[i]) {
                incrementor[i] = 0;
                increment = true;
            }

            recipe_map[i] = recipes[i][incrementor[i]];

            if (is_nuclear[i]) {
                rebuild_needed = true;
            }
        }
    }
    if (rebuild_needed) {
        stats.incrementor_rebuild_count++;
        auto incrementor_rebuild_start = chrono::steady_clock::now();
        size_t item_ID = 0;
        incrementor_values.clear();

        vector<Resource> new_ingredients;
        unordered_set<size_t> item_IDs;
        for (size_t i = 0; i < is_nuclear.size(); i++) {
            if (is_nuclear[i]) {
                for (const Resource& ingredient : recipe_map[i].get_ingredients_ref()) {
                    if (item_IDs.insert(ingredient.get_product_ID()).second) {
                        new_ingredients.push_back(ingredient);
                    }
                }
            }
        }
        for (size_t i = 0; i < new_ingredients.size(); i++) {
            item_ID = new_ingredients[i].get_product_ID();
            if (!is_terminal[item_ID]) {
                incrementor_values.push_back(item_ID);
            }
        }
        incrementor_values.push_back(product_ID);
        sort(incrementor_values.begin(), incrementor_values.end());
        auto incrementor_rebuild_end = chrono::steady_clock::now();
        stats.incrementor_rebuild_time += (incrementor_rebuild_end - incrementor_rebuild_start);
    }
    auto incrementor_end = chrono::steady_clock::now();
    stats.incrementor_time += (incrementor_end - incrementor_start);
}



bool check_duplicate_incrementor_values(const vector<size_t>& incrementor_values, const vector<Recipe>& recipe_map, ofstream& status_log) {
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
        vector<string> incrementor_products(incrementor_values.size());
        for (size_t j = 0; j < incrementor_values.size(); j++) {
            incrementor_products[j] = recipe_map[incrementor_values[j]].get_product(0).get_name();
        }
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
            cerr << "   Recipe " << j << ": product(0) = '" << incrementor_products[j] << "' index = " << incrementor_values[j] << endl;
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
            status_log << "   Recipe " << j << ": product(0) = '" << incrementor_products[j] << "' index = " << incrementor_values[j] << endl;
        }
        status_log << endl;
    }

    return duplicate_found;
}