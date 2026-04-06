#include "lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

/**
 * @brief Pulls ingredient name and amount from string .
 * 
 * @param data A list of ingredients as a string.
 * @return A vector<json> containing a list of ingredients: name and amount.
 */
vector<json> parseIngredients(const string data);

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream fin(exePath / "dat/en-US.json");
    ofstream name_out(exePath / "dat/name_pairs.json");
    ofstream resource_out(exePath / "dat/terminal_resources.json");
    ofstream recipe_out(exePath / "int/recipes_raw.json");

    if (!fin.is_open()) {
        cerr << "Failed to open input file.\n";
        return 0;
    }

    if (!name_out.is_open()) {
        cerr << "Failed to open name output file.\n";
        return 0;
    }

    if (!resource_out.is_open()) {
        cerr << "Failed to open resource output file.\n";
    }

    if (!recipe_out.is_open()) {
        cerr << "Failed to open recipe output file.\n";
        return 0;
    }

    // pulls the full file for processing
    json root;
    fin >> root;

    json name_data = json::object();
    json resource_data = json::object();
    json recipe_data = json::object();
    json nameOut = json::array();
    json resourceOut = json::array();
    json recipeOut = json::array();

    string ingredients;
    vector<string> Class_Name;
    vector<string> Display_Name;

    for (const auto& block : root) {
        // if the value of "NativeClass" is correct for the item, equipment, and building description section, this is true
        if (block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptor'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGResourceDescriptor'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGPowerShardDescriptor'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptorBiomass'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptorNuclearFuel'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGEquipmentDescriptor'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGAmmoTypeProjectile'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGItemDescriptorPowerBoosterFuel'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGAmmoTypeInstantHit'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGAmmoTypeSpreadshot'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGBuildableManufacturer'"
        || block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGBuildableManufacturerVariablePower'") {
            // iterates over all objects to pull the needed data
            for (const auto& data : block["Classes"]) {
                name_data["ClassName"] = data.value("ClassName", ""); // grabs the class name that shows up in the recipe data
                name_data["mDisplayName"] = data.value("mDisplayName", ""); // grabs the display name which is human readable

                nameOut.push_back(name_data); // adds the collected data to the vector
            }
        }

        if (block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGResourceDescriptor'") {
            for (const auto& data : block["Classes"]) {
                resource_data["ItemClass"] = data.value("mDisplayName", ""); // adds the display name
                resource_data["Amount"] = "0"; // sets amount to 0;
                resourceOut.push_back(resource_data); // adds the collected data to the vector
            }
        }

        // if the value of "NativeClass" is correct for the recipe section, this is true
        if (block.value("NativeClass", "") == "/Script/CoreUObject.Class'/Script/FactoryGame.FGRecipe'") {
            // iterates over all objects to pull the needed data
            // if the data needs processing to be usable, it is done here
            for (const auto& data : block["Classes"]) {
                if (data.value("mProducedIn", "") != "(\"/Game/FactoryGame/Equipment/BuildGun/BP_BuildGun.BP_BuildGun_C\")" 
                && data.value("mProducedIn", "") != "(\"/Game/FactoryGame/Buildable/-Shared/WorkBench/BP_WorkshopComponent.BP_WorkshopComponent_C\")"
                && data.value("mProducedIn", "") != "(\"/Script/FactoryGame.FGBuildGun\")"
                && data.value("mProducedIn", "") != "") {
                    recipe_data["DisplayName"] = data.value("mDisplayName", ""); // grabs the display name which is human readable

                    ingredients = data.value("mIngredients", ""); // grabs and processes the ingredient data into usable form
                    recipe_data["Ingredients"] = parseIngredients(ingredients);

                    ingredients = data.value("mProduct", ""); // grabs and processes the product data into usable form
                    recipe_data["Product"] = parseIngredients(ingredients);

                    recipe_data["ProducedIn"] = data.value("mProducedIn", ""); // grabs where the part is made

                    recipe_data["ManufactoringDuration"] = data.value("mManufactoringDuration", ""); // grabs the manufacturing time

                    recipeOut.push_back(recipe_data); // adds the collected data to the vector
                }
            }            
        }
    }
    
    resource_data["ItemClass"] = "Polymer Resin"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "Compacted Coal"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "Dark Matter Residue"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "Heavy Oil Residue"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "Sulfuric Acid"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "FICSMAS Gift"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    resource_data["ItemClass"] = "Biomass"; // adds the display name
    resource_data["Amount"] = "0"; // sets amount to 0;
    resourceOut.push_back(resource_data); // adds the  data to the vector

    // outputs the collected data
    name_out << nameOut.dump(4);
    resource_out << resourceOut.dump(4);
    recipe_out << recipeOut.dump(4);

    // closes all the opened files
    fin.close();
    name_out.close();
    recipe_out.close();

    return 0;
}

vector<json> parseIngredients(string data) {
    vector<json> result; // the usable data being output

    // removes the double parathesises at the front and back
    if (data.size() >= 4 && data.front() == '(' && data[1] == '(') {
        data = data.substr(2, data.size() - 4);
    }

    // seperates each of the pieces of data
    size_t start = 0;
    while (start < data.size()) { // runs while start is not located at the end of data
        size_t end = data.find("),(", start); // finds the next seperator
        string part; // the data wanted
        if (end == string::npos) { // true if there is not another seperator found
            part = data.substr(start); // pulls only the part of the initial string wanted
            start = data.size(); // sets start to the end of data
        }
        else {
            part = data.substr(start, end - start); // pulls only the part of the initial string wanted
            start = end + 3; // sets start to the previous end, skipping the "),(", to get the next piece of data
        }

        // Extracts ItemClass
        json ingredient; // the ingredient being pulled
        size_t itemStart = part.find("ItemClass=\""); // finds the begining of item name
        if (itemStart != string::npos) { 
            itemStart += 11; // increases itemStart by the length of "ItemClass=\"", effectively removing it
            size_t itemEnd = part.find("\"", itemStart); // finds the end of the item name
            ingredient["ItemClass"] = part.substr(itemStart, itemEnd - itemStart); // adds ItemClass to the json object
        }

        // Extracts Amount
        size_t amountStart = part.find("Amount=");
        if (amountStart != string::npos) {
            amountStart += 7; // increases amountStart by the length of length of "Amount=", effectively removing it
            ingredient["Amount"] = part.substr(amountStart); // adds Amount to the json object
        }

        result.push_back(ingredient); // adds the json object to the vector
    }

    return result;
}