#include "recipe.h"
#include "resource.h"

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    ifstream recipe_in(exePath / "dat" / "recipes.json");

    json recipe_root;
    recipe_in >> recipe_root;

    vector<Recipe> recipe_list;

    for (auto& recipe : recipe_root) {
        Recipe recipe_input(recipe);
        recipe_list.push_back(recipe_input);
    }
}