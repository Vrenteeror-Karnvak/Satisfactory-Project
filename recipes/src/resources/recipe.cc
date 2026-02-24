#ifndef RECIPE_CC
#define RECIPE_CC

#include "recipe.h"

using namespace std;
using json = nlohmann::ordered_json;

Recipe::Recipe() {
    name = "N/A";
    factory = "N/A";
    machine_speed = 0;
}

Recipe::Recipe(const json& data) {
    name = data.value("DisplayName", "");
    factory = data.value("ProducedIn", "");
    machine_speed = stod(data.value("ManufactoringDuration", ""));
    
    ingredients.clear();
    for (auto& ingredient : data["Ingredients"]) {
        Resource stuff(ingredient);
        ingredients.push_back(stuff);
    }

    products.clear();
    for (auto& product : data["Product"]) {
        Resource stuff(product);
        products.push_back(stuff);
    }
}

void Recipe::set_recipe(const json& data) {
    name = data.value("DisplayName", "");
    factory = data.value("ProducedIn", "");
    machine_speed = stod(data.value("ManufactoringDuration", ""));
    
    ingredients.clear();
    for (auto& ingredient : data["Ingredients"]) {
        Resource stuff(ingredient);
        ingredients.push_back(stuff);
    }

    products.clear();
    for (auto& product : data["Product"]) {
        Resource stuff(product);
        products.push_back(stuff);
    }

    processed = false;
}

void Recipe::set_terminal_recipe(const Resource product) {
    name = product.get_name() + " (Terminal)";
    factory = "Terminal Resource";
    machine_speed = 1;
    
    ingredients.clear();

    products.clear();
    products.push_back(product);

    processed = true;
}

void Recipe::set_name(const string title) {
    name = title;
}

void Recipe::set_factory(const string building) {
    factory = building;
}

void Recipe::set_machine_speed(const double rate) {
    machine_speed = rate;
}

void Recipe::set_ingredients(const vector<Resource> ingredient) {
    for (size_t i = 0; i < ingredient.size(); i++) {
        ingredients.push_back(ingredient.at(i));
    }
}

void Recipe::add_ingredient(const Resource ingredient) {
    ingredients.push_back(ingredient);
}

void Recipe::set_products(const vector<Resource> product) {
    for (size_t i = 0; i < product.size(); i++) {
        products.push_back(product.at(i));
    }
}

void Recipe::add_product(const Resource product) {
    products.push_back(product);
}

void Recipe::set_processed() {
    processed = true;
}

void Recipe::combine_recipes(const Recipe other) {
    if (name != other.get_name()) {
        // Terminates the process if the recipes aren't the same.
        cout << "These are not the same recipe. Can not combine." << endl;
        return;
    }

    for (int i = 0; i < ingredients.size(); i++) {
        ingredients.at(i).combine_resource(other.get_ingredient(i));
    }
    for (int i = 0; i < products.size(); i++) {
        products.at(i).combine_resource(other.get_product(i));
    }
}

string Recipe::get_name() const {
    return name;
}

string Recipe::get_factory() const {
    return factory;
}

double Recipe::get_machine_speed() const {
    return machine_speed;
}

vector<Resource> Recipe::get_ingredients() const {
    return ingredients;
}

Resource Recipe::get_ingredient(int i) const {
    return ingredients.at(i);
}

vector<Resource> Recipe::get_products() const {
    return products;
}

Resource Recipe::get_product(int i) const {
    return products.at(i);
}

json Recipe::to_json() const {
    json output = json::object();
    json current = json::object();

    output["DisplayName"] = name;
    for (int i = 0; i < ingredients.size(); i++) {
        current["ItemClass"] = ingredients.at(i).get_name();
        current["Amount"] = ingredients.at(i).get_amount();
        output["Ingredients"].push_back(current);
    }
    for (int i = 0; i < products.size(); i++) {
        current["ItemClass"] = products.at(i).get_name();
        current["Amount"] = products.at(i).get_amount();
        output["Product"].push_back(current);
    }
    output["ProducedIn"] = factory;
    output["ManufactoringDuration"] = machine_speed;
    return output;
}

void Recipe::set_to(const double end_result) {
    double multiple = end_result / products.at(0).get_amount();

    for (int i = 0; i < ingredients.size(); i++) {
        ingredients.at(i).set_amount(ingredients.at(i).get_amount() * multiple);
    }

    for (int i = 0; i < products.size(); i++) {
        products.at(i).set_amount(products.at(i).get_amount() * multiple);
    }
}

bool Recipe::is_processed() const {
    return processed;
}

bool Recipe::same_name(const Recipe& other) const {
    return (name == other.get_name() && factory == other.get_factory());
}

bool Recipe::operator==(const Recipe& other) const {
    // if name, factory, or machine speed are not equivalent, return false
    if (name != other.get_name() || factory != other.get_factory() || machine_speed != other.get_machine_speed()) {
        return false;
    }

    // if either the product or ingredient vectors aren't the same size, return false
    if (ingredients.size() != other.get_ingredients().size() || products.size() != other.get_products().size()) {
        return false;
    }

    // if any of the ingredients or products do not match, return false;
    for (int i = 0; i < ingredients.size(); i++) {
        if (ingredients.at(i) != other.get_ingredient(i)) {
            return false;
        }
    }
    for (int i = 0; i < products.size(); i++) {
        if (products.at(i) != other.get_product(i)) {
            return false;
        }
    }

    // if everything matches, return true
    // The processed flag is not compared as it is irrelevent
    return true;
}

bool Recipe::operator!=(const Recipe& other) const {
    return !(*this == other);
}

#endif