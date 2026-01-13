#include "json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

string parseData(const string data, vector<string> Class_Name, vector<string> Display_Name);

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream resource_in(exePath / "name_pairs.json");
    ifstream recipe_in(exePath / "recipes_raw.json");
    ofstream recipe_out(exePath / "recipes_fixed.json");
    ofstream recipe_names(exePath / "original_recipe_names.txt");

    if (!resource_in.is_open()) {
        cerr << "Failed to open name input file.\n";
    }

    if (!recipe_in.is_open()) {
        cerr << "Failed to open recipe input file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
    }

    if (!recipe_names.is_open()) {
        cerr << "Failed to open recipe name output file.\n";
    }

    // pulls the resource file for usage
    json resource;
    resource_in >> resource;

    // pulls the resouce file for refining
    json recipe;
    recipe_in >> recipe;

    json recipe_data = json::object();
    json dataOut = json::array();

    string ingredient;
    vector<string> Class_Name;
    vector<string> Display_Name;

    // puts the data in resource into parallel vectors
    for (const auto& data : resource) {
        Class_Name.push_back(data.value("ClassName", ""));
        Display_Name.push_back(data.value("mDisplayName", ""));
    }

    for (auto& block : recipe) {
        for (auto& data : block["Ingredients"]) {
            ingredient = data.value("ItemClass", ""); // pulls the data
            data["ItemClass"] = parseData(ingredient, Class_Name, Display_Name); // makes the data usable
        }
        for (auto& data : block["Product"]) {
            ingredient = data.value("ItemClass", ""); // pulls the data
            data["ItemClass"] = parseData(ingredient, Class_Name, Display_Name); // makes the data usable
        }
        ingredient = block.value("ProducedIn", ""); // pulls the data
        block["ProducedIn"] = parseData(ingredient, Class_Name, Display_Name); // makes the data usable

        recipe_names << block.value("DisplayName", "") << endl;
    }

    // puts the new recipes in the output file
    recipe_out << recipe.dump(4);

    // closes all the opened files
    resource_in.close();
    recipe_in.close();
    recipe_out.close();
}

string parseData(const string data, vector<string> Class_Name, vector<string> Display_Name) {
    string result; // the usable data being output
    bool found = false; // is something found

    for (int i = 0; i < Class_Name.size(); i++) {
        if (data.find(Class_Name.at(i)) != string::npos) {
            result = Display_Name.at(i);
            found = true;
        }
    }

    if (!found) {
        result = "N/A";
    }

    return result;
}