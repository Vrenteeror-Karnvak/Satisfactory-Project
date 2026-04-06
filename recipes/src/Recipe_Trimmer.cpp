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
    ifstream recipe_in(exePath / "int/recipes_fixed.json");
    ofstream recipe_out(exePath / "int/recipes_trimmed.json");
    ofstream removed_recipes_out(exePath / "int/recipes_removed.json");
    ofstream recipe_names(exePath / "int/recipe_names.txt");

    if (!recipe_in.is_open()) {
        cerr << "Failed to open recipe input file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
    }

    if (!recipe_names.is_open()) {
        cerr << "Failed to open recipe name output file.\n";
    }

    // pulls the resouce file for refining
    json recipe;
    recipe_in >> recipe;

    json recipe_data = json::object();
    json removed_recipes = json::array();
    json dataOut = json::array();
    
    // Used to add the conversion from fuel rods to waste
    json nuclear_object = json::object();
    json water_object = json::object();

    string ingredient;
    vector<string> Class_Name;
    vector<string> Display_Name;

    bool to_delete = false;

    for (const auto& block : recipe) {
        to_delete = false;
        for (auto& data : block["Ingredients"]) {
            if (data.value("ItemClass", "").find("Bio") != string::npos && block.value("DisplayName", "").find("Gas Nobelisk") == string::npos) {
                to_delete = true;
            }
            if (data.value("ItemClass", "").find("Dissolved Silica") != string::npos
            || data.value("ItemClass", "").find("Wood") != string::npos
            || data.value("ItemClass", "").find("Leaves") != string::npos
            || data.value("ItemClass", "").find("Packaged") != string::npos) {
                to_delete = true;
            }
        }

        for (auto& data : block["Product"]) {
            if (data.value("ItemClass", "").find("Biomass") != string::npos
            || data.value("ItemClass", "").find("Dissolved Silica") != string::npos
            || data.value("ItemClass", "").find("Packaged") != string::npos
            || data.value("ItemClass", "").find("Alien Protein") != string::npos
            || data.value("ItemClass", "").find("Alien DNA Capsule") != string::npos) {
                to_delete = true;
            }
        }

        if (block.value("DisplayName", "").find("Recycled") != string::npos
        || (block.value("DisplayName", "").find("Ficsite Ingot") == string::npos && block.value("DisplayName", "").find("(") != string::npos)) {
            to_delete = true;
        }

        if (!to_delete) {
            recipe_data = block;
            dataOut.push_back(recipe_data);
            recipe_names << block.value("DisplayName", "") << endl;
        }
        else {
            recipe_data = block;
            removed_recipes.push_back(recipe_data);
        }
    }

    // Adds the Uranium Fuel Rod conversion
    recipe_data["DisplayName"] = "Uranium Fuel Rod (Burning)";
    nuclear_object["ItemClass"] = "Uranium Fuel Rod";
    nuclear_object["Amount"] = "1";
    water_object["ItemClass"] = "Water";
    water_object["Amount"] = "1200";
    recipe_data["Ingredients"] = {nuclear_object, water_object};
    nuclear_object["ItemClass"] = "Uranium Waste";
    nuclear_object["Amount"] = "50";
    recipe_data["Product"] = {nuclear_object};
    recipe_data["ProducedIn"] = "Nuclear Power Plant";
    recipe_data["ManufactoringDuration"] = "300";
    dataOut.push_back(recipe_data);

    // Adds the Plutonium Fuel Rod conversion
    recipe_data["DisplayName"] = "Plutonium Fuel Rod (Burning)";
    nuclear_object["ItemClass"] = "Plutonium Fuel Rod";
    nuclear_object["Amount"] = "1";
    water_object["ItemClass"] = "Water";
    water_object["Amount"] = "2400";
    recipe_data["Ingredients"] = {nuclear_object, water_object};
    nuclear_object["ItemClass"] = "Plutonium Waste";
    nuclear_object["Amount"] = "10";
    recipe_data["Product"] = {nuclear_object};
    recipe_data["ProducedIn"] = "Nuclear Power Plant";
    recipe_data["ManufactoringDuration"] = "600";
    dataOut.push_back(recipe_data);

    // puts the new recipes in the output file
    removed_recipes_out << removed_recipes.dump(4);
    recipe_out << dataOut.dump(4);

    // closes all the opened files
    recipe_in.close();
    recipe_out.close();
    recipe_names.close();
}