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
        void combine_resource(const Resource other);
        string get_name() const;
        double get_amount() const;
        bool operator==(const Resource& other) const;

    private:
        string name;
        double amount;

};

#endif