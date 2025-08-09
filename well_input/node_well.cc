#ifndef NODE_CC
#define NODE_CC

#include "node_well.h"

using namespace std;

Node::Node() {
    ipm = 0;
    type = "N/A";
    x = 0;
    y = 0;
    tier = 0;
    id = -1;
}

Node::Node(istream& fin) {
    int count;
    fin >> ipm >> type >> x >> y >> tier >> id >> count;
    for (int i = 0; i < count; i++) {
        int n;
        fin >> n;
        child_ids.push_back(n);
    }
    type = fix_string(type);
}

void Node::find_children(vector<Node> list) {
    map<int,Node*> id_to_Node;
    for (int i = 0; i<list.size(); i++) {
        Node n = list[i];
        id_to_Node[n.get_id()] = &n;
    }
    //create child list from child ids
    for (int i = 0; i<list.size(); i++) {
        Node n = list[i];
        const vector<int> v = n.get_child_ids();
        for (int j = 0; j<v.size(); j++) {
            int id = v[j];
            if (id_to_Node.count(id)>0) {
                n.add_child(*id_to_Node[id], false);
            } else {
                cout << "Failed to get child " << n.get_id() << " for node " << id << endl;
            }
        }
    }
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

int Node::get_id() const {
    return id;
}

const vector<int> Node::get_child_ids() const {
    return child_ids;
}

const vector<Node*> Node::get_childs() const {
    return childs;
}

string Node::to_string() const {
    int cid_size = child_ids.size();
    string data =
        std::to_string(ipm) + " " +
        type + " " +
        std::to_string(x) + " " +
        std::to_string(y) + " " +
        std::to_string(tier) + " " +
        std::to_string(id) + " " +
        std::to_string(cid_size);
    if (cid_size > 0) {
        for (int i = 0; i < cid_size; i++) {
            data += " " + std::to_string(child_ids[i]);
        }
    }
    return data;
}

void Node::set_rate(int rate) {
    ipm = rate;
}

void Node::set_quality(string node_quality) {
    node_quality = fix_string(node_quality);
    if (node_quality == "Impure") {
        ipm = 75;
    }
    else if (node_quality == "Normal") {
        ipm = 150;
    }
    else if (node_quality == "Pure") {
        ipm = 300;
    }
    else {
        ipm = 0;
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

void Node::set_id(int value) {
    id = value;
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

void Node::add_child(int nid) {
    child_ids.push_back(nid);
}

void Node::add_child(Node n) {
    add_child(n, true);
}

void Node::add_child(Node n, bool b) {
    childs.push_back(&n);
    int id = n.get_id();
    if (b) {
        bool found;
        for (int i = 0; i<child_ids.size(); i++) {
            if (child_ids[i]==id) {
                found = true;
            }
        }
        if (!found) {
            child_ids.push_back(id);
        }
    }
}

Node Node::create_center_node(vector<Node> nodes, int id) {
    Node created;
    created.set_id(id);
    int a_x = 0;
    int a_y = 0;
    int total_ipm = 0;
    int highest_tier = 0;
    
    for (int i = 0; i < nodes.size(); i++) {
        Node n = nodes.at(i);
        //sum position
        a_x += n.get_x();
        a_y += n.get_y();

        //sum IPM
        total_ipm += n.get_rate();

        if (n.get_tier() > highest_tier) {
            highest_tier = n.get_tier();
        }

        created.add_child(n.get_id());
        created.add_child(n);
    }
    a_x /= nodes.size();
    a_y /= nodes.size();
    created.set_rate(total_ipm);
    created.set_resource(nodes.at(0).get_resource());
    created.set_position(a_x, a_y);
    created.set_tier(highest_tier + 1);
    return created;
}

bool Node::operator==(const Node& other) {
    return 
        other.get_x() == x &&
        other.get_y() == y &&
        other.get_resource() == type &&
        other.get_tier() == tier;
}

string Node::fix_string(string s) {
    if (s.size()>0) {
        s[0] = std::toupper(s[0]);
        for (int i = 1; i<s.size(); i++) {
            s[i] = tolower(s[i]);
        }
    }
    return s;
}

#endif
