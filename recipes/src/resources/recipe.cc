#ifndef RECIPE_CC
#define RECIPE_CC

#include "recipe.h"

using namespace std;
using json = nlohmann::ordered_json;

Recipe::Recipe() {
    name = "N/A";
    factory = "N/A";
    ipm = 0;
}

Recipe::Recipe(const json& data) {
    name = data.value("DisplayName", "");
    factory = data.value("ProducedIn", "");
    ipm = stod(data.value("ManufactoringDuration", ""));
    
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
    ipm = stod(data.value("ManufactoringDuration", ""));
    
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
    ipm = 1;
    
    ingredients.clear();

    products.clear();
    products.push_back(product);
}

void Recipe::set_name(const string title) {
    name = title;
}

void Recipe::set_factory(const string building) {
    factory = building;
}

void Recipe::set_ipm(const double rate) {
    ipm = rate;
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

string Recipe::get_name() const {
    return name;
}

string Recipe::get_factory() const {
    return factory;
}

double Recipe::get_ipm() const {
    return ipm;
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

bool Recipe::is_processed() const {
    return processed;
}

#endif