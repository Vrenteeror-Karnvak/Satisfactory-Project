#include "lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "resources/recipe.h"
#include "resources/resource.h"

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream recipe_in(exePath / "dat" / "recipes.json");
    ofstream recipe_out(exePath / "int" / "byproducts.json");

    json recipe;
    recipe_in >> recipe;

    json byproducts = json::object();
    json double_output = json::array();

    bool byproduct_found = false;

    for (auto& block : recipe) {
        if (block["Product"].size() != 1) {
            byproduct_found = false;

            for (auto& product : block["Product"]) {
                if (product["ItemClass"] == "Compacted Coal" || product["ItemClass"] == "Polymer Resin"
                || product["ItemClass"] == "Dark Matter Residue" || product["ItemClass"] == "Water"
                || product["ItemClass"] == "Dissolved Silica" || product["ItemClass"] == "Sulfuric Acid"
                || product["ItemClass"] == "Heavy Oil Residue" || product["ItemClass"] == "Silica"
                || product["ItemClass"] == "Biomass")  {
                    byproduct_found = true;
                }
            }

            /*
            Label as terminal:
            Polymer Resin
            Compacted Coal
            Dark Matter Residue
            Water
            Heavy Oil Residue
            Sulfuric Acid
            Biomass
            
            Remove the associated recipes entirely:
            Dissolved Silica

            Needs more consideration:
            Silica
            */

            if (!byproduct_found) {
                byproducts["DisplayName"] = block["DisplayName"];
                byproducts["Product"] = block["Product"];
                double_output.push_back(byproducts);
            }
        }
    }

    recipe_out << double_output.dump(4);

    recipe_in.close();
    recipe_out.close();
}


// Hello World! It took me 8 minutes to figure out what a camera was.
// In my defense, I was asked to determine an item that was art related and a device.