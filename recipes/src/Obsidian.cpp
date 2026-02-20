#include "lib/json.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

using json = nlohmann::ordered_json;
using namespace std;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    filesystem::path outPath = exePath / "dat/satisfactory_vault";

    // opens all of the input and output file streams
    ifstream recipe_in(exePath / "dat/recipes.json");
    
    //get json
    json recipes;
    recipe_in >> recipes;
    recipe_in.close();

    //vars for read
    json object_in, array_in;
    string string_in, file_name;

    for (const auto& block : recipes) {
        object_in = block.at("Data").at(0);

        string_in = object_in.at("DisplayName");
        file_name = string_in+".md";
        if (!filesystem::exists(outPath / file_name)) {
            vector<string> ingredients;
            array_in = object_in.value("Ingredients", array_in);

            //create ingredients if not exist and catalogue them in vector
            for (const auto& ingredient : array_in) {
                string_in = ingredient.value("ItemClass","");
                file_name = string_in + ".md";

                if (string_in.length()==0) {
                    cout << "Failed to open ingredient\n";
                    return 1;
                }

                if (!filesystem::exists(outPath / file_name)) {
                    ofstream create_file(outPath / file_name);
                    create_file.close();
                }
                ingredients.push_back(string_in);
            }

            array_in = object_in.value("Product", array_in);

            //create products and link them to ingredients
            for (const auto& product : array_in) {
                string_in = product.value("ItemClass","");
                file_name = string_in+".md";

                if (string_in.length()==0) {
                    cout << "Failed to open product\n";
                    return 1;
                }

                if (!filesystem::exists(outPath / file_name)) {
                    ofstream create_file(outPath / file_name);
                    create_file.close();
                }

                ofstream write_file(outPath / file_name,ios::app);
                for (string ingredient : ingredients) {
                    write_file << "[[" << ingredient << "]]";
                }
                write_file.close();
            }
        }
    }
}