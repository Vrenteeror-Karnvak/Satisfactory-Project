#include "../lib/json.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include <chrono>

#include "../resources/recipe.h"
#include "../resources/resource.h"
#include "../resources/fraction.h"
#include "../resources/dominance.h"

using namespace std;
using json = nlohmann::ordered_json;

/**
 * @brief Creates and compresses a recipe chain. Places the compressed chain into sorting_map.
 *
 * @param sorting_map       All created recipe chains for a given item
 * @param filter_map        A map of all the filter values
 * @param recipe_map        A map of all current recipes
 * @param terminal_map      A map of all terminal resources
 * @param incrementor       The vector of all incrementor values
 * @param test_item         The current item being processed
 * @param max_product       The absolute maximum number of machines a recipe can need
 * @return An uncompressed version of the recipe chain. A vector of the recipes in the chain.
 */
vector<Recipe> build_chain(map<DominanceKey, vector<Recipe>>& sorting_map, const unordered_map<string, Recipe> recipe_map, const unordered_map<string, int> filter_map, const unordered_map<string, Resource>& terminal_map, const vector<size_t>& incrementor, const string test_item, const int max_product);

// Status tracking variables
int total = 0; // the number of unique recipe chains found for the current item
int true_total = 0; // the total number of unique recipe chains found across all items
int unfiltered = 0; // the number of recipe chains filtered out for the current item
int true_unfiltered = 0; // the number of recipe chains filtered out across all items
int machine_filtered = 0; // the number of recipe chains filtered out for the current item due to the number of machines
int dominance_filtered = 0; // the number of recipe chains filtered out during the dominance filtering
int true_machine_filtered = 0; // the number of recipe chains filtered out due to the number of machines across all items
int true_dominance_filtered = 0; // the number of recipe chains filtered out during the dominance filtering across all items
int expected_count = 0; // the number of times the loop has run for the current item


