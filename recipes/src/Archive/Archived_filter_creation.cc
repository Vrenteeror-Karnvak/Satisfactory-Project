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

int percentile(vector<int>& v, double p);

int main(int argc, char* argv[]) {
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
    ofstream status_log(exePath / "dat" / "filter_creation_status.log");

    // Inputs the filter data and reopens the file as an output
    ifstream filter_in(exePath / "dat" / "item_filters.json");
    json filter_json;
    filter_in >> filter_json;
    filter_in.close();

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
    const int max_loops = stoi(test_recipe_root.at(2).value("max_loops", "0")); // the maximum number of loops the program is allowed to run
    const chrono::minutes max_time(stoi(test_recipe_root.at(2).value("max_time", "0"))); // the max time the program is allowed to run
    size_t num_to_test = static_cast<size_t>(stoi(test_recipe_root.at(2).value("number_items_to_test", "0")) - 1); // the number of items to test before terminating the loop in order to avoid super complex items
    const chrono::seconds update_frequency(stoi(test_recipe_root.at(3).value("update_frequency", "0"))); // the frequency the program updates its progress
    int u = 1; // the number of updates
    
    // The filter information
    int max_product = test_recipe_root.at(4).value("max_product", 1000); // the maximum amount of product a recipe chain is allowed to have period
    int filter_value = 0; // the value of the filter
    int number_of_machines = 0;
    vector<int> number_machines_info;
    bool remake_filters = test_recipe_root.at(4).value("remake_filters", false); // whether or not to remake the filter
    bool filter_made = false; // has the filter for the item already been made?
    bool filter_tested = false; // has the filter been tested?
    unordered_map<string, int> filter_map;
    for (const auto& data : filter_json) {
        filter_map.insert({data.value("ItemClass", "N/A"), data.value("Depth", 0)});
    }

    // The json file containing the terminal resources
    json terminal_root;
    terminal_recipe_in >> terminal_root;
    Resource terminal_resource;
    unordered_map<string, Resource> terminal_map;

    // The variables that the stack uses to increment through all nodes
    stack<Recipe> recipe_stack; // the stack of recipes in the chain
    vector<Resource> ingredients; // the ingredients being added
    Resource ingredient; // the current ingredient being added
    Recipe new_recipe; // the recipe being added to the stack for non terminal resources
    Recipe terminal_recipe; // the recipe being added to the stack for terminal resources

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

        /*
        // Use this to reset the item_filter list.
        if (m == 0) {
            filter_json.clear();
        }
        json temp_json = json::object();
        temp_json["ItemClass"] = data.value("Category", "");
        temp_json["Depth"] = 0;
        filter_json.push_back(temp_json);
        */

        m += 1;
    }
    
    /*
    // Use this to reset the item_filter list
    ofstream filter_set(exePath / "dat" / "item_filters.json");
    filter_set << filter_json.dump(4);
    filter_set.close();
    return 0;
    */

    for (size_t k = 0; k < recipe_root.size(); k++) {
        auto start = chrono::steady_clock::now(); // starts the timer

        // Clears termination flags and debug variables
        loop_termination = false;
        time_termination = false;

        // clears the output storage vectors
        output_vector.clear();
        number_machines_info.clear();

        // Sets the item being processed
        test_item = recipe_root.at(k).value("Category", "");
        filter_value = filter_map.at(test_item);

        if (!remake_filters && !filter_made) {
            filter_made = true;
            filter_tested = true;
            status_log << test_item << " filter has already been made." << endl;
        }
        if (filter_value == 0) {
            filter_made = false;
            filter_tested = false;
        }

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
            m = incrementor_map[test_item];
            recipe_stack.push(recipes.at(test_item).at(incrementor.at(m)));
            
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

            int speed_lm = 1;
            Fraction rate;
            for (size_t i = 0; i < output_recipes.size(); i++) {
                rate = (output_recipes.at(i).get_product(0).get_amount() / recipe_map.at(test_item).get_product(0).get_amount());
                rate *= recipe_map.at(test_item).get_machine_speed();
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
            // output.set_ID(incrementor_ID); no longer uses a string ID
            output.set_name(test_item);
            for (size_t i = 0; i < output.get_ingredients().size(); i++) {
                item_lm = lcm(item_lm, output.get_ingredient(i).get_amount().get_denominator());
            }
            for (size_t i = 0; i < output.get_products().size(); i++) {
                item_lm = lcm(item_lm, output.get_product(i).get_amount().get_denominator());
            }
            output *= item_lm;
            output *= speed_lm;

            rate = (output.get_product(0).get_amount() / recipe_map.at(test_item).get_product(0).get_amount());
            rate /= 60;
            rate *= recipe_map.at(test_item).get_machine_speed();
            number_of_machines = rate.get_numerator();
            if (number_of_machines < 0 || number_of_machines > max_product) {
                number_of_machines = 2147483647;
            }

            if (filter_made) {
                // Checks if the total number of machines is more than the maximum and doesn't add it if it is
                if (number_of_machines <= filter_value) {
                    // if the recipe is valid, adds it to the output
                    output_vector.push_back(output);
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
            }
            else {
                if (number_of_machines != 2147483647) {
                    // Adds the recipe to the info list if it is considered valid.
                    number_machines_info.push_back(number_of_machines);
                }
            }

            // increments the incrementor vector
            vector<size_t> incrementor_values;
            vector<string> incrementor_products;
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
            }
            
            // If the last value reached its maximum
            // Set all incrementor values to 0 to end the while loop
            if (increment) {
                for (size_t j = 0; j < incrementor.size(); j++) {
                    incrementor.at(j) = 0;
                    product_name = recipe_root.at(j).value("Category", "");
                    recipe_map[product_name] = recipes.at(product_name).at(0);
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
                cout << unfiltered << " recipes had a product amount less than or equal to " << filter_value << "." << endl;
                cout << filtered << " recipes had a product amount greater than " << filter_value << "." << endl;
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
        } while (!duplicate_found && incrementor != all_zeros);

        if (duplicate_found) {
            break;
        }

        int value;
        if (!filter_made) {
            // checks if the maximum is less than 10. Skips the creation process if it is.
            if (percentile(number_machines_info, 1.0) <= 10) {
                filter_map.at(test_item) = 10;
                status_log << test_item << " has a maximum of less than 10. Skipping." << endl;
                cout << test_item << " has a maximum of less than 10. Skipping." << endl;
                filter_made = true;
                filter_tested = true;
                k--;
                continue;
            }

            // outputs the filter data into the terminal
            cout << "The 00th percentile value is: " << percentile(number_machines_info, 0.00) << "." << endl;
            cout << "The 05th percentile value is: " << percentile(number_machines_info, 0.05) << "." << endl;
            cout << "The 10th percentile value is: " << percentile(number_machines_info, 0.10) << "." << endl;
            cout << "The 25th percentile value is: " << percentile(number_machines_info, 0.25) << "." << endl;
            cout << "The 35th percentile value is: " << percentile(number_machines_info, 0.35) << "." << endl;
            cout << "The 50th percentile value is: " << percentile(number_machines_info, 0.50) << "." << endl;
            cout << "The 75th percentile value is: " << percentile(number_machines_info, 0.75) << "." << endl;
            cout << "The 90th percentile value is: " << percentile(number_machines_info, 0.75) << "." << endl;
            cout << "The 100th percentile value is: " << percentile(number_machines_info, 1.0) << "." << endl;
            cout << "Total possibilies equals: " << number_machines_info.size() << "." << endl;
            cout << test_item << " is being processed." << endl;

            // collects the new filter value
            cout << "Please enter filter value (or type \"-1\" to quit): ";
            cin >> value;
            if (value == -1) {
                filter_map.at(test_item) = 0;
                status_log << test_item << " filter was being created." << endl;
                status_log << "Ending program early." << endl;
                product_name = test_item;
                duplicate_found = true;
                break;
            }
            filter_map.at(test_item) = value;
            status_log << test_item << " filter has been created." << endl;
            cout << test_item << " filter has been created." << endl;
            filter_made = true;
            k--;
            continue;
        }
        else if (!filter_tested) {
            // outputs the data and asks if the filter is good.
            string response;
            cout << test_item << " is being processed." << endl;
            cout << unfiltered << " recipes had a product amount less than or equal to " << filter_value << "." << endl;
            cout << filtered << " recipes had a product amount greater than " << filter_value << "." << endl;
            cout << "The program has tested " << count << " combinations of recipes." << endl;
            cout << "Is this filter good (y/n): ";
            cin >> response;
            // if it is not, allows to user to set a new value or restart the process for the given item
            if (response == "n") {
                cout << "Please enter new filter value (or type \"0\" to restart or \"-1\" to quit): ";
                cin >> value;
                if (value == 0) {
                    filter_map.at(test_item) = 0;
                    filter_made = false;
                }
                else if (value == -1) {
                    filter_map.at(test_item) = 0;
                    status_log << test_item << " filter was being tested." << endl;
                    status_log << "Ending program early." << endl;
                    product_name = test_item;
                    duplicate_found = true;
                    break;
                }
                else {
                    filter_map.at(test_item) = value;
                }
                k--;
                continue;
            }
            status_log << test_item << " filter has been tested." << endl;
            cout << test_item << " filter has been tested." << endl;
            filter_tested = true;
            k--;
            continue;
        }
        else {
            filter_made = false;
            filter_tested = false;
        }

        m = incrementor_map.at(test_item);
        recipes.at(test_item) = output_vector;
        incrementor_max.at(m) = output_vector.size();

        auto end = chrono::steady_clock::now();
        chrono::duration<double> elapsed = end - start;

        // cout << test_item << " has been proccessed." << endl;
        status_log << test_item << " has been proccessed." << endl;
        status_log << total << " combinations have been found." << endl;
        status_log << unfiltered << " recipes had a product amount less than or equal to " << filter_value << "." << endl;
        status_log << filtered << " recipes had a product amount greater than " << filter_value << "." << endl;
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

        if (k == num_to_test) {
            break;
        }
    }

    auto true_end = chrono::steady_clock::now();
    chrono::duration<double> total_elapsed = true_end - true_start;
    cout << "Execution time: " << total_elapsed.count() << " seconds." << endl;
    status_log << "Execution time: " << total_elapsed.count() << " seconds." << endl;
    
    for (auto& item : filter_json) {
        test_item = item.value("ItemClass", "N/A");
        item["Depth"] = filter_map.at(test_item);
    }

    ofstream filter_out(exePath / "dat" / "item_filters.json");
    filter_out << filter_json.dump(4);
    filter_out.close();

    recipe_in.close();
    test_recipe_in.close();
    terminal_recipe_in.close();
    status_log.close();

    if (total_loop_termination) {
        cout << loop_termination_count << " items exceeded " << max_loops << " loops." << endl;
    }
    else if (total_time_termination) {
        cout << time_termination_count << " items exceeded " << max_time.count() << " minutes." << endl;
    }
    else if (duplicate_found) {
        cout << "The program was ended early. Program terminated after processing " << product_name << "." << endl;
    }
    else {
        cout << "Everything is in working order here." << endl;
    }
}

int percentile(vector<int>& v, double p) {
    if (v.size() == 0) {
        return 0;
    }
    
    size_t k = static_cast<size_t>(p * (v.size() - 1));
    nth_element(v.begin(), v.begin() + k, v.end());
    return v[k];
}