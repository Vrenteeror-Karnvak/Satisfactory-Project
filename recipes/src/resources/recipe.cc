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
    
    for (auto& ingredient : data["Ingredients"]) {
        Resource stuff(ingredient);
        ingredients.push_back(stuff);
    }

    for (auto& product : data["Product"]) {
        Resource stuff(product);
        products.push_back(stuff);
    }
}

void Recipe::set_name(string title) {
    name = title;
}

void Recipe::set_factory(string building) {
    factory = building;
}

void Recipe::set_ipm(double rate) {
    ipm = rate;
}

void Recipe::set_ingredients(vector<Resource> ingredient) {
    for (size_t i = 0; i < ingredient.size(); i++) {
        ingredients.push_back(ingredient.at(i));
    }
}

void Recipe::add_ingredient(Resource ingredient) {
    ingredients.push_back(ingredient);
}

void Recipe::set_products(vector<Resource> product) {
    for (size_t i = 0; i < product.size(); i++) {
        products.push_back(product.at(i));
    }
}

void Recipe::add_product(Resource product) {
    products.push_back(product);
}

string Recipe::get_name() {
    return name;
}

string Recipe::get_factory() {
    return factory;
}

double Recipe::get_ipm() {
    return ipm;
}

vector<Resource> Recipe::get_ingredients() {
    return ingredients;
}

Resource Recipe::get_ingredient(int i) {
    return ingredients.at(i);
}

vector<Resource> Recipe::get_products() {
    return products;
}

Resource Recipe::get_product(int i) {
    return products.at(i);
}

#endif