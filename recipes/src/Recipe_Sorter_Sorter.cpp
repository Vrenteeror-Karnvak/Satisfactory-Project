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
    ofstream recipe_out(exePath / "dat/recipes.json");
    ofstream item_out(exePath / "int/item_list.txt");
    ofstream item_analysis(exePath / "int/item_analysis.txt");
    ofstream filter_out(exePath / "dat" / "item_filters.json");
    
    json filter_array = json::array();
    json filter_object = json::object();
    unordered_map<string, int> filter_map;

    // pulls the resouce file for refining
    json root;
    recipe_in >> root;

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
    bool first = true; // is it the first recipe in the section?

    // The queue used during sorting and the output vector
    queue<string> sorter;
    vector<string> sorted;

    for (const auto& category : root) {
        first = true;
        product = category.value("Category", "");
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
        for (auto& ingredient : ingredients) {
            algorithm_form[ingredient].insert(product);
            if (!layer.count(ingredient)) {
                layer[ingredient] = 0;
            }
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
            filter_object["ItemClass"] = current;
            filter_object["Depth"] = max_tier - 1;
            filter_array.push_back(filter_object);
        }
        else if (current == "Excited Photonic Matter") {
            filter_object["ItemClass"] = current;
            filter_object["Depth"] = max_tier;
            filter_array.push_back(filter_object);
        }
    }

    recipe_out << dataOut.dump(4);
    filter_out << filter_array.dump(4);

    // closes all the opened files
    recipe_in.close();
    recipe_out.close();
    item_out.close();
    item_analysis.close();
    filter_out.close();

    return 0;
}