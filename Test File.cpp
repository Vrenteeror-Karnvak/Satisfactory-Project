#include "json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <typeinfo>

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    // opens all of the input and output file streams
    ifstream recipe_in(exePath / "recipes.json");
    ofstream recipe_out(exePath / "recipes_ipm.json");

    json recipe;
    recipe_in >> recipe;

    double value = 0;

    for (auto& block : recipe) {
        value = stod(block.value("ManufactoringDuration", ""));
        block["ManufactoringDuration"] = to_string(60 / value);
    }

    recipe_out << recipe.dump(4);

    recipe_in.close();
    recipe_out.close();
}