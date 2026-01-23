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
        void set_name(string title);
        void set_factory(string building);
        void set_ipm(double rate);
        void set_ingredients(vector<Resource> ingredient);
        void add_ingredient(Resource ingredient);
        void set_products(vector<Resource> product);
        void add_product(Resource product);
        string get_name();
        string get_factory();
        double get_ipm();
        vector<Resource> get_ingredients();
        Resource get_ingredient(int i);
        vector<Resource> get_products();
        Resource get_product(int i);

    private:
        string name;
        string factory;
        double ipm;
        vector<Resource> ingredients;
        vector<Resource> products;

};

#endif