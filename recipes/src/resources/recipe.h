#ifndef RECIPE_H
#define RECIPE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <sstream>

#include "resource.h"
#include "fraction.h"

#include "stats.h"

using namespace std;
using json = nlohmann::ordered_json;

class Recipe {
    public:
        Recipe();
        Recipe(const json& data, const size_t id = SIZE_MAX);
        void set_recipe(const json& data, const size_t id = SIZE_MAX);
        void set_terminal_recipe(const Resource product);
        void set_name(const string& title);
        void set_ID(const vector<int>& id);
        void set_factory(const string& building);
        void set_machine_speed(const double rate);
        void set_ingredients(const vector<Resource>& ingredient);
        void add_ingredient(const Resource& ingredient);
        void set_products(const vector<Resource>& product);
        void add_product(const Resource& product);
        void set_product_ID(const size_t id);
        void set_processed();
        void combine_recipes(const Recipe& other);
        void merge_recipes(const vector<Recipe>& data);
        void set_primary_product(const string& primary_name);
        string get_name() const;
        vector<int>& modify_ID();
        vector<int> get_ID() const;
        const vector<int>& get_ID_ref() const;
        string get_factory() const;
        double get_machine_speed() const;
        vector<Resource>& modify_ingredients();
        vector<Resource> get_ingredients() const;
        const vector<Resource>& get_ingredients_ref() const;
        Resource get_ingredient(int i) const;
        const Resource& get_ingredient_ref(int i) const;
        vector<Resource>& modify_products();
        vector<Resource> get_products() const;
        const vector<Resource>& get_products_ref() const;
        Resource get_product(int i) const;
        const Resource& get_product_ref(int i) const;
        size_t get_product_ID() const;
        json to_json() const;
        json to_compressed_json() const;
        string to_string() const;
        void set_to(const Fraction end_result);
        bool is_processed() const;
        bool same_name(const Recipe& other) const;
        bool same_product_ID(const Recipe& other) const;

        /**************************************************/
        // Operator Overloads
        /**************************************************/

        bool operator==(const Recipe& other) const;
        bool operator!=(const Recipe& other) const;
        Recipe& operator+=(const Recipe& other);
        Recipe operator+(const Recipe& other) const;
        Recipe& operator-=(const Recipe& other);
        Recipe operator-(const Recipe& other) const;
        Recipe& operator*=(const Fraction multiple);
        Recipe operator*(const Fraction multiple) const;

    private:
        size_t product_ID; // the ID of the recipe
        string name; // the name of the recipe
        vector<int> ID; // the incrementor values that made the recipe
        string factory; // the name of the machine the recipe is made it
        double machine_speed; // the number of seconds it takes for the machine to operate once
        vector<Resource> ingredients; // the ingredients of the recipe
        vector<Resource> products; // the products of the recipe
        bool processed = false; // Marks if the recipe has been processed

};

#endif