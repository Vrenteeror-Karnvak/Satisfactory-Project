#ifndef RESOURCE_CC
#define RESOURCE_CC

#include "json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "resource.h"

using namespace std;
using json = nlohmann::ordered_json;

Resource::Resource() {
    name = "N/A";
    amount = 0;
}

Resource::Resource(const json& data) {
    name = data.value("ItemClass", "");
    amount = stod(data.value("Amount", ""));
}

Resource::Resource(const string title, const double rate) {
    name = title;
    amount = rate;
}

void Resource::set_name(const string title) {
    name = title;
}

void Resource::set_amount(const double rate) {
    amount = rate;
}

string Resource::get_name() const {
    return name;
}

double Resource::get_amount() const {
    return amount;
}

#endif