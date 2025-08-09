#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

class Node {
    public:
        //HERE
        Node();
        Node(std::istream& fin);
        static void find_children(std::vector<Node> list);
        //END
        int get_rate() const;
        std::string get_resource() const;
        std::pair<int,int> get_position() const;
        int get_x() const;
        int get_y() const;
        int get_tier() const;
        //HERE
        int get_id() const;
        const std::vector<int> get_child_ids() const;
        const std::vector<Node*> get_childs() const;
        std::string to_string() const;
        //END
        void set_rate(int rate);
        void set_quality(std::string node_quality);
        void set_resource(std::string resource);
        void set_position(std::pair<int,int> p);
        void set_position(int x_coord, int y_coord);
        void set_tier(int value);
        //HERE
        void set_id(int value);
        //END
        void set_values_quality(std::string node_quality, std::string resource, int x_coord, int y_coord);
        void set_values_rate(int rate, std::string resource, int x_coord, int y_coord);
        //HERE
        void add_child(int nid);
        void add_child(Node n);
        void add_child(Node n, bool b);
        Node create_center_node(std::vector<Node> nodes, int id);
        //END
        bool operator==(const Node& other);
    private:
        std::string fix_string(std::string s) {
            if (s.size()>0) {
                s[0] = std::toupper(s[0]);
                for (int i = 1; i<s.size(); i++) {
                    s[i] = tolower(s[i]);
                }
            }
            return s;
        }
        int ipm; // the rate that the node produces its resource in items per minute
        std::string type; // the type of resource the node produces
        int x; // the x-coordinate of the node
        int y; // the y-coordinate of the node
        int tier; // tier zero is a resource patch, while higher tiers are combined nodes (can be combined resource patches)
        //HERE
        int id; //unique id of node
        std::vector<int> child_ids; //id of children;
        std::vector<Node*> childs; //pointer to child objects to mimic behavior of a function that will be created the unity project : obj.child(num) returns child_obj
        //END
};


#endif