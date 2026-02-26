#ifndef RESOURCE_H
#define RESOURCE_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

class Resource {
    public:
        Resource();
        Resource(const json& data);
        Resource(const string title, const double rate);
        Resource(const string title);
        void set_resource(const json& data);
        void set_name(const string title);
        void set_amount(const double rate);
        string get_name() const;
        double get_amount() const;
        bool same_name(const Resource& other) const;

        bool operator==(const Resource& other) const;
        bool operator!=(const Resource& other) const;
        Resource& operator+=(const Resource& other);
        Resource operator+(const Resource& other) const;
        Resource& operator-=(const Resource& other);
        Resource operator-(const Resource& other) const;
        Resource& operator*=(const double multiple);
        Resource operator*(const double multiple) const;

        inline static const double EPSILON = 1e-9;

    private:
        string name;
        double amount;

};

#endif