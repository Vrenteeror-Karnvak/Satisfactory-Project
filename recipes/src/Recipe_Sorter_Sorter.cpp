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
    
    if (!recipe_in.is_open()) {
        cerr << "Failed to open recipe input file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
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

        // Clears the list
        ingredients.clear();
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

    return 0;
}