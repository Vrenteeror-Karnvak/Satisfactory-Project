#ifndef RESOURCE_H
#define RESOURCE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>

#include "fraction.h"

#include "stats.h"

using namespace std;
using json = nlohmann::ordered_json;

class Resource {
    public:
        Resource();
        Resource(const json& data, const size_t ID = SIZE_MAX);
        Resource(const string title, const Fraction rate, const size_t ID = SIZE_MAX);
        Resource(const string title, const size_t ID = SIZE_MAX);
        void set_resource(const json& data);
        void set_name(const string title);
        void set_amount(const int n, const int d = 1);
        void set_amount(const Fraction value);
        void set_product_ID(const size_t value);
        string get_name() const;
        Fraction get_amount() const;
        size_t get_product_ID() const;
        bool same_name(const Resource& other) const;
        bool same_product_ID(const Resource& other) const;

        /**************************************************/
        // Operator Overloads
        /**************************************************/

        bool operator==(const Resource& other) const;
        bool operator!=(const Resource& other) const;
        bool operator<(const Resource& other) const;
        bool operator<=(const Resource& other) const;
        bool operator>(const Resource& other) const;
        bool operator>=(const Resource& other) const;
        Resource& operator+=(const Resource& other);
        Resource operator+(const Resource& other) const;
        Resource& operator-=(const Resource& other);
        Resource operator-(const Resource& other) const;
        Resource& operator*=(const Fraction multiple);
        Resource operator*(const Fraction multiple) const;

    private:
        size_t product_ID; // the ID of the resource
        string name; // the name of the resource
        Fraction amount; // the amount of the resource

};

#endif