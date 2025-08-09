#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>

class Node {
    public:
        //constructors
        Node();
        Node(std::string patch_quality, std::string resource, int x_coord, int y_coord);
        Node(int rate, std::string resource, int x_coord, int y_coord);
        Node(std::istream& in);

        //getters
        int get_rate() const;
        std::string get_resource();
        std::pair<int,int> get_position() const;
        int get_tier() const {return tier;}
        std::string data_string() const;

        //setters
        void set_rate(int rate);
        void set_resource(std::string resource);
        void set_position(std::pair<int,int> p);
        void set_tier(int value);
        void set_values_quality(std::string patch_quality, std::string resource, int x_coord, int y_coord);
        void set_values_rate(int rate, std::string resource, int x_coord, int y_coord);

    private:
        int ipm; // the rate that the node produces its resource in items per minute
        std::string type; // the type of resource the node produces
        int x; // the x-coordinate of the node
        int y; // the y-coordinate of the node
        int tier; // tier zero means a singular node)
};

#endif
