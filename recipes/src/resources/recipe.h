#ifndef RECIPE_H
#define RECIPE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "resource.h"

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
        void set_ipm(const double rate);
        void set_ingredients(const vector<Resource> ingredient);
        void add_ingredient(const Resource ingredient);
        void set_products(const vector<Resource> product);
        void add_product(const Resource product);
        void set_processed();
        string get_name() const;
        string get_factory() const;
        double get_ipm() const;
        vector<Resource> get_ingredients() const;
        Resource get_ingredient(int i) const;
        vector<Resource> get_products() const;
        Resource get_product(int i) const;
        bool is_processed() const;

    private:
        string name;
        string factory;
        double ipm;
        vector<Resource> ingredients;
        vector<Resource> products;
        bool processed = false;

};

#endif