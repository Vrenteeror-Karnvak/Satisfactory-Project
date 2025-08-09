#include "node.h"

using namespace std;

//constructors

Node::Node() {
    ipm = 0;
    type = "N/A";
    x = 0;
    y = 0;
    tier = 0;
}

Node::Node(string patch_quality, string resource, int x_coord, int y_coord) {
    if (patch_quality == "Impure" || patch_quality == "impure") {
        ipm = 300;
    }
    else if (patch_quality == "Normal" || patch_quality == "normal") {
        ipm = 600;
    }
    else if (patch_quality == "Pure" || patch_quality == "pure") {
        ipm = 1200;
    }
    else {
        ipm = 0;
    }
    type = resource;
    x = x_coord;
    y = y_coord;
    tier = 0;
}

Node::Node(int rate, string resource, int x_coord, int y_coord) {
    ipm = rate;
    type = resource;
    x = x_coord;
    y = y_coord;
    tier = 0;
}

Node::Node(std::istream& in) {
    in >> type >> ipm >> x >> y >> tier;
}

//getters

int Node::get_rate() const {
    return ipm;
}

string Node::get_resource() {
    return type;
}

pair<int,int> Node::get_position() const {
    pair<int,int> p;
    p.first = x;
    p.second = y;
    return p;
}

int Node::get_tier() const {
    return tier;
}

string Node::data_string() const {
    string data =
        to_string(ipm) + " "
        + type + " "
        + to_string(x) + " "
        + to_string(y) + " "
        + to_string(tier) + "\n";
    return data;
}

//setters

void Node::set_rate(int rate) {
    ipm = rate;
}

void Node::set_resource(string resource) {
    type = resource;
}

void Node::set_position(pair<int,int> p) {
    x = p.first;
    y = p.second;
}

void Node::set_tier(int value) {
    tier = value;
}

void Node::set_values_quality(string patch_quality, string resource, int x_coord, int y_coord) {
    if (patch_quality == "Impure" || patch_quality == "impure") {
        ipm = 300;
    }
    else if (patch_quality == "Normal" || patch_quality == "normal") {
        ipm = 600;
    }
    else if (patch_quality == "Pure" || patch_quality == "pure") {
        ipm = 1200;
    }
    else {
        ipm = 0;
    }
    type = resource;
    x = x_coord;
    y = y_coord;
    tier = 0;
}

void Node::set_values_rate(int rate, string resource, int x_coord, int y_coord) {
    ipm = rate;
    type = resource;
    x = x_coord;
    y = y_coord;
    tier = 0;
}
