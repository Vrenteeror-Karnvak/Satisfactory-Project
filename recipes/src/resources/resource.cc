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
    if (data.value("Amount", "").find("/") != string::npos) {
        int pos = data.value("Amount", "").find("/");
        Fraction temp(stoi(data.value("Amount", "").substr(0, pos)), stoi(data.value("Amount", "").substr(pos + 1)));
        amount = temp;
    }
    else {
        amount = stoi(data.value("Amount", ""));
    }
}

Resource::Resource(const string title, const Fraction rate) {
    name = title;
    amount = rate;
}

Resource::Resource(const string title) {
    name = title;
    amount.set_numerator(0);
    amount.set_denominator(1);
}

void Resource::set_resource(const json& data) {
    name = data.value("ItemClass", "");
    amount = stod(data.value("Amount", ""));
}

void Resource::set_name(const string title) {
    name = title;
}

void Resource::set_amount(const int n, const int d) {
    amount.set_numerator(n);
    amount.set_denominator(d);
}
void Resource::set_amount(const Fraction value) {
    amount = value;
}

string Resource::get_name() const {
    return name;
}

Fraction Resource::get_amount() const {
    return amount;
}

bool Resource::same_name(const Resource& other) const {
    return name == other.get_name();
}

/**************************************************/
// Operator Overloads
/**************************************************/

bool Resource::operator==(const Resource& other) const {
    return (name == other.get_name() && amount == other.get_amount());
}

bool Resource::operator!=(const Resource& other) const {
    return !(*this == other);
}

bool Resource::operator<(const Resource& other) const {
    if (!same_name(other)) {
        throw invalid_argument("Cannot compare different resources.");
    }

    return (amount < other.get_amount());
}

bool Resource::operator<=(const Resource& other) const {
    return (*this < other || *this == other);
}

bool Resource::operator>(const Resource& other) const {
    if (!same_name(other)) {
        throw invalid_argument("Cannot compare different resources.");
    }

    return (amount > other.get_amount());
}

bool Resource::operator>=(const Resource& other) const {
    return (*this > other || *this == other);
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

Resource& Resource::operator*=(const Fraction multiple) {
    amount *= multiple;
    return *this;
}

Resource Resource::operator*(const Fraction multiple) const {
    Resource result;
    result *= multiple;
    return result;
}

#endif