/*
Expected Output:
110061 combinations were processed.
1024 recipes were output.
108985 recipes were filtered due to number of machines.
52 recipes were filtered during the dominance filtering.
*/

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    bool issue_occured = false; // Triggers if a duplicate item is found in the incrementor

    // opens the filestreams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    ifstream terminal_recipe_in(exePath / "dat" / "terminal_resources.json");
    ofstream results(exePath / "dat" / "100_test_results.json");
    ofstream status_log(exePath / "dat" / "backup_test_status.log");

    // The json file containing all recipes as well as the variables needed to increment through them
    json recipe_root;
    recipe_in >> recipe_root;
    Recipe recipe_input;
    unordered_map<string, Recipe> recipe_map;
    vector<size_t> incrementor;
    vector<size_t> incrementor_max;
    vector<size_t> all_zeros(recipe_root.size(), 0);
    unordered_map<string, size_t> incrementor_map; // the location of the incrementor for a given product inside of the incrementor vector
    unordered_map<string, vector<Recipe>> recipes; // holds all the recipes
    size_t m = 0;

    // The json file containing the recipe or item
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    Recipe test_recipe(test_recipe_root.at(0)); // Use to inject a RECIPE into the system
    string test_item = test_recipe_root.at(1).value("ItemClass", ""); // Use to inject an ITEM into the system

    // The auto terminate information
    size_t num_to_test = static_cast<size_t>(stoi(test_recipe_root.at(2).value("number_items_to_test", "0")) - 1); // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds update_frequency(stoi(test_recipe_root.at(3).value("update_frequency", "0"))); // the frequency the program updates its progress

    // The filter information
    int max_product = test_recipe_root.at(4).value("max_product", 1000); // the maximum amount of product a recipe chain is allowed to have
    int max_filter = test_recipe_root.at(4).value("max_filter", 200); // the maximum number of machines the filter is allowed to have under normal circumstances
    unsigned int max_output = test_recipe_root.at(4).value("max_combinations", 1000); // the maximum number of combinations being output
    unsigned int absolute_max_output = test_recipe_root.at(4).value("absolute_max_combinations", 10000);
    const unsigned int base_max_output = max_output;
    bool remake_filters = test_recipe_root.at(4).value("remake_filters", false); // whether or not to remake the filter
    bool filter_made = false; // has the filter for the item already been made?
    bool redo_filter = false; // does the filter need to be redone to futher reduce the output?
    map<DominanceKey, vector<Recipe>> sorting_map; // the map containing the sorted outputs ready for dominance filtering

    // The json file containing the terminal resources
    json terminal_root;
    terminal_recipe_in >> terminal_root;
    Resource terminal_resource;
    unordered_map<string, Resource> terminal_map;

    // The variables that the stack uses to increment through all nodes
    Resource ingredient; // the current ingredient being added
    Recipe new_recipe; // the recipe being added to the stack for non terminal resources
    Recipe terminal_recipe; // the recipe being added to the stack for terminal resources

    // The variables that the stack uses to record the data
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain

    // The variables used to output the data
    json chain_object = json::object();
    json output_chain = json::object(); // the current recipe chain being processed
    json output_object = json::object(); // the json object containing all recipe chains being sent to the file
    vector<Recipe> output_vector;
    json output_array = json::array(); // the recipes being output into the file
    bool first = true; // is this the first item being output in the given array?

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

    // creates an unordered map of all terminal resources
    for (const auto& terminal : terminal_root) {
        terminal_resource.set_resource(terminal);
        terminal_map.insert({terminal_resource.get_name(), terminal_resource});
    }

    for (const auto& data : recipe_root) {
        // adds the first recipe of all items to recipe_list and creates the incrementors
        // also builds the recipe map
        incrementor_max.push_back(data.value("Data", empty_array).size());
        incrementor.push_back(0);

        for (const auto& recipe : data["Data"]) {
            recipe_input.set_recipe(recipe);
            output_recipes.push_back(recipe_input);
        }

        incrementor_map.insert({data.value("Category", ""), m});
        recipes.insert({data.value("Category", ""), output_recipes});

        recipe_input.set_recipe(data.value("Data", empty_array).at(0));
        recipe_map.insert({data.value("Category", ""), recipe_input});
        output_recipes.clear();
        m += 1;
    }

    //
    // Nothing right now
    //

    results << "[" << endl;

    chrono::duration<double> increment_elapsed;
    
    for (size_t k = 0; k < recipe_root.size(); k++) {
        auto start = chrono::steady_clock::now(); // starts the timer

        // clears the output storage vectors
        output_vector.clear();
        sorting_map.clear();

        // Sets the item being processed
        test_item = recipe_root.at(k).value("Category", "");

        if (!remake_filters && filter_map.at(test_item) != 0) {
            filter_made = true;
        }
        
        if (redo_filter) {
            filter_made = false;
            redo_filter = false;
        }

        expected_count = 0;
        unfiltered = 0;
        machine_filtered = 0;
        dominance_filtered = 0;
        total = 0;

        for (size_t i = 0; i < recipes.at(test_item).size(); i++) {
            vector<Resource> recipe_vector = recipes.at(test_item).at(i).get_ingredients();
            int temp_count = 1;
            string temp_item;

            for (size_t j = 0; j < recipe_vector.size(); j++) {
                temp_item = recipe_vector.at(j).get_name();
                if (recipes.find(temp_item) != recipes.end()) {
                    temp_count *= recipes.at(temp_item).size();
                }
            }
            expected_count += temp_count;
        }

        status_log << test_item << " about to be processed." << endl;
        status_log << "Estimated to need to process " << expected_count << " combinations to complete." << endl;

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            /* Threading stuff done here */
            // Function takes an average of 250 micro seconds to complete
            // Incrementor takes an average of 7 micro seconds to complete
            output_recipes = build_chain(sorting_map, recipe_map, filter_map, terminal_map, incrementor, test_item, max_product);

            // increments the incrementor vector
            vector<size_t> incrementor_values;
            vector<string> incrementor_products;
            string product_name;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                product_name = output_recipes.at(i).get_product(0).get_name();
                incrementor_values.push_back(incrementor_map[product_name]);
                incrementor_products.push_back(product_name);
            }
            sort(incrementor_values.begin(), incrementor_values.end());
            bool increment = true; // Determines if the value should be incremented
            for (size_t j = 0; j < incrementor_values.size(); j++) {
                size_t i = incrementor_values.at(j);
                // if the value needs to be incremented, add one to it
                if (increment) {
                    incrementor.at(i) += 1;
                    increment = false;
                }
                // if the value has reached its maximum, set it to zero and set to increment the next value
                if (incrementor.at(i) >= incrementor_max.at(i)) {
                    incrementor.at(i) = 0;
                    increment = true;
                }

                product_name = recipe_root.at(i).value("Category", "");
                recipe_map[product_name] = recipes.at(product_name).at(incrementor.at(i));

                if (!increment) {
                    break;
                }
            }
            
            // If the last value reached its maximum
            // Set all incrementor values to 0 to end the while loop
            if (increment) {
                for (size_t j = 0; j < incrementor.size(); j++) {
                    product_name = recipe_root.at(j).value("Category", "");
                    recipe_map[product_name] = recipes.at(product_name).at(0);
                }
            }
        } while (!issue_occured && incrementor != all_zeros);

        if ((chrono::steady_clock::now() - start) >= update_frequency) {  
            auto creation_end = chrono::steady_clock::now();
            chrono::duration<double> creation_elapsed = creation_end - start;
            cout << test_item << " was proccessed." << endl;
            cout << total << " combinations were found." << endl;
            cout << "Execution time: " << creation_elapsed.count() << " seconds." << endl;
            cout << endl;
        }
        
        auto result = max_element(output_vector.begin(), output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
        auto minimum = min_element(output_vector.begin(), output_vector.end(), [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });
        if (!filter_made) {
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
            true_dominance_filtered -= dominance_filtered;
            k--;
            continue;
        }
        else {
            filter_made = false;
        }

        // filters based on dominance
        auto dominance_start = chrono::steady_clock::now();
        size_t number_of_values = 0;
        int c = 1;
        output_vector.clear();
        for (auto& [map_key, value] : sorting_map) {
            // cout << c << " out of " << sorting_map.size() << ": " << value.size() << "! loops." << endl;
            number_of_values = max(number_of_values, value.size());
            vector<bool> removed(value.size(), false);
            Dominance dominance_result;
            for (size_t i = 0; i < value.size(); i++) {
                if (removed.at(i)) {
                    continue;
                }
                
                for (size_t j = i + 1; j < value.size(); j++) {
                    if (removed.at(j)) {
                        continue;
                    }

                    // Sees if either recipe dominates the other
                    dominance_result = does_dominate(value.at(i), value.at(j));

                    // If value.at(i) dominates, remove value.at(j)
                    if (dominance_result == Dominance::A_DOMINATES) {
                        removed.at(j) = true;
                    }
                    // If value.at(j) dominates, remove value.at(i)
                    // Then break because there is no point in comparing anything else to value.at(i)
                    else if (dominance_result == Dominance::B_DOMINATES) {
                        removed.at(i) = true;
                        break;
                    }
                }
            }

            for (size_t i = 0; i < removed.size(); i++) {
                if (!(removed.at(i))) {
                    output_vector.push_back(value.at(i));
                }
                else {
                    unfiltered -= 1;
                    true_unfiltered -= 1;
                    dominance_filtered += 1;
                    true_dominance_filtered += 1;
                }
            }
            c += 1;
        }
        auto dominance_end = chrono::steady_clock::now();
        chrono::duration<double> dominance_elapsed = dominance_end - dominance_start;

        // If the output_vector is still too large, halve the max output allowed to reduce combinations
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
                true_dominance_filtered -= dominance_filtered;
                k--;
                continue;
            }
        }
        else {
            max_output = base_max_output;
        }

        // preps the array to be output
        output_array.clear();
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_vector.at(i).set_machine_speed(60.0);
            output_chain = output_vector.at(i).to_compressed_json();
            output_array.push_back(output_chain);
        }
        output_object["Category"] = test_item;
        output_object["Data"] = output_array;

        m = incrementor_map.at(test_item);
        recipes.at(test_item) = output_vector;
        incrementor_max.at(m) = output_vector.size();

        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;



        auto pre_output = chrono::steady_clock::now();
        
        // outputs the array to the file
        if (!first) {
            results << "," << endl;
        }
        first = false;

        results << output_object.dump(4);

        string filter_item;
        for (auto& item : filter_json) {
            filter_item = item.value("ItemClass", "N/A");
            item["Depth"] = filter_map.at(filter_item);
        }

        ofstream filter_out(exePath / "dat" / "100_combination_filter.json");
        filter_out << filter_json.dump(4);
        filter_out.close();

        auto post_output = chrono::steady_clock::now();
        chrono::duration<double> output_time = post_output - pre_output;

        cout << test_item << " has been proccessed." << endl;
        if (expected_count != total) {
            cout << "Estimated total doesn't equal calculated total. " << expected_count << " != " << total << endl;
            status_log << "Estimated total doesn't equal calculated total. " << expected_count << " != " << total << endl;
        }
        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been processed." << endl;
        status_log << unfiltered << " recipes were output." << endl;
        status_log << machine_filtered << " recipes were filtered due to number of machines." << endl;
        status_log << dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
        status_log << "A maximum of " << number_of_values << " combinations were grouped together in the dominance filter." << endl;
        status_log << "Dominance filtering took " << dominance_elapsed.count() << " seconds to complete while making the filter." << endl;
        status_log << "Execution time: " << elapsed.count() << " seconds." << endl;
        status_log << "Output time: " << output_time.count() << " seconds." << endl;
        status_log << endl;

        if (elapsed >= update_frequency) {
            cout << total << " combinations have been processed." << endl;
            cout << unfiltered << " recipes were output." << endl;
            cout << machine_filtered << " recipes were filtered due to number of machines." << endl;
            cout << dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            cout << "Output time: " << output_time.count() << " seconds." << endl;
            cout << endl;
        }

        if (k == num_to_test) {
            break;
        }
    }

    results << "]" << endl;
    // results << recipe_root.dump(4);

    auto true_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = true_end - true_start;
    cout << true_total << " combinations were processed." << endl;
    cout << true_unfiltered << " recipes were output." << endl;
    cout << true_machine_filtered << " recipes were filtered due to number of machines." << endl;
    cout << true_dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
    cout << "Execution time: " << total_elapsed.count() << " seconds." << endl;

    status_log << true_total << " combinations were processed." << endl;
    status_log << true_unfiltered << " recipes were output." << endl;
    status_log << true_machine_filtered << " recipes were filtered due to number of machines." << endl;
    status_log << true_dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
    status_log << "Execution time: " << total_elapsed.count() << " seconds." << endl;
    
    for (auto& item : filter_json) {
        test_item = item.value("ItemClass", "N/A");
        item["Depth"] = filter_map.at(test_item);
    }

    ofstream filter_out(exePath / "dat" / "100_combination_filter.json");
    filter_out << filter_json.dump(4);
    filter_out.close();

    recipe_in.close();
    test_recipe_in.close();
    terminal_recipe_in.close();
    results.close();
    status_log.close();

    increment_elapsed /= true_total;
    cout << "Average increment time: " << increment_elapsed.count() << endl;

    if (issue_occured) {
        cout << "A duplicate was found in the incrementor. Program terminated while processing " << test_item << "." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}

vector<Recipe> build_chain(map<DominanceKey, vector<Recipe>>& sorting_map, const unordered_map<string, Recipe> recipe_map, const unordered_map<string, int> filter_map, const unordered_map<string, Resource>& terminal_map, const vector<size_t>& incrementor, const string test_item, const int max_product) {
    vector<Recipe> output_recipes; // the vector of all recipes used by the chain
    stack<Recipe> recipe_stack; // the stack of recipes in the chain
    string product_name; // the name of the item being processed
    DominanceKey key; // the key to each section of sorting_map

    // Use to inject an item into the system
    recipe_stack.push(recipe_map.at(test_item));
    
    // Creates the recipe chain based on the provided recipes
    while (!recipe_stack.empty()) {
        if (recipe_stack.top().is_processed()) {
            bool already_added = false; // marks if a recipe is already in the vector
            size_t location = 0; // the location of the identical recipe in the vector
            
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
            vector<Resource> ingredients = recipe_stack.top().get_ingredients(); // gets the ingredients of the current recipe
            for (size_t i = 0; i < ingredients.size(); i++) { // increments through all the ingredients

                auto terminal_location = terminal_map.find(ingredients.at(i).get_name()); // finds if the item is terminal
                if (terminal_location != terminal_map.end()) {
                    continue;
                }

                auto recipe_location = recipe_map.find(ingredients.at(i).get_name()); // finds the recipe in the map
                if (recipe_location != recipe_map.end()) {
                    Recipe new_recipe = recipe_location->second; // sets new_recipe to the recipe found
                    new_recipe.set_to(ingredients.at(i).get_amount()); // raises the recipe product to match the ingredient it is for
                    recipe_stack.push(new_recipe); // adds the new recipe to the stack
                }
                else {
                    // if now recipe was found, outputs the fact as there may be missing data somewhere
                    // the program otherwise continues as if the resource was terminal
                    cout << "No recipe found for " << ingredients.at(i).get_name() << "." << endl;
                    // status_log << "No recipe found for " << ingredients.at(i).get_name() << "." << endl;
                }
            }
        }
    }

    int speed_lm = 1;
    Fraction rate;
    for (size_t i = 0; i < output_recipes.size(); i++) {
        product_name = output_recipes.at(i).get_product(0).get_name();
        rate = (output_recipes.at(i).get_product(0).get_amount() / recipe_map.at(product_name).get_product(0).get_amount());
        rate *= recipe_map.at(product_name).get_machine_speed();
        rate /= 60;
        speed_lm = lcm(speed_lm, rate.get_denominator());
    }

    // converts the output vector into compressed json
    Recipe output;
    int item_lm = 1; // the least common multiple of the denominators
    string incrementor_ID = ""; // The ID that identifies what recipes were used to make the chain
    output.merge_recipes(output_recipes);
    output.set_primary_product(test_item);
    for (size_t i = 0; i < incrementor.size(); i++) {
        incrementor_ID.append(to_string(incrementor.at(i)));
        if ((i + 1) != incrementor.size()) {
            incrementor_ID.append("|");
        }
    }
    output.set_ID(incrementor_ID);
    output.set_name(test_item);
    output *= speed_lm;
    for (size_t i = 0; i < output.get_ingredients().size(); i++) {
        item_lm = lcm(item_lm, output.get_ingredient(i).get_amount().get_denominator());
    }
    for (size_t i = 0; i < output.get_products().size(); i++) {
        item_lm = lcm(item_lm, output.get_product(i).get_amount().get_denominator());
    }
    output *= item_lm;

    rate = (output.get_product(0).get_amount() / recipe_map.at(product_name).get_product(0).get_amount());
    rate /= 60;
    rate *= recipe_map.at(test_item).get_machine_speed();
    int number_of_machines = rate.get_numerator();
    if (number_of_machines < 0 || number_of_machines > 10000) {
        number_of_machines = 2147483647;
    }
    output.set_machine_speed(number_of_machines);

    // Checks if the total number of machines is more than the maximum and doesn't add it if it is
    if (number_of_machines > 0 && number_of_machines <= max_product && (number_of_machines <= filter_map.at(test_item) || filter_map.at(test_item) == 0)) {
        // if the recipe is valid, adds it to the output
        sort(output.modify_ingredients().begin(), output.modify_ingredients().end(), [](const Resource& a, const Resource& b) { return a.get_name() < b.get_name(); });
        key.ingredient_names.clear();
        key.product_amount = output.get_product(0).get_amount();
        for (size_t i = 0; i < output.get_ingredients().size(); i++) {
            key.ingredient_names.push_back(output.get_ingredient(i).get_name());
        }
        sorting_map[key].push_back(output);

        unfiltered += 1;
        true_unfiltered += 1;
        total += 1;
        true_total += 1;
    }
    else {
        // if the recipe is not valid, removes it
        machine_filtered += 1;
        true_machine_filtered += 1;
        total += 1;
        true_total += 1;
    }

    return output_recipes;
}