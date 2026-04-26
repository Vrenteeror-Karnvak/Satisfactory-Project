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
    ifstream space_elevator_in(exePath / "dat/space_elevator_items.json");
    ofstream recipe_out(exePath / "int/recipes_trimmed.json");
    ofstream removed_recipes_out(exePath / "int/recipes_removed.json");
    ofstream consumable_recipe(exePath/ "int/consumable_recipes.json");
    ofstream recipe_names(exePath / "int/recipe_names.txt");

    // pulls the resouce file for refining
    json recipe;
    recipe_in >> recipe;

    // pulls the space elevator part list
    json space_elevator_parts;
    space_elevator_in >> space_elevator_parts;
    vector<string> space_elevator;
    for (const auto& data : space_elevator_parts) {
        if (data.value("ItemClass", "") != "Nuclear Pasta") {
            space_elevator.push_back(data.value("ItemClass", ""));
        }
    }

    json recipe_data = json::object();
    json removed_recipes = json::array();
    json consumable_items = json::array();
    json dataOut = json::array();
    
    // Used to add the conversion from fuel rods to waste
    json nuclear_object = json::object();
    json water_object = json::object();

    string ingredient;
    int amount;
    vector<string> Class_Name;
    vector<string> Display_Name;
    vector<string> fluids;

    bool to_delete = false; // Is the recipe to be removed?
    bool consumable = false; // Does the recipe make a consumable?

    for (const auto& block : recipe) {
        if (block.value("DisplayName", "").find("Packaged") != string::npos) {
            fluids.push_back(block["Ingredients"].at(0).value("ItemClass", ""));
        }
    }
    fluids.push_back("Dissolved Silica");
    fluids.push_back("Excited Photonic Matter");
    fluids.push_back("Dark Matter Residue");
    
    for (auto& block : recipe) {
        for (auto& data : block["Ingredients"]) {
            ingredient = data.value("ItemClass", "");
            if (find(fluids.begin(), fluids.end(), ingredient) != fluids.end()) {
                amount = stoi(data.value("Amount", ""));
                amount /= 1000;
                data["Amount"] = to_string(amount);
            }
        }

        for (auto& data : block["Product"]) {
            ingredient = data.value("ItemClass", "");
            if (find(fluids.begin(), fluids.end(), ingredient) != fluids.end()) {
                amount = stoi(data.value("Amount", ""));
                amount /= 1000;
                data["Amount"] = to_string(amount);
            }
        }
    }

    for (const auto& block : recipe) {
        to_delete = false;
        consumable = false;
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
            || data.value("ItemClass", "").find("fuel") != string::npos
            || (data.value("ItemClass", "").find("Fuel") != string::npos && data.value("ItemClass", "") != "Fuel")
                && data.value("ItemClass", "").find("Fuel Rod") == string::npos) {
                to_delete = true;
            }

            ingredient = data.value("ItemClass", "");
            if (find(space_elevator.begin(), space_elevator.end(), ingredient) != space_elevator.end()) {
                to_delete = true;
            }
        }

        if (block.value("DisplayName", "").find("Recycled") != string::npos
        || (block.value("DisplayName", "").find("Ficsite Ingot") == string::npos && block.value("DisplayName", "").find("(") != string::npos)) {
            to_delete = true;
        }

        if (!to_delete) {
            for (auto& data : block["Product"]) {
                if (data.value("ItemClass", "").find("Ammo") != string::npos
                || data.value("ItemClass", "").find("Rebar") != string::npos
                || data.value("ItemClass", "").find("Nobelisk") != string::npos
                || data.value("ItemClass", "").find("Empty") != string::npos
                || data.value("ItemClass", "").find("Filter") != string::npos
                || data.value("ItemClass", "").find("Fabric") != string::npos
                || data.value("ItemClass", "").find("Alien Power Matrix") != string::npos) {
                    to_delete = true;
                    consumable = true;
                }
            }
        }

        if (!to_delete) {
            recipe_data = block;
            dataOut.push_back(recipe_data);
            recipe_names << block.value("DisplayName", "") << endl;
        }
        else if (consumable) {
            recipe_data = block;
            consumable_items.push_back(recipe_data);
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
    consumable_recipe << consumable_items.dump(4);
    recipe_out << dataOut.dump(4);

    // closes all the opened files
    recipe_in.close();
    recipe_out.close();
    removed_recipes_out.close();
    consumable_recipe.close();
    recipe_names.close();
}