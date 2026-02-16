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

void Resource::combine_resource(const Resource other) {
    if (name != other.get_name()) {
        // Terminates the process if the resources aren't the same.
        cout << "These are not the same resource. Can not combine." << endl;
        return;
    }
    amount += other.get_amount(); // adds the other amount to the original
}

string Resource::get_name() const {
    return name;
}

double Resource::get_amount() const {
    return amount;
}

bool Resource::operator==(const Resource& other) const {
    return name == other.get_name();
}

#endif