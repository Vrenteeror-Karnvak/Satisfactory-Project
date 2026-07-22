#include "lib/json.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream recipe_in(exePath / "int/recipes_sorted.json");
    ifstream terminal_in(exePath / "dat" / "terminal_resources.json");
    ofstream recipe_out(exePath / "dat/recipes.json");
    ofstream item_out(exePath / "int/item_list.txt");
    ofstream item_analysis(exePath / "int/item_analysis.txt");
    ofstream tier_out(exePath / "dat" / "item_tiers.json");
    
    json tier_array = json::array();
    json tier_object = json::object();
    unordered_map<string, int> filter_map;

    // pulls the resouce file for refining
    json root;
    recipe_in >> root;
    recipe_in.close();

    json terminal_root;
    terminal_in >> terminal_root;
    terminal_in.close();
    unordered_set<string> terminal_resources;
    // These are chained terminal recipes
    // They make everything that uses them terminal
    terminal_resources.insert("Heavy Modular Frame");
    terminal_resources.insert("Radio Control Unit");
    terminal_resources.insert("Battery");
    terminal_resources.insert("Diamonds");

    unordered_set<string> nuclear_resources;
    // These are chained nuclear recipes
    // They make everything that uses them nuclear
    nuclear_resources.insert("Uranium Fuel Rod");

    unordered_set<string> capstone_resources;

    // all of the objects needed to hold the data while processing it.
    json recipe_object = json::object();
    json dataOut = json::array();

    // an empty json array to make the .value() function work
    json empty_array = json::array();

    // the storage for a give item and its possible ingredients
    string product;
    unordered_set<string> ingredients;

    // the maps the program uses to run
    unordered_map<string, unordered_set<string>> recipe_form; // the form that recipes use
    unordered_map<string, unordered_set<string>> vanilla_recipe_form; // the form that recipes use using only default recipes
    unordered_map<string, unordered_set<string>> algorithm_form; // the form that the kahn algorith wants
    unordered_map<string, unordered_set<string>> vanilla_algorithm_form; // the form that the kahn algorith wants using only default recipes
    unordered_map<string, int> layer; // how many dependent recipes are left to add to the queue.
    unordered_map<string, int> vanilla_layer; // how many dependent recipes are left to add to the queue using only default recipes
    unordered_map<string, int> usage_count; // how many times is an item used in a recipe
    bool first = true; // is it the first recipe in the section?

    // The queue used during sorting and the output vector
    queue<string> sorter;
    vector<string> sorted;

    for (const auto& category : root) {
        first = true;
        product = category.value("Category", "");
        recipe_form[product];
        for (const auto& recipe : category["Data"]) {
            if (first) {
                for (const auto& value : recipe["Ingredients"]) {
                    vanilla_recipe_form[product].insert(value.value("ItemClass", ""));
                }
                first = false;
            }
            for (const auto& value : recipe["Ingredients"]) {
                recipe_form[product].insert(value.value("ItemClass", ""));
            }
        }

        // Outputs used to analyse and filter the item list
        // This will be what determines the items that get turned into a chain
        
        if (((product.find("fuel") != string::npos || product.find("Fuel") != string::npos) && product.find("Ficsonium Fuel Rod") == string::npos)
        || product.find("Heavy Oil Residue") != string::npos || product.find("Petroleum Coke") != string::npos || product.find("Copper Powder") != string::npos
        || product.find("Matter") != string::npos || product.find("Acid") != string::npos) { }
        else if (product.find("matter") != string::npos || product.find("Matter") != string::npos) {
            item_analysis << product << endl;
        }
        else {
            item_out << product << endl;
        }
    }

    for (auto& [product, ingredients] : recipe_form) {
        layer[product] = ingredients.size();
        usage_count.insert({product, 0});
        for (auto& ingredient : ingredients) {
            algorithm_form[ingredient].insert(product);
            if (!layer.count(ingredient)) {
                layer[ingredient] = 0;
            }

            usage_count.insert({ingredient, 0});
            usage_count[ingredient] += 1;
        }
    }

    for (auto& [product, depth] : layer) {
        if (depth == 0) {
            sorter.push(product);
        }
    }

    while (!sorter.empty()) {
        // Extracts the current value and removes it from the queue
        string current = sorter.front();
        sorter.pop();

        // Adds the value to the results list
        sorted.push_back(current);

        if (algorithm_form.count(current)) { // True if the current value points to other things
            for (const auto& user : algorithm_form.at(current)) {
                // For each thing the current item points to, decrease the number of dependents each thing has 
                if (--layer[user] == 0) {
                    // If the thing has no more depandents, adds it to the queue
                    sorter.push(user);
                }
            }
        }
    }

    for (size_t i = 0; i < sorted.size(); i++) {
        for (const auto& category : root) {
            if (category.value("Category", "") == sorted.at(i)) {
                dataOut.push_back(category);
            }
        }
    }

    for (auto& [product, ingredients] : vanilla_recipe_form) {
        vanilla_layer[product] = ingredients.size();
        for (auto& ingredient : ingredients) {
            vanilla_algorithm_form[ingredient].insert(product);
            if (!vanilla_layer.count(ingredient)) {
                vanilla_layer[ingredient] = 0;
            }
        }
    }

    for (auto& [product, depth] : vanilla_layer) {
        if (depth == 0) {
            sorter.push(product);
        }
    }

    while (!sorter.empty()) {
        // Extracts the current value and removes it from the queue
        string current = sorter.front();
        sorter.pop();

        int max_tier = filter_map.count(current) ? filter_map[current] : 0;
        if (vanilla_algorithm_form.count(current)) { // True if the current value points to other things
            for (const auto& user : vanilla_algorithm_form.at(current)) {
                // For each product that uses the current ingredient, assign it a tier one higher
                int product_tier = max_tier + 1;
                if (!filter_map.count(user) || filter_map[user] < product_tier) {
                    filter_map[user] = product_tier;
                }
                
                if (--vanilla_layer[user] == 0) {
                    // If the product has no more ingredient dependencies, adds it to the queue
                    sorter.push(user);
                }
            }
        }
        if (max_tier != 0) {
            tier_object["ItemClass"] = current;
            tier_object["Depth"] = max_tier - 1;
            tier_array.push_back(tier_object);
        }
        else if (current == "Excited Photonic Matter") {
            tier_object["ItemClass"] = current;
            tier_object["Depth"] = max_tier;
            tier_array.push_back(tier_object);
        }
    }

    /*
    for (auto& [item, count] : usage_count) {
        if (item.find("Ore") != string::npos || item.find("SAM") != string::npos || item.find("Coal") != string::npos
        || item.find("Uranium") != string::npos || item.find("Plutonium") != string::npos || item.find("Ficsonium") != string::npos) { }
        else if (terminal_resources.find(item) != terminal_resources.end()) { }
        else if (count == 2) {
            item_analysis << item << " --> ";
            if (algorithm_form.count(item)) {
                for (auto& items : algorithm_form.at(item)) {
                    item_analysis << items << ", ";
                }
            }
            else {
                item_analysis << "nothing";
            }
            item_analysis << endl;
        }
    }
    */

    // Transfers statuses from ingredients to products
    for (const auto& category : dataOut) {
        string product = category.value("Category", "");
        unordered_set<string> ingredients = recipe_form.at(product);
        for (auto& ingredient : ingredients) {
            if (nuclear_resources.find(ingredient) != nuclear_resources.end()) {
                nuclear_resources.insert(product);
            }
            else if (terminal_resources.find(ingredient) != terminal_resources.end()) {
                if (product.find("Uranium") == string::npos && product.find("Plutonium") == string::npos && product.find("Ficsonium") == string::npos) {
                    terminal_resources.insert(product);
                }
            }
        }
        if (algorithm_form.find(product) == algorithm_form.end()) {
            capstone_resources.insert(product);
        }
    }
    
    // These are independent terminals
    // They do not make anything else terminal
    terminal_resources.insert("Electromagnetic Control Rod");
    terminal_resources.insert("Encased Uranium Cell");
    terminal_resources.insert("Ficsite Trigon");
    terminal_resources.insert("Aluminum Ingot");
    terminal_resources.insert("Aluminum Casing");
    terminal_resources.insert("Alclad Aluminum Sheet");
    terminal_resources.insert("Reanimated SAM");
    terminal_resources.insert("High-Speed Connector");
    terminal_resources.insert("Crystal Oscillator");
    
    json resource_data = json::object();
    resource_data["ItemClass"] = "";
    resource_data["Amount"] = "0"; // sets amount to 0;
    cout << "Progagated Terminal Resources: " << terminal_resources.size() << endl;
    for (auto& item : terminal_resources) {
        resource_data["ItemClass"] = item; // adds the display name
        terminal_root.push_back(resource_data);
    }
    ofstream terminal_out(exePath / "dat" / "terminal_resources.json");
    terminal_out << terminal_root.dump(4);
    terminal_out.close();

    json base_root;
    for (auto& item : terminal_root) {
        product = item["ItemClass"];
        bool unique = true;
        for (size_t i = 0; i < dataOut.size(); i++) {
            if (dataOut.at(i).value("Category", "") == product) {
                unique = false;
                break;
            }
        }
        if (unique) {
            resource_data["ItemClass"] = product; // adds the display name
            base_root.push_back(resource_data);
        }
    }
    cout << "Base Resources: " << base_root.size() << endl;
    ofstream base_out(exePath / "dat" / "base_resources.json");
    base_out << base_root.dump(4);
    base_out.close();

    json nuclear_root;
    cout << "Nuclear Resources: " << nuclear_resources.size() << endl;
    for (auto& item : nuclear_resources) {
        resource_data["ItemClass"] = item; // adds the display name
        nuclear_root.push_back(resource_data);
    }
    ofstream nuclear_out(exePath / "dat" / "nuclear_resources.json");
    nuclear_out << nuclear_root.dump(4);
    nuclear_out.close();

    json true_capstone_root;
    cout << "True Capstone Resources: " << capstone_resources.size() << endl;
    for (auto& item : capstone_resources) {
        resource_data["ItemClass"] = item; // adds the display name
        true_capstone_root.push_back(resource_data);
    }
    ofstream true_capstone_out(exePath / "int" / "true_capstone_resources.json");
    true_capstone_out << true_capstone_root.dump(4);
    true_capstone_out.close();

    json capstone_root;
    capstone_resources.insert(terminal_resources.begin(), terminal_resources.end());
    for (auto& item : capstone_resources) {
        resource_data["ItemClass"] = item; // adds the display name
        capstone_root.push_back(resource_data);
    }
    ofstream capstone_out(exePath / "dat" / "capstone_resources.json");
    capstone_out << capstone_root.dump(4);
    capstone_out.close();

    recipe_out << dataOut.dump(4);
    tier_out << tier_array.dump(4);

    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    test_recipe_in.close();

    bool remake_filters = test_recipe_root[4].value("remake_filters", false); // whether or not to remake the filter
    if (remake_filters) {
        ofstream filter_out(exePath / "dat" / "item_filters.json");
        json filter_array = json::array();
        for (const auto& data : dataOut) {
            resource_data["ItemClass"] = data.value("Category", "Error");
            filter_array.push_back(resource_data);
        }
        filter_out << filter_array.dump(4);
        filter_out.close();
    }

    // closes all the opened files
    recipe_out.close();
    item_out.close();
    item_analysis.close();
    tier_out.close();

    return 0;
}