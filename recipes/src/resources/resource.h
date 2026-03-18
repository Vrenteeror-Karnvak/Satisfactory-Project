#ifndef RESOURCE_H
#define RESOURCE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>

#include "fraction.h"

using namespace std;
using json = nlohmann::ordered_json;

class Resource {
    public:
        Resource();
        Resource(const json& data);
        Resource(const string title, const Fraction rate);
        Resource(const string title);
        void set_resource(const json& data);
        void set_name(const string title);
        void set_amount(const int n, const int d = 1);
        void set_amount(const Fraction value);
        string get_name() const;
        Fraction get_amount() const;
        bool same_name(const Resource& other) const;

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
        string name;
        Fraction amount;

};

#endif