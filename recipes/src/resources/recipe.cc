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
    bool found = false;
    // combines the ingredients
    for (int i = 0; i < other.get_ingredients().size(); i++) {
        found = false;
        for (int j = 0; j < ingredients.size(); j++) {
            if (ingredients.at(j) == other.get_ingredient(i)) {
                ingredients.at(j) += other.get_ingredient(i);
                found = true;
                break;
            }
        }
        if (!found) {
            ingredients.push_back(other.get_ingredient(i));
        }
    }

    // combines the products
    for (int i = 0; i < other.get_products().size(); i++) {
        found = false;
        for (int j = 0; j < products.size(); j++) {
            if (products.at(j) == other.get_product(i)) {
                products.at(j) += other.get_product(i);
                found = true;
                break;
            }
        }
        if (!found) {
            products.push_back(other.get_product(i));
        }
    }

    // Removes any similarities between the products and ingredients
    for (int i = 0; i < products.size(); i++) {
        for (int j = 0; j < ingredients.size(); j++) {
            if (products.at(i).same_name(ingredients.at(j))) {
                products.at(i) -= ingredients.at(j);
                ingredients.erase(find(ingredients.begin(), ingredients.end(), ingredients.at(j)));
            }
        }
    }

    // Removes any products that have an amount of 0
    // Also converts and products with a negative amount into ingredients
    for (int i = 0; i < products.size(); i++) {
        if (abs(products.at(i).get_amount()) < EPSILON) {
            products.erase(find(products.begin(), products.end(), products.at(i)));
        }
        else if (products.at(i).get_amount() < 0) {
            ingredients.push_back(products.at(i));
            products.erase(find(products.begin(), products.end(), products.at(i)));
        }
    }
}

void Recipe::merge_recipes(const vector<Recipe> data) {
    for (int i = 0; i < data.size(); i++) {
        this->combine_recipes(data.at(i));
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

json Recipe::to_compressed_json() const {
    json output = json::object();
    json current = json::object();

    output["DisplayName"] = products.at(0).get_name();
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
    return (name == other.get_name() && factory == other.get_factory()
            && machine_speed == other.get_machine_speed()
            && ingredients.size() == other.get_ingredients().size()
            && products.size() == other.get_products().size());
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

Recipe& Recipe::operator+=(const Recipe& other) {
    if (!same_name(other)) {
        throw invalid_argument("Cannot combine different recipes.");
    }

    for (int i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) += other.get_ingredient(i);
    }
    for (int i = 0; i < products.size(); i++) {
        products.at(i) += other.get_product(i);
    }
    return *this;
}

Recipe Recipe::operator+(const Recipe& other) const {
    Recipe result = *this;
    result += other;
    return result;
}

Recipe& Recipe::operator-=(const Recipe& other) {
    if (!same_name(other)) {
        throw invalid_argument("Cannot combine different recipes.");
    }

    for (int i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) -= other.get_ingredient(i);
    }
    for (int i = 0; i < products.size(); i++) {
        products.at(i) -= other.get_product(i);
    }
    return *this;
}

Recipe Recipe::operator-(const Recipe& other) const {
    Recipe result = *this;
    result -= other;
    return result;
}


Recipe& Recipe::operator*=(const double multiple) {
    for (int i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) *= multiple;
    }
    for (int i = 0; i < products.size(); i++) {
        products.at(i) *= multiple;
    }
    return *this;
}

Recipe Recipe::operator*(const double multiple) const {
    Recipe result = *this;
    result *= multiple;
    return result;
}

#endif