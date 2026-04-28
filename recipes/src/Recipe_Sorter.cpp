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
    ofstream recipe_out(exePath / "int/recipes_sorted.json");

    // pulls the resouce file for refining
    json recipe;
    recipe_in >> recipe;

    // pulls the names of all possible resources
    json name_pairs;
    resource_name >> name_pairs;

    // all of the objects needed to hold the data while processing it.
    json recipe_data = json::object();
    json recipe_group = json::array();
    json recipe_object = json::object();
    json dataOut = json::array();

    // an empty json array to make the .value() function work
    json empty_array = json::array();

    // the storage for all possible item names
    string product;
    vector<string> names;

    // counters for data collection
    int original_count = 0;
    int new_count = 0;
    int single_count = 0;
    int multi_count = 0;

    // adds all the possible item names to a vector
    for (const auto& pair : name_pairs) {
        names.push_back(pair.value("mDisplayName", ""));
    }

    // counts how many recipes were at the start to ensure none are lost.
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
            // if the array is empty, doesn't add it to the output
            continue;
        }

        recipe_object["Category"] = product;
        recipe_object["Data"] = recipe_group;

        if (recipe_group.size() == 1) {
            // if there is only a single recipe in the vector, adds it to a seperate category
            dataOut.push_back(recipe_object);
            single_count += 1;
        }
        else {
            // if there multiple recipes in the vector, adds it to a seperate category
            dataOut.push_back(recipe_object);
            multi_count += 1;
        }

        recipe_group.clear();
    }

    // puts the sorted recipes in the output file
    recipe_out << dataOut.dump(4);
    
    // outputs the collected data
    cout << original_count << " turns into " << new_count << endl;
    cout << "There are " << single_count << " items with only one recipe." << endl;
    cout << "There are " << multi_count << " items with multiple recipes." << endl;

    // closes all the opened files
    resource_name.close();
    recipe_in.close();
    recipe_out.close();

    return 0;
}