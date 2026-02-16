#include "lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream resource_name(exePath / "dat/name_pairs.json");
    ifstream recipe_in(exePath / "int/recipes_trimmed.json");
    ofstream recipe_out(exePath / "dat/recipes.json");


    if (!resource_name.is_open()) {
        cerr << "Failed to open resource name input file.\n";
    }
    
    if (!recipe_in.is_open()) {
        cerr << "Failed to open recipe input file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
    }

    // pulls the resouce file for refining
    json recipe;
    recipe_in >> recipe;

    json name_pairs;
    resource_name >> name_pairs;

    json recipe_data = json::object();
    json recipe_group = json::array();
    json recipe_object = json::object();
    json dataOut = json::array();

    json empty_array = json::array();

    string product;
    vector<string> names;

    int original_count = 0;
    int new_count = 0;

    for (const auto& pair : name_pairs) {
        names.push_back(pair.value("mDisplayName", ""));
    }

    for (const auto& block : recipe) {
        original_count += 1;
    }

    for (int i = 0; i < names.size(); i++) {
        product = names.at(i);

        for (const auto& block : recipe) {
            recipe_data = block.value("Product", empty_array).at(0);
            if (product == recipe_data.value("ItemClass", "")
            && (block.value("DisplayName", "").find("Alternate:") == string::npos)) {
                recipe_group.push_back(block);
                new_count += 1;
            }
        }
        
        for (const auto& block : recipe) {
            recipe_data = block.value("Product", empty_array).at(0);
            if (product == recipe_data.value("ItemClass", "")
            && (block.value("DisplayName", "").find("Alternate:") != string::npos)) {
                recipe_group.push_back(block);
                new_count += 1;
            }
        }

        if (recipe_group.size() == 0) {
            continue;
        }

        recipe_object["Category"] = product;
        recipe_object["Data"] = recipe_group;
        dataOut.push_back(recipe_object);
        recipe_group.clear();
    }

    // puts the sorted recipes in the output file
    recipe_out << dataOut.dump(4);

    cout << original_count << " turns into " << new_count << endl;

    // closes all the opened files
    resource_name.close();
    recipe_in.close();
    recipe_out.close();

    return 0;
}