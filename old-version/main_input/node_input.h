#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>

class Node {
    public:
        Node();
        Node(std::istream& fin);
        int get_rate() const;
        std::string get_resource() const;
        std::pair<int,int> get_position() const;
        int get_x() const;
        int get_y() const;
        int get_tier() const;
        std::string to_string() const;
        void set_rate(int rate);
        void set_quality(std::string node_quality);
        void set_resource(std::string resource);
        void set_position(std::pair<int,int> p);
        void set_position(int x_coord, int y_coord);
        void set_tier(int value);
        void set_values_quality(std::string node_quality, std::string resource, int x_coord, int y_coord);
        void set_values_rate(int rate, std::string resource, int x_coord, int y_coord);
        bool operator==(const Node& other);
    private:
        std::string fix_string(std::string s);
        int ipm; // the rate that the node produces its resource in items per minute
        std::string type; // the type of resource the node produces
        int x; // the x-coordinate of the node
        int y; // the y-coordinate of the node
        int tier; // tier zero is a resource patch, while higher tiers are combined nodes (can be combined resource patches)
};

#endif
