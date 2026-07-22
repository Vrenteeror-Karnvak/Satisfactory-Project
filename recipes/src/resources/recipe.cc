#ifndef RECIPE_CC
#define RECIPE_CC

#include "recipe.h"

using namespace std;
using json = nlohmann::ordered_json;

Recipe::Recipe() {
    name = "N/A";
    factory = "N/A";
    machine_speed = 0;
    product_ID = SIZE_MAX;
}

Recipe::Recipe(const json& data, const size_t id) {
    vector<int> empty_vector;
    name = data.value("DisplayName", "Unknown");
    ID = data.value("ID", empty_vector);
    factory = data.value("ProducedIn", "N/A");
    machine_speed = stod(data.value("ManufactoringDuration", "0.0"));
    
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

    product_ID = id;
    processed = false;
}

void Recipe::set_recipe(const json& data, const size_t id) {
    vector<int> empty_vector;
    name = data.value("DisplayName", "Unknown");
    ID = data.value("ID", empty_vector);
    factory = data.value("ProducedIn", "N/A");
    machine_speed = stod(data.value("ManufactoringDuration", "0.0"));
    
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

    product_ID = id;
    processed = false;
}

void Recipe::set_terminal_recipe(const Resource product) {
    name = product.get_name() + " (Terminal)";
    factory = "Terminal Resource";
    machine_speed = 60;
    
    ingredients.clear();

    products.clear();
    products.push_back(product);

    processed = true;
}

void Recipe::set_name(const string& title) {
    name = title;
}

void Recipe::set_ID(const vector<int>& id) {
    ID = id;
}

void Recipe::set_factory(const string& building) {
    factory = building;
}

void Recipe::set_machine_speed(const double rate) {
    machine_speed = rate;
}

void Recipe::set_ingredients(const vector<Resource>& ingredient) {
    for (size_t i = 0; i < ingredient.size(); i++) {
        ingredients.push_back(ingredient[i]);
    }
}

void Recipe::add_ingredient(const Resource& ingredient) {
    ingredients.push_back(ingredient);
}

void Recipe::set_products(const vector<Resource>& product) {
    for (size_t i = 0; i < product.size(); i++) {
        products.push_back(product[i]);
    }
}

void Recipe::add_product(const Resource& product) {
    products.push_back(product);
}

void Recipe::set_product_ID(const size_t value) {
    product_ID = value;
}

void Recipe::set_processed() {
    processed = true;
}

void Recipe::combine_recipes(const Recipe& other) {
    bool found = false;
    // combines the ingredients
    const vector<Resource>& other_ingredients = other.get_ingredients_ref();
    for (size_t i = 0; i < other_ingredients.size(); i++) {
        found = false;
        for (size_t j = 0; j < ingredients.size(); j++) {
            if (ingredients[j].same_product_ID(other_ingredients[i])) {
                ingredients[j] += other_ingredients[i];
                found = true;
                break;
            }
        }
        if (!found) {
            ingredients.push_back(other_ingredients[i]);
        }
    }

    // combines the products
    const vector<Resource>& other_products = other.get_products_ref();
    for (size_t i = 0; i < other_products.size(); i++) {
        found = false;
        for (size_t j = 0; j < products.size(); j++) {
            if (products[j].same_product_ID(other_products[i])) {
                products[j] += other_products[i];
                found = true;
                break;
            }
        }
        if (!found) {
            products.push_back(other_products[i]);
        }
    }

    // Removes any similarities between the products and ingredients
    for (size_t i = 0; i < products.size(); i++) {
        for (size_t j = 0; j < ingredients.size(); j++) {
            if (products[i].same_product_ID(ingredients[j])) {
                products[i] -= ingredients[j];
                ingredients[j].set_amount(0);
                break;
            }
        }
    }

    // Remove ingredients that were cancelled out by products (rebuild to avoid index shifting)
    vector<Resource> cleaned_ingredients;
    cleaned_ingredients.reserve(ingredients.size());
    for (size_t i = 0; i < ingredients.size(); i++) {
        if (ingredients[i].get_amount() == 0) {
            // Skip zero-amount ingredients
            continue;
        }
        else {
            cleaned_ingredients.push_back(ingredients[i]);
        }
    }
    ingredients = cleaned_ingredients;

    // Removes any products that have an amount of 0
    // Also converts products with a negative amount into ingredients
    // Rebuild the vector to preserve original indices of byproducts
    vector<Resource> cleaned_products;
    cleaned_products.reserve(products.size());
    for (size_t i = 0; i < products.size(); i++) {
        if (products[i].get_amount() == 0) {
            // Skip zero-amount products
            continue;
        }
        else if (products[i].get_amount() < 0) {
            // Convert negative products to ingredients
            products[i] *= -1;
            ingredients.push_back(products[i]);
        }
        else {
            // Keep positive-amount products in their original order
            cleaned_products.push_back(products[i]);
        }
    }
    products = cleaned_products;
}

void Recipe::merge_recipes(const vector<Recipe>& data) {
    if (data.empty()) {
        return;
    }

    *this = data.back();
    for (size_t i = (data.size() - 1); i > 0; i--) {
        this->combine_recipes(data[i - 1]);
    }
}

void Recipe::set_primary_product(const string& primary_name) {
    // Find the product matching primary_name and move it to index 0
    for (size_t i = 0; i < products.size(); i++) {
        if (products[i].get_name() == primary_name) {
            // If not already at index 0, swap it to the front
            if (i != 0) {
                Resource temp = products[0];
                products[0] = products[i];
                products[i] = temp;
            }
            return;
        }
    }
    // If primary product not found, do nothing
}

string Recipe::get_name() const {
    return name;
}

vector<int>& Recipe::modify_ID() {
    return ID;
}

vector<int> Recipe::get_ID() const {
    return ID;
}

const vector<int>& Recipe::get_ID_ref() const {
    return ID;
}

string Recipe::get_factory() const {
    return factory;
}

double Recipe::get_machine_speed() const {
    return machine_speed;
}

vector<Resource>& Recipe::modify_ingredients() {
    return ingredients;
}

vector<Resource> Recipe::get_ingredients() const {
    return ingredients;
}

const vector<Resource>& Recipe::get_ingredients_ref() const {
    return ingredients;
}

Resource Recipe::get_ingredient(int i) const {
    return ingredients[i];
}

const Resource& Recipe::get_ingredient_ref(int i) const {
    return ingredients[i];
}

vector<Resource>& Recipe::modify_products() {
    return products;
}

vector<Resource> Recipe::get_products() const {
    return products;
}

const vector<Resource>& Recipe::get_products_ref() const {
    return products;
}

Resource Recipe::get_product(int i) const {
    return products[i];
}

const Resource& Recipe::get_product_ref(int i) const {
    return products[i];
}

size_t Recipe::get_product_ID() const {
    return product_ID;
}

json Recipe::to_json() const {
    json output = json::object();
    json current = json::object();
    json empty_array = json::array();
    string fraction;
    string string_ID;

    output["DisplayName"] = name;
    for (size_t i = 0; i < ID.size(); i++) {
        if (i != 0) {
            string_ID += '|';
        }
        string_ID += ID[i];
    }
    output["ID"] = string_ID;
    output["Ingredients"] = empty_array;
    output["Product"] = empty_array;
    for (size_t i = 0; i < ingredients.size(); i++) {
        current["ItemClass"] = ingredients[i].get_name();
        fraction = std::to_string(ingredients[i].get_amount().get_numerator()) + "/" + std::to_string(ingredients[i].get_amount().get_denominator());
        current["Amount"] = fraction;
        output["Ingredients"].push_back(current);
    }
    for (size_t i = 0; i < products.size(); i++) {
        current["ItemClass"] = products[i].get_name();
        fraction = std::to_string(products[i].get_amount().get_numerator()) + "/" + std::to_string(products[i].get_amount().get_denominator());
        current["Amount"] = fraction;
        output["Product"].push_back(current);
    }
    output["ProducedIn"] = factory;
    output["ManufactoringDuration"] = std::to_string(machine_speed);
    return output;
}

json Recipe::to_compressed_json() const {
    json output = json::object();
    json current = json::object();
    json empty_array = json::array();
    string string_ID;

    output["DisplayName"] = name;
    for (size_t i = 0; i < ID.size(); i++) {
        if (i != 0) {
            string_ID += '|';
        }
        string_ID += ID[i];
    }
    output["ID"] = string_ID;
    output["Ingredients"] = empty_array;
    output["Product"] = empty_array;
    for (size_t i = 0; i < ingredients.size(); i++) {
        current["ItemClass"] = ingredients[i].get_name();
        current["Amount"] = std::to_string(ingredients[i].get_amount().get_numerator());
        output["Ingredients"].push_back(current);
    }
    for (size_t i = 0; i < products.size(); i++) {
        current["ItemClass"] = products[i].get_name();
        current["Amount"] = std::to_string(products[i].get_amount().get_numerator());;
        output["Product"].push_back(current);
    }
    return output;
}

string Recipe::to_string() const {
    ostringstream out;

    out << "ID: {";
    for (size_t i = 0; i < ID.size(); i++) {
        if (i != 0) {
            out << '|';
        }
        out << ID[i];
    }
    out << "}" << endl;

    out << "ING: {";
    for (size_t i = 0; i < ingredients.size(); i++) {
        if (i != 0) {
            out << ';';
        }
        out << ingredients[i].get_name() << '=' << ingredients[i].get_amount().get_numerator();
    }
    out << "}" << endl;

    out << "PROD: {";
    for (size_t i = 0; i < products.size(); i++) {
        if (i != 0) {
            out << ';';
        }
        out << products[i].get_name() << '=' << products[i].get_amount().get_numerator();
    }
    out << "}" << endl << endl;

    return out.str();
}

void Recipe::set_to(const Fraction end_result) {
    Fraction multiple = end_result / products[0].get_amount();

    for (size_t i = 0; i < ingredients.size(); i++) {
        ingredients[i].set_amount(ingredients[i].get_amount() * multiple);
    }

    for (size_t i = 0; i < products.size(); i++) {
        products[i].set_amount(products[i].get_amount() * multiple);
    }
}

bool Recipe::is_processed() const {
    return processed;
}

bool Recipe::same_name(const Recipe& other) const {
    return (name == other.get_name() && ID == other.get_ID());
}

bool Recipe::same_product_ID(const Recipe& other) const {
    return (product_ID == other.get_product_ID() && ID == other.get_ID());
}

/**************************************************/
// Operator Overloads
/**************************************************/

bool Recipe::operator==(const Recipe& other) const {
    // if name, ID, factory, or machine speed are not equivalent, return false
    if (product_ID != other.get_product_ID() || machine_speed != other.get_machine_speed() || name != other.get_name() || factory != other.get_factory() || ID != other.get_ID()) {
        return false;
    }

    // if either the product or ingredient vectors aren't the same size, return false
    if (ingredients.size() != other.get_ingredients_ref().size() || products.size() != other.get_products_ref().size()) {
        return false;
    }

    // if any of the ingredients or products do not match, return false;
    for (size_t i = 0; i < ingredients.size(); i++) {
        if (ingredients.at(i) != other.get_ingredient_ref(i)) {
            return false;
        }
    }
    for (size_t i = 0; i < products.size(); i++) {
        if (products.at(i) != other.get_product_ref(i)) {
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
    if (!same_product_ID(other)) {
        throw invalid_argument("Cannot combine different recipes.\n" + name + " != " + other.get_name() + ".");
    }

    for (size_t i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) += other.get_ingredient(i);
    }
    for (size_t i = 0; i < products.size(); i++) {
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
    if (!same_product_ID(other)) {
        throw invalid_argument("Cannot combine different recipes.\n" + name + " != " + other.get_name() + ".");
    }

    for (size_t i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) -= other.get_ingredient(i);
    }
    for (size_t i = 0; i < products.size(); i++) {
        products.at(i) -= other.get_product(i);
    }
    return *this;
}

Recipe Recipe::operator-(const Recipe& other) const {
    Recipe result = *this;
    result -= other;
    return result;
}


Recipe& Recipe::operator*=(const Fraction multiple) {
    for (size_t i = 0; i < ingredients.size(); i++) {
        ingredients.at(i) *= multiple;
    }
    for (size_t i = 0; i < products.size(); i++) {
        products.at(i) *= multiple;
    }
    return *this;
}

Recipe Recipe::operator*(const Fraction multiple) const {
    Recipe result = *this;
    result *= multiple;
    return result;
}

#endif