#ifndef NODE_INPUT_CC
#define NODE_INPUT_CC

#include "node_input.h"

using namespace std;

Node::Node() {
    ipm = 0;
    type = "N/A";
    x = 0;
    y = 0;
    tier = 0;
}

Node::Node(istream& fin) {
    fin >> ipm >> type >> x >> y >> tier;
    type = fix_string(type);
}

int Node::get_rate() const {
    return ipm;
}

string Node::get_resource() const {
    return type;
}

pair<int,int> Node::get_position() const {
    pair<int,int> p;
    p.first = x;
    p.second = y;
    return p;
}

int Node::get_x() const {
    return x;
}

int Node::get_y() const {
    return y;
}

int Node::get_tier() const {
    return tier;
}

string Node::to_string() const {
    string data =
        std::to_string(ipm) + " " +
        type + " " +
        std::to_string(x) + " " +
        std::to_string(y) + " " +
        std::to_string(tier);
    return data;
}

void Node::set_rate(int rate) {
    ipm = rate;
}

void Node::set_quality(string node_quality) {
    node_quality = fix_string(node_quality);
    if (node_quality == "Impure") {
        ipm = 300;
    }
    else if (node_quality == "Normal") {
        ipm = 600;
    }
    else if (node_quality == "Pure") {
        ipm = 1200;
    }
    else {
        ipm = stoi(node_quality);
    }
}

void Node::set_resource(string resource) {
    type = fix_string(resource);
}

void Node::set_position(pair<int,int> p) {
    x = p.first;
    y = p.second;
}

void Node::set_position(int x_coord, int y_coord) {
    x = x_coord;
    y = y_coord;
}

void Node::set_tier(int value) {
    tier = value;
}

void Node::set_values_quality(string node_quality, string resource, int x_coord, int y_coord) {
    set_quality(node_quality);
    type = fix_string(resource);
    x = x_coord;
    y = y_coord;
    tier = 0;
}

void Node::set_values_rate(int rate, string resource, int x_coord, int y_coord) {
    ipm = rate;
    type = fix_string(resource);
    x = x_coord;
    y = y_coord;
    tier = 0;
}

bool Node::operator==(const Node& other) {
    return 
        other.get_x() == x &&
        other.get_y() == y &&
        other.get_resource() == type &&
        other.get_tier() == tier;
}

string Node::fix_string(string s) {
    if (s.size() > 0) {
        s[0] = std::toupper(s[0]);
        for (int i = 1; i < s.size(); i++) {
            s[i] = tolower(s[i]);
        }
        if (s == "Sam") {
            s = "SAM";
        }
    }
    return s;
}

#endif
