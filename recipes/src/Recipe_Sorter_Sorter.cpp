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
    
    if (!recipe_in.is_open()) {
        cerr << "Failed to open recipe input file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
    }

    if (!item_out.is_open()) {
        cerr << "Failed to open item output file.\n";
    }

    if (!item_analysis.is_open()) {
        cerr << "Failed to open item analysis file.\n";
    }


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
    unordered_map<string, unordered_set<string>> algorithm_form; // the form that the kahn algorith wants
    unordered_map<string, int> layer; // how many dependent recipes are left to add to the queue.

    // The queue used during sorting and the output vector
    queue<string> sorter;
    vector<string> sorted;

    for (const auto& category : root) {
        product = category.value("Category", "");
        for (const auto& recipe : category["Data"]) {
            for (const auto& value : recipe["Ingredients"]) {
                recipe_form[product].insert(value.value("ItemClass", ""));
            }
        }

        // Outputs used to analyse and filter the item list
        // This will be what determines the items that get turned into a chain
        
        if ((product.find("fuel") != string::npos || product.find("Fuel") != string::npos) && product.find("Ficsonium Fuel Rod") == string::npos
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

    for (int i = 0; i < sorted.size(); i++) {
        for (const auto& category : root) {
            if (category.value("Category", "") == sorted.at(i)) {
                dataOut.push_back(category);
            }
        }
    }

    recipe_out << dataOut.dump(4);

    // closes all the opened files
    recipe_in.close();
    recipe_out.close();
    item_out.close();
    item_analysis.close();

    return 0;
}