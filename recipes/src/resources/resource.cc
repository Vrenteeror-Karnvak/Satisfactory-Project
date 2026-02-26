#ifndef RESOURCE_CC
#define RESOURCE_CC

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

Resource::Resource(const string title) {
    name = title;
    amount = 0;
}

void Resource::set_resource(const json& data) {
    name = data.value("ItemClass", "");
    amount = stod(data.value("Amount", ""));
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

bool Resource::same_name(const Resource& other) const {
    return name == other.get_name();
}

bool Resource::operator==(const Resource& other) const {
    return (name == other.get_name() && ((amount - other.get_amount())) < EPSILON);
}

bool Resource::operator!=(const Resource& other) const {
    return !(*this == other);
}

Resource& Resource::operator+=(const Resource& other) {
    if (!same_name(other)) {
        throw invalid_argument("Cannot combine different resources.");
    }

    amount += other.get_amount();
    return *this;
}

Resource Resource::operator+(const Resource& other) const {
    Resource result = *this;
    result += other;
    return result;
}

Resource& Resource::operator-=(const Resource& other) {
    if (!same_name(other)) {
        throw invalid_argument("Cannot combine different resources.");
    }

    amount -= other.get_amount();
    return *this;
}

Resource Resource::operator-(const Resource& other) const {
    Resource result = *this;
    result -= other;
    return result;
}

Resource& Resource::operator*=(const double multiple) {
    amount *= multiple;
    return *this;
}

Resource Resource::operator*(const double multiple) const {
    Resource result;
    result *= multiple;
    return result;
}

#endif