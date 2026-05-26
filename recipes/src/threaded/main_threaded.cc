#include "../lib/json.hpp"

#include <cstddef>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <numeric>

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <algorithm>

#include <chrono>

#include "../resources/recipe.h"
#include "../resources/resource.h"
#include "../resources/fraction.h"
#include "../resources/dominance.h"
#include "incrementor.h"


using namespace std;
using json = nlohmann::ordered_json;


#define ANSI_RED "\033[31m"
#define ANSI_RESET "\033[0m"


#define ifstream(stream, file) ifstream stream(file);\
    if (!stream.is_open()) {\
        cerr << ANSI_RED << "File failed to open:" << endl << '\t' << file << ANSI_RESET << endl;\
        stream.close();\
        throw runtime_error("File not found");\
    }

#define ofstream(stream, file) ofstream stream(file);\
    if (!stream.is_open()) {\
        cerr << ANSI_RED << "File failed to open:" << endl << '\t' << file << ANSI_RESET << endl;\
        stream.close();\
        throw runtime_error("File not found");\
    }


int main(int argc, char* argv[]) {

    // Open IO
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    ifstream(recipe_in,             exePath / "dat" / "recipes.json");
    ifstream(test_recipe_in,        exePath / "dat" / "test_input.json");
    ifstream(terminal_recipe_in,    exePath / "dat" / "terminal_resources.json");
    ofstream(results,               exePath / "dat" / "test_results.json");
    ofstream(status_log,            exePath / "dat" / "test_status.log");


    // Handle JSON inputs
    json recipe_root;
        recipe_in >> recipe_root;
        recipe_in.close();

    json test_recipe_root;
        test_recipe_in >> test_recipe_root;
        test_recipe_in.close();

    json terminal_root;
        terminal_recipe_in >> terminal_root;
        terminal_recipe_in.close();



    auto processing_start = chrono::steady_clock::now();

    // Process test_recipe
    Recipe test_recipe  (test_recipe_root.at(0));                         // Use to inject a RECIPE into the system
    string test_item =   test_recipe_root.at(1).value("ItemClass", "");   // Use to inject an ITEM into the system

    // The auto terminate information
    const int               MAX_LOOPS =         stoi(                        test_recipe_root.at(2).value("max_loops", "0"));                   // the maximum number of loops the program is allowed to run
    const chrono::minutes   MAX_TIME            (stoi(                       test_recipe_root.at(2).value("max_time", "0")));                   // the max time the program is allowed to run
    const size_t            ITEMS_TO_TEST =     static_cast<size_t>(stoi(    test_recipe_root.at(2).value("number_items_to_test", "0")) - 1);   // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds   UPDATE_FREQUENCY    (stoi(                       test_recipe_root.at(3).value("update_frequency", "0")));           // the frequency the program updates its progress

    int update_counter = 1; // the number of updates

    // The filter information
    const int           MAX_PRODUCT =           test_recipe_root.at(4).value("max_product", 1000);                  // the maximum amount of product a recipe chain is allowed to have
    const int           MAX_MACHINES =          test_recipe_root.at(4).value("max_filter", 200);                    // the maximum number of machines the filter is allowed to have under normal circumstances
    const unsigned int  BASE_MAX_OUTPUT =       test_recipe_root.at(4).value("max_combinations", 1000);             // the maximum number of combinations being output
    const unsigned int  ABSOLUTE_MAX_OUTPUT =   test_recipe_root.at(4).value("absolute_max_combinations", 10000);   // max_output will never exceed this value
    
    bool                remake_filters =        test_recipe_root.at(4).value("remake_filters", false);  // whether or not to remake the filter
    unsigned int        max_output =            BASE_MAX_OUTPUT;                                        // the current maximum output for filtering


    // an empty json array to make the .value() function work
    json empty_array = json::array();


    // Process terminal resources
    unordered_map<string, Resource> terminal_map;
    for (const auto& terminal : terminal_root) {
        Resource terminal_resource(terminal);
        terminal_map.insert({terminal_resource.get_name(), terminal_resource});
    }


    // Attempt to open filters
    bool filters_exist = true;
    unordered_map<string, int> filter_map;
    json filters_root;
    try {
        ifstream(filters_in,            exePath / "dat" / "100_combination_filter.json");

        //JSON input
        filters_in >> filters_root;
        filters_in.close();

        // Process filters
        for (const auto& data : filters_root) {
            if (remake_filters) {
                filter_map.insert({data.value("ItemClass", "N/A"), 0});
            }
            else {
                filter_map.insert({data.value("ItemClass", "N/A"), data.value("Depth", 0)});
            }
        }
    } catch (const runtime_error& e) {
        if (strcmp(e.what(), "File not found") == 0) {
            filters_exist = false;
        } else {
            throw e;
        }
    }


    // Process recipes
    // Build incrementor
    // Builds filters if they don't exist
    Incrementor incrementor;                                    // controls stepping through combinations of alternate recipes
    unordered_map<string, Recipe> recipe_map;                   // holds Recipe objects
    unordered_map<string, vector<Recipe>> recipes_by_category;  // holds all the recipes

    for (const auto& data : recipe_root) {
        string category = data.value("Category", "");
        size_t selected_recipe = 0,
            recipe_count = data.value("Data", empty_array).size();

        incrementor.push_back(category, selected_recipe, recipe_count);

        vector<Recipe> output_recipes;
        for (const auto& recipe : data["Data"]) {
            output_recipes.push_back(Recipe(recipe));
        }
        recipes_by_category.insert({category, output_recipes});

        recipe_map.insert({
            category,
            Recipe(data.value("Data", empty_array).at(0))
        });
        
        if (!filters_exist) {
            filter_map.insert({category, 0});
        }
    }


    //
    // Add a method to know total number of combinations needing to be processed for a given item
    // Note that it is not accurate if an ingredient was terminated early.
    // It still would be a good start though.
    //


    // DEBUG
    // stat variables
    int across_tests_unfiltered = 0,
        across_tests_machine_filtered = 0,
        across_tests_dominance_filtered = 0,
        across_tests_total = 0,
        loop_termination_count = 0,
        time_termination_count = 0;


    // Begin execution
    results << "[" << endl;
    bool redo_filter = false;

    for (size_t k = 0; k < recipe_root.size() && k < ITEMS_TO_TEST; k++) {
        auto test_start = chrono::steady_clock::now(); // starts the timer

        
        // DEBUG flags
        bool loop_termination = false,
            time_termination = false;
        update_counter = 1;


        // Output storage vectors
        vector<Recipe> output_vector;
        map<DominanceKey, vector<Recipe>> sorting_map; // the map containing the sorted outputs ready for dominance filtering


        // Set the item being processed
        test_item = recipe_root.at(k).value("Category", "");    // NOTE, overrides test_item before it is injected into system

        bool filter_made = false;
        if (!remake_filters && filter_map.find(test_item) != filter_map.end()) {
            filter_made = true;
        }
        
        if (redo_filter) {
            filter_made = false;
            redo_filter = false;
        }


        // Status tracking variables
        int count = 0,
            unfiltered = 0,
            machine_filtered = 0,
            dominance_filtered = 0,
            total = 0;


        // Estimate combinations to process
        for (size_t i = 0; i < recipes_by_category.at(test_item).size(); i++) {

            vector<Resource> ingredients = recipes_by_category.at(test_item).at(i).get_ingredients();
            int combinations = 1;

            for (size_t j = 0; j < ingredients.size(); j++) {
                string temp_item = ingredients.at(j).get_name();

                if (recipes_by_category.find(temp_item) != recipes_by_category.end()) {

                    combinations *= recipes_by_category.at(temp_item).size();
                }
            }
            count += combinations;
        }

        status_log << test_item << " about to be processed." << endl;
        status_log << "Estimated to need to process " << count << " combinations to complete." << endl;

        stack<Recipe> recipe_stack; // the stack of recipes in the chain

        // The main function, runs until the incrementor vector has returned back to its starting value
        do {
            // Output storage vectors
            vector<Recipe> output_recipes;
            json chain_array;


            // Inject an item into the system
            {
                size_t selected_recipe = incrementor.get_index(test_item);
                size_t selected_alternate = incrementor.alternate(selected_recipe);

                recipe_stack.push(recipes_by_category.at(test_item).at(selected_alternate));
            }

            while (!recipe_stack.empty()) {

                // if the current recipe has already been processed, remove it from the stack
                if (recipe_stack.top().is_processed()) {
                    
                    //output recipes should only contain unique recipes
                    //add recipe from stack to output recipes

                    bool already_added = false;
                    size_t location;
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

                }
                // if the current recipe has not been processed, process it
                else {
                    recipe_stack.top().set_processed();

                    //examine ingredients for processing
                    vector<Resource> ingredients = recipe_stack.top().get_ingredients();
                    for (size_t i = 0; i < ingredients.size(); i++) {

                        string ingredient_name = ingredients.at(i).get_name();

                        //skip terminal ingredients
                        auto terminal_location = terminal_map.find(ingredient_name);
                        if (terminal_location != terminal_map.end()) {

                            // if the ingredient was terminal, adds it to the stack and moves on to the next one
                            /*
                            terminal_recipe.set_terminal_recipe(ingredients.at(i));
                            recipe_stack.push(terminal_recipe);
                            */
                            continue;
                        }

                        auto recipe_location = recipe_map.find(ingredient_name); // finds the recipe in the map
                        if (recipe_location != recipe_map.end()) {

                            Recipe new_recipe = recipe_location->second; // the recipe found
                            new_recipe.set_to(ingredients.at(i).get_amount()); // raises the recipe product to match the ingredient it is for
                            recipe_stack.push(new_recipe); // adds the new recipe to the stack

                        }
                        else {

                            // if no recipe was found, outputs the fact as there may be missing data somewhere
                            // the program otherwise continues as if the resource was terminal
                            cout << "No recipe found for " << ingredient_name << "." << endl;
                            status_log << "No recipe found for " << ingredient_name << "." << endl;
                        }
                    } //examine ingredients
                } //if processed...
            } //recipe stack...


            // converts the output vector into uncompressed json
            /*
            for (size_t i = 0; i < output_recipes.size(); i++) {
                chain_object = output_recipes.at(i).to_json();
                chain_array.push_back(chain_object);
            }
            output_chain = chain_array;
            output_array.push_back(output_chain);
            */

            
            // Find lm rate for whole-number combinations            
            int speed_lm = 1;
            string product_name;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                product_name = output_recipes.at(i).get_product(0).get_name();

                Fraction rate = output_recipes.at(i).get_product(0).get_amount() / recipe_map.at(product_name).get_product(0).get_amount();

                rate *= recipe_map.at(product_name).get_machine_speed();
                rate /= 60;

                speed_lm = lcm(speed_lm, rate.get_denominator());
            }


            // Convert the output vector into compressed json
            Recipe output;
            output.merge_recipes(output_recipes);
            output.set_primary_product(test_item);

            string incrementor_ID = ""; // The ID that identifies what recipes were used to make the chain
            for (size_t i = 0; i < incrementor.size(); i++) {

                incrementor_ID.append(to_string(incrementor.alternate(i)));

                if (i + 1 < incrementor.size()) {
                    incrementor_ID.append("|");
                }
            }

            output.set_ID(incrementor_ID);
            output.set_name(test_item);
            output *= speed_lm;

            int item_lm = 1; // the least common multiple of the denominators
            for (size_t i = 0; i < output.get_ingredients().size(); i++) {
                item_lm = lcm(item_lm, output.get_ingredient(i).get_amount().get_denominator());
            }
            for (size_t i = 0; i < output.get_products().size(); i++) {
                item_lm = lcm(item_lm, output.get_product(i).get_amount().get_denominator());
            }
            output *= item_lm;

            // output_chain = output.to_compressed_json();

            Fraction rate = output.get_product(0).get_amount() / recipe_map.at(product_name).get_product(0).get_amount();
            rate /= 60;
            rate *= recipe_map.at(test_item).get_machine_speed();

            int number_of_machines = rate.get_numerator();
            if (number_of_machines < 0 || number_of_machines > 10000) {

                number_of_machines = INT_MAX;
            }
            output.set_machine_speed(number_of_machines);


            // Filter output
            // If the total number of machines <= maximum, check if valid
            if (
                number_of_machines > 0 && 
                number_of_machines <= MAX_PRODUCT && 
                (
                    number_of_machines <= filter_map.at(test_item) ||
                    filter_map.at(test_item) != 0
                )
            ) {
                // If the recipe is valid, add it to the output

                sort(output.modify_ingredients().begin(), output.modify_ingredients().end(),
                    [](const Resource& a, const Resource& b) { return a.get_name() < b.get_name(); });


                DominanceKey key;
                key.product_amount = output.get_product(0).get_amount();

                for (size_t i = 0; i < output.get_ingredients().size(); i++) {
                    key.ingredient_names.push_back(output.get_ingredient(i).get_name());
                }

                sorting_map[key].push_back(output);
                output_vector.push_back(output);

                unfiltered += 1;
                across_tests_unfiltered += 1;
                total += 1;
                across_tests_total += 1;

            }
            // if the recipe is not valid, remove it
            else {
                machine_filtered += 1;
                across_tests_machine_filtered += 1;
                total += 1;
                across_tests_total += 1;
            }


            // Increment the incrementor
            vector<size_t> incrementor_values;
            vector<string> incrementor_products;
            for (size_t i = 0; i < output_recipes.size(); i++) {

                product_name = output_recipes.at(i).get_product(0).get_name();
                incrementor_products.push_back(product_name);

                incrementor_values.push_back(incrementor.get_index(product_name));
            }

            sort(incrementor_values.begin(), incrementor_values.end());
            // duplicate_found = check_duplicate_incrementor_values(incrementor_values, incrementor_products, incrementor_map, status_log);
            bool increment = true; // Determines if the value should be incremented
            for (size_t j = 0; j < incrementor_values.size(); j++) {
                size_t i = incrementor_values.at(j);
                // if the value needs to be incremented, add one to it
                if (increment) {
                    incrementor.alternate(i) += 1;
                    increment = false;
                }
                // if the value has reached its maximum, set it to zero and set overflow flag
                if (incrementor.alternate(i) >= incrementor.alternate_max(i)) {
                    incrementor.alternate(i) = 0;
                    increment = true;
                }

                //update recipe_map with new alternate
                product_name = recipe_root.at(i).value("Category", "");
                recipe_map[product_name] = recipes_by_category.at(product_name).at(incrementor.alternate(i));
            }
            
            // If the last value reached its maximum
            // Set all incrementor values to 0 to end the while loop
            if (increment) {
                for (size_t j = 0; j < incrementor.size(); j++) {
                    incrementor.alternate(j) = 0;
                    product_name = recipe_root.at(j).value("Category", "");
                    recipe_map[product_name] = recipes_by_category.at(product_name).at(0);
                }
            }


            // DEBUG
            // current status
            auto increment_end = chrono::steady_clock::now();
            chrono::duration<double> elapsed = increment_end - test_start;

            if (elapsed >= (UPDATE_FREQUENCY * update_counter)) {
                update_counter += 1;

                cout << test_item << " is being proccessed." << endl;
                cout << total << " combinations have been processed." << endl;
                cout << unfiltered << " recipes were output." << endl;
                cout << machine_filtered << " recipes were filtered due to number of machines." << endl;
                cout << dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
                cout << "Execution time: " << elapsed.count() << " seconds." << endl;
                cout << endl;
            }


            // Terminate early
            if (count >= MAX_LOOPS) {
                loop_termination = true;
                break;
            }
            if (elapsed >= MAX_TIME) {
                time_termination = true;
                break;
            }

        } while (!incrementor.all_zeros());


        // DEBUG
        // current status
        {
            auto test_end = chrono::steady_clock::now();
            chrono::duration<double> elapsed = test_end - test_start;

            if (elapsed >= UPDATE_FREQUENCY) {  
                cout << test_item << " was proccessed." << endl;
                cout << total << " combinations were found." << endl;
                cout << "Execution time: " << elapsed.count() << " seconds." << endl;
                cout << endl;
            }
        }

        // Make filters

        auto result = max_element(output_vector.begin(), output_vector.end(),
            [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });


        auto minimum = min_element(output_vector.begin(), output_vector.end(),
            [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });


        if (!filter_made) {
            // checks if the maximum is less than 10. Skips the creation process if it is.
            if ((*result).get_machine_speed() <= 10) {
                filter_map.at(test_item) = 10;

            }
            else {
                if (output_vector.size() > BASE_MAX_OUTPUT) {

                    partial_sort(output_vector.begin(), output_vector.begin() + BASE_MAX_OUTPUT, output_vector.end(),
                        [](const Recipe& a, const Recipe& b) { return a.get_machine_speed() < b.get_machine_speed(); });


                    // filter_map.at(test_item) = (ceil(output_vector.at(max_output).get_machine_speed() / 10) * 10);
                    filter_map.at(test_item) = output_vector.at(max_output - 1).get_machine_speed();

                }
                else {
                    filter_map.at(test_item) = (ceil((*result).get_machine_speed() / 10) * 10);
                }
            }


            if (output_vector.at(0).get_machine_speed() > MAX_MACHINES) {
                filter_map.at(test_item) = (*minimum).get_machine_speed();

            }
            else if (filter_map.at(test_item) > MAX_MACHINES) {
                filter_map.at(test_item) = MAX_MACHINES;
            }

            // DEBUG
            // log
            cout << test_item << " filter has been created." << endl;
            status_log << test_item << " filter has been created." << endl;

            if (time_termination || loop_termination) {
                cout << "Item too complex, filter may be incomplete." << endl;
                status_log << "Item too complex, filter may be incomplete." << endl;
            }


            //Run test again, with new filter
            filter_made = true;
            across_tests_unfiltered -= unfiltered;
            across_tests_machine_filtered -= machine_filtered;
            across_tests_dominance_filtered -= dominance_filtered;
            k--;
            continue;

        }
        else {
            /*
            if (output_vector.size() > max_output) {
                filtered += (output_vector.size() - 100);
                true_filtered += (output_vector.size() - 100);
                unfiltered += 100;
                true_unfiltered += 100;
                output_vector.resize(max_output);
            }
            else {
                unfiltered += output_vector.size();
                true_unfiltered += output_vector.size();
            }
            */

            filter_made = false;
        }


        // Filter based on dominance
        auto dominance_start = chrono::steady_clock::now();
        size_t number_of_values = 0;
        output_vector.clear();
        for (auto& [map_key, value] : sorting_map) {
            
            // cout << c << " out of " << sorting_map.size() << ": " << value.size() << "! loops." << endl;
            number_of_values = max(number_of_values, value.size());

            // Compare recipes in value to trim based on dominance
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
                    across_tests_unfiltered -= 1;
                    dominance_filtered += 1;
                    across_tests_dominance_filtered += 1;
                }
            }
        }
        auto dominance_end = chrono::steady_clock::now();
        chrono::duration<double> dominance_elapsed = dominance_end - dominance_start;


        // If the output_vector is still too large, halve the max output allowed to reduce combinations
        if (output_vector.size() > ABSOLUTE_MAX_OUTPUT) {
            if (
                (*minimum).get_machine_speed() == filter_map.at(test_item) ||
                ((*minimum).get_machine_speed() < 10 &&
                filter_map.at(test_item) == 10)
            ) {
                cout << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
                cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                status_log << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
                status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;

                max_output = BASE_MAX_OUTPUT;
            }
            else {
                cout << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
                cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
                status_log << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
                status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;

                // Run test again with new filter
                redo_filter = true;
                filter_map.at(test_item) = 0;
                max_output /= 2;
                across_tests_unfiltered -= unfiltered;
                across_tests_machine_filtered -= machine_filtered;
                across_tests_dominance_filtered -= dominance_filtered;
                k--;
                continue;
            }
        }
        else {
            max_output = BASE_MAX_OUTPUT;
        }

        // Prep the array to be output
        json output_array = json::array();
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_vector.at(i).set_machine_speed(60.0);
            json output_chain = output_vector.at(i).to_compressed_json();
            output_array.push_back(output_chain);
        }
        json output_object = json::object();
        output_object["Category"] = test_item;
        output_object["Data"] = output_array;

        size_t selected_alternate = incrementor.get_index(test_item);
        recipes_by_category.at(test_item) = output_vector;
        incrementor.alternate_max(selected_alternate) = output_vector.size();

        auto test_end = chrono::steady_clock::now();
        chrono::duration<double> test_elapsed = test_end - test_start;


        // Output to file
        auto pre_output_start = chrono::steady_clock::now();
        
        // outputs the array to the file
        if (k>0) {
            results << "," << endl;
        }

        results << output_object.dump(4);

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
        output_vector = recipes.at(test_item);
        for (size_t i = 0; i < output_vector.size(); i++) {
            output_chain = output_vector.at(i).to_compressed_json();
            output_array.push_back(output_chain);
        }
        recipe_root.at(m)["Data"] = output_array;
        */


        // Output filters to file
        string filter_item;
        for (auto& item : filters_root) {
            filter_item = item.value("ItemClass", "N/A");
            item["Depth"] = filter_map.at(filter_item);
        }

        ofstream(filter_out, exePath / "dat" / "100_combination_filter.json");
        filter_out << filters_root.dump(4);
        filter_out.close();

        auto post_output_end = chrono::steady_clock::now();
        chrono::duration<double> output_time = post_output_end - pre_output_start;


        // DEBUG
        // log        
        cout << test_item << " has been proccessed." << endl;
        if (count != total) {
            cout << "Estimated total doesn't equal calculated total. " << count << " != " << total << endl;
            status_log << "Estimated total doesn't equal calculated total. " << count << " != " << total << endl;
        }

        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been processed." << endl;
        status_log << unfiltered << " recipes were output." << endl;
        status_log << machine_filtered << " recipes were filtered due to number of machines." << endl;
        status_log << dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
        status_log << "A maximum of " << number_of_values << " combinations were grouped together in the dominance filter." << endl;
        status_log << "Dominance filtering took " << dominance_elapsed.count() << " seconds to complete while making the filter." << endl;
        status_log << "Execution time: " << test_elapsed.count() << " seconds." << endl;
        status_log << "Output time: " << output_time.count() << " seconds." << endl;

        if (loop_termination) {
            status_log << "The program exceeded " << MAX_LOOPS << " loops. The item is too complex." << endl;
            loop_termination_count++;
        }
        else if (time_termination) {
            status_log << "The program exceeded " << MAX_TIME.count() << " minutes. The item is too complex." << endl;
            time_termination_count++;
        }
        status_log << endl;

        auto end = chrono::steady_clock::now();
        auto elapsed = end - processing_start;
        if (elapsed >= UPDATE_FREQUENCY || loop_termination || time_termination) {
            cout << total << " combinations have been processed." << endl;
            cout << unfiltered << " recipes were output." << endl;
            cout << machine_filtered << " recipes were filtered due to number of machines." << endl;
            cout << dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
            cout << "Execution time: " << elapsed.count() << " seconds." << endl;
            cout << "Output time: " << output_time.count() << " seconds." << endl;
            if (loop_termination) {
                cout << "The program exceeded " << MAX_LOOPS << " loops. The item is too complex." << endl;
            }
            else if (time_termination) {
                cout << "The program exceeded " << MAX_TIME.count() << " minutes. The item is too complex." << endl;
            }
            cout << endl;
        }
    }


    // All tests have completed
    results << "]" << endl;
    // results << recipe_root.dump(4);

    auto all_tests_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = all_tests_end - processing_start;
    cout << across_tests_total << " combinations were processed." << endl;
    cout << across_tests_unfiltered << " recipes were output." << endl;
    cout << across_tests_machine_filtered << " recipes were filtered due to number of machines." << endl;
    cout << across_tests_dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
    cout << "Execution time: " << total_elapsed.count() << " seconds." << endl;

    status_log << across_tests_total << " combinations were processed." << endl;
    status_log << across_tests_unfiltered << " recipes were output." << endl;
    status_log << across_tests_machine_filtered << " recipes were filtered due to number of machines." << endl;
    status_log << across_tests_dominance_filtered << " recipes were filtered during the dominance filtering." << endl;
    status_log << "Execution time: " << total_elapsed.count() << " seconds." << endl;
    

    // Output filters to file
    for (auto& item : filters_root) {
        test_item = item.value("ItemClass", "N/A");
        item["Depth"] = filter_map.at(test_item);
    }

    ofstream(filter_out, exePath / "dat" / "100_combination_filter.json");
    filter_out << filters_root.dump(4);
    filter_out.close();

    results.close();
    status_log.close();

}

#undef ifstream
#undef ofstream
