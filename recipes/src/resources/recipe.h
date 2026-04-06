#ifndef RECIPE_H
#define RECIPE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include "resource.h"
#include "fraction.h"

using namespace std;
using json = nlohmann::ordered_json;

class Recipe {
    public:
        Recipe();
        Recipe(const json& data);
        void set_recipe(const json& data);
        void set_terminal_recipe(const Resource product);
        void set_name(const string title);
        void set_factory(const string building);
        void set_machine_speed(const double rate);
        void set_ingredients(const vector<Resource> ingredient);
        void add_ingredient(const Resource ingredient);
        void set_products(const vector<Resource> product);
        void add_product(const Resource product);
        void set_processed();
        void combine_recipes(const Recipe other);
        void merge_recipes(const vector<Recipe> data);
        void set_primary_product(const string& primary_name);
        string get_name() const;
        string get_factory() const;
        double get_machine_speed() const;
        vector<Resource> get_ingredients() const;
        Resource get_ingredient(int i) const;
        vector<Resource> get_products() const;
        Resource get_product(int i) const;
        json to_json() const;
        json to_compressed_json() const;
        void set_to(const Fraction end_result);
        bool is_processed() const;
        bool same_name(const Recipe& other) const;

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
        string name; // the name of the recipe
        string factory; // the name of the machine the recipe is made it
        double machine_speed; // the number of seconds it takes for the machine to operate once
        vector<Resource> ingredients; // the ingredients of the recipe
        vector<Resource> products; // the products of the recipe
        bool processed = false; // Marks if the recipe has been processed

};

#endif