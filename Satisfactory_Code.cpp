#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
using namespace std;

class Node {
    public:
        Node();
        Node(string patch_quality, string resource, int x_coord, int y_coord);
        Node(int rate, string resource, int x_coord, int y_coord);
        Node(ifstream& in) {
            in >> type >> ipm >> x >> y >> tier;
        }
        int get_rate() const {return ipm;}
        string get_resource() {return type;}
        pair<int,int> get_position() const {
            pair<int,int> p;
            p.first = x;
            p.second = y;
            return p;
        }
        int get_tier() const {return tier;}
        string data_string() {
            string data = to_string(ipm) + " " + type + " " + to_string(x) + " " + to_string(y) + " " + to_string(tier) + "\n";
            return data;
        }
        void set_rate(int rate) {ipm = rate;}
        void set_resource(string resource) {type = resource;}
        void set_position(pair<int,int> p) {
            x = p.first;
            y = p.second;
        }
        void set_tier(int value) {tier = value;}
        void set_values_quality(string patch_quality, string resource, int x_coord, int y_coord) {
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
        void set_values_rate(int rate, string resource, int x_coord, int y_coord) {
            ipm = rate;
            type = resource;
            x = x_coord;
            y = y_coord;
            tier = 0;
        }

    private:    
        int ipm; // the rate that the node produces its resource in items per minute
        string type; // the type of resource the node produces
        int x; // the x-coordinate of the node
        int y; // the y-coordinate of the node
        int tier; // tier zero means a singular node)
};

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

int main() {
    string resource; // the type of resource the node produces
    string patch_quality;
    int rate; // the rate that the node produces its resource in items per minute
    int x_coord; // the x-coordinate of the node
    int y_coord; // the y-coordinate of the node
    char selection;
    
    while (true) {
        Node node;

        cout << "Select input mode (n for node quality and r for rate) or press q to quit: ";
        cin >> selection;

        if (selection == 'r') {
            cout << "Enter the rate, resource, x-coordinate, and y-coordinate: ";
            cin >> rate >> resource >> x_coord >> y_coord;
            node.set_values_rate(rate, resource, x_coord, y_coord);
        }
        else if (selection == 'n') {
            cout << "Enter the node quality, resource, x-coordinate, and y-coordinate: ";
            cin >> patch_quality >> resource >> x_coord >> y_coord;
            node.set_values_quality(patch_quality, resource, x_coord, y_coord);
        }
        else if (selection == 'q') {
            break;
        }
        else {
            cout << "Invalid selection. Restarting." << endl;
            continue;
        }
        
        pair<int,int> coord = node.get_position();

        cout << node.get_resource() << endl;
        cout << node.get_rate() << endl;
        cout << "(" << coord.first << ", " << coord.second << ")" << endl;
        cout << node.get_tier() << endl;
        cout << node.data_string();
    }

    return 0;
}