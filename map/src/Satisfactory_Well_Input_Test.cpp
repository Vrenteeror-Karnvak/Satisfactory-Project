#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
using namespace std;

class Node {
    public:
        //HERE
        Node() {
            ipm = 0;
            type = "N/A";
            x = 0;
            y = 0;
            tier = 0;
            id = -1;
        }
        Node(ifstream& fin) {
            int count;
            fin >> ipm >> type >> x >> y >> tier >> id >> count;
            for (int i = 0; i < count; i++) {
                int n;
                fin >> n;
                child_ids.push_back(n);
            }
            type = fix_string(type);
        }
        static void find_children(vector<Node> list) {
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
        //END
        int get_rate() const {return ipm;}
        string get_resource() const {return type;}
        pair<int,int> get_position() const {
            pair<int,int> p;
            p.first = x;
            p.second = y;
            return p;
        }
        int get_x() const {return x;}
        int get_y() const {return y;}
        int get_tier() const {return tier;}
        //HERE
        int get_id() const {return id;}
        const vector<int> get_child_ids() const {return child_ids;}
        const vector<Node*> get_childs() const {return childs;}
        string to_string() const {
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
        //END
        void set_rate(int rate) {ipm = rate;}
        void set_quality(string node_quality) {
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
        void set_resource(string resource) {type = fix_string(resource);}
        void set_position(pair<int,int> p) {
            x = p.first;
            y = p.second;
        }
        void set_position(int x_coord, int y_coord) {
            x = x_coord;
            y = y_coord;
        }
        void set_tier(int value) {tier = value;}
        //HERE
        void set_id(int value) {
            id = value;
        }
        //END
        void set_values_quality(string node_quality, string resource, int x_coord, int y_coord) {
            set_quality(node_quality);
            type = fix_string(resource);
            x = x_coord;
            y = y_coord;
            tier = 0;
        }
        void set_values_rate(int rate, string resource, int x_coord, int y_coord) {
            ipm = rate;
            type = fix_string(resource);
            x = x_coord;
            y = y_coord;
            tier = 0;
        }
        //HERE
        void add_child(int nid) {
            child_ids.push_back(nid);
        }
        void add_child(Node n) {
            add_child(n, true);
        }
        void add_child(Node n, bool b) {
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
        Node create_center_node(vector<Node> nodes, int id) {
            Node created;
            created.set_id(id);
            int a_x = 0;
            int a_y = 0;
            int total_ipm = 0;
            int highest_tier = 0;

            cout << a_y << endl;

            for (int i = 0; i < nodes.size(); i++) {
                Node n = nodes.at(i);
                //sum position
                a_x += n.get_x();

                cout << a_y << endl;
                a_y += n.get_y();
                cout << a_y << endl;

                //sum IPM
                total_ipm += n.get_rate();

                if (n.get_tier() > highest_tier) {
                    highest_tier = n.get_tier();
                }

                created.add_child(n.get_id());
                created.add_child(n);
            }

            a_x /= nodes.size();

            cout << a_y << endl;
            a_y /= nodes.size();
            cout << a_y << endl;

            created.set_rate(total_ipm);
            created.set_resource(nodes.at(0).get_resource());
            created.set_position(a_x, a_y);
            created.set_tier(highest_tier + 1);
            return created;
        }
        //END
        bool operator==(const Node& other) {
            return 
                other.get_x() == x &&
                other.get_y() == y &&
                other.get_resource() == type &&
                other.get_tier() == tier;
        }
    private:
        string fix_string(string s) {
            if (s.size()>0) {
                s[0] = std::toupper(s[0]);
                for (int i = 1; i<s.size(); i++) {
                    s[i] = tolower(s[i]);
                }
            }
            return s;
        }
        int ipm; // the rate that the node produces its resource in items per minute
        string type; // the type of resource the node produces
        int x; // the x-coordinate of the node
        int y; // the y-coordinate of the node
        int tier; // tier zero is a resource patch, while higher tiers are combined nodes (can be combined resource patches)
        //HERE
        int id; //unique id of node
        vector<int> child_ids; //id of children;
        vector<Node*> childs; //pointer to child objects to mimic behavior of a function that will be created the unity project : obj.child(num) returns child_obj
        //END
};

int main(int argc, char* argv[]) {
    //dat folder has copies of all out files
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    filesystem::path PATH = exePath / "satisfactory_node_data_test.txt";
    ifstream fin(PATH);
    vector<Node> nodes;
    if (fin) {
        cout << "File found, loading data." << endl;
        while (!fin.eof()) {
            Node n(fin);
            if (!fin.eof()) {
                nodes.push_back(n);
            }
        }
        

        cout << "Data loaded. Checking for any repeats." << endl;
        int repeats_found = 0;
        //for nodes
        for (vector<Node>::const_iterator i = nodes.cbegin(); i!=nodes.cend(); i++) {
            Node n = *i;
            //for nodes starting at i+1
            vector<Node>::const_iterator j = i;
            j++;
            while (j!=nodes.cend()) {
                //if duplicate found, save position, step back, delete saved position
                if (n==*j) {
                    vector<Node>::const_iterator k = j;
                    j--;
                    nodes.erase(k);
                    ++repeats_found;
                }
                j++;
            }
        }
        cout << repeats_found << " found and removed. Continuing with program." << endl;
    }
    fin.close();

    int ID_num = 0;

    ofstream fout(PATH, ios_base::trunc);

    if (fout.is_open()) {
        cout << "File opened correctly. Continuing with program." << endl;
    }
    else {
        cout << "File failed to open. Terminating program." << endl;
        exit(10);
    }

    if (nodes.size()>0) {
        cout << "Writing nodes to file. ";
        for (int i = 0; i<nodes.size(); i++) {
            fout << nodes[i].to_string() << '\n';
            ID_num += 1;
        }
        cout << "Finished." << endl;
    }

    bool resource_flag, quality_flag, coordinate_flag;
    string input;
    Node node;

    resource_flag = true;
    while (resource_flag) {        
        cout << "Enter the name of the resource (q to quit): ";
        cin >> input;
        if (input == "q") {
            resource_flag = false;
            continue;
        }
        node.set_resource(input);

        cout << node.get_resource() << "\nIs that correct? (y/n): ";
        cin >> input;
        if (input != "y") {
            continue;
        }

        int first_ID = ID_num;
        int count = 0;
        vector<Node> well_nodes;

        quality_flag = true;
        while (quality_flag) {
            cout << "Enter the quality of the node (q to quit): ";
            cin >> input;
            if (input == "q") {
                quality_flag = false;
                continue;
            }
            else if (!(input == "Impure" || input == "Normal" || input == "Pure")) {
                cout << "Not a valid purity. Would you like to enter a ipm instead? (y/n): ";
                cin >> input;
                if (input == "n") {
                    quality_flag = false;
                    continue;
                }

                cout << "Enter the ipm of the node: ";
                cin >> input;
            }
            node.set_quality(input);
            
            cout << input << " " << node.get_resource() << "\nIs that correct? (y/n): ";
            cin >> input;
            if (input != "y") {
                continue;
            }

            coordinate_flag = true;
            while (coordinate_flag) {
                cout << "Enter the x- and y-coordinates (q to quit) ";
                cin >> input;
                if (input == "q") {
                    coordinate_flag = false;
                    continue;
                }
                int x, y;
                x = stoi(input);
                cin >> y;
                node.set_position(x, y);

                cout << node.get_x() << " " << node.get_y() << "\nIs that correct? (y/n): ";
                cin >> input;
                if (input != "y") {
                    continue;
                }

                bool duplicate_node = false;
                for (int i = 0; i<nodes.size() && !duplicate_node; i++) {
                    if (nodes[i]==node) {
                        cout << "Found in list: " << nodes[i].to_string() << endl;
                        duplicate_node = true;
                    }
                }
                if (duplicate_node) {
                    cout << "Node already exists, not adding to file." << endl;
                    continue;
                }

                node.set_id(ID_num);
                ID_num += 1;

                well_nodes.push_back(node);
                nodes.push_back(node);
                fout << node.to_string() << '\n';
                cout << "Added to well. " << (++count) << " values entered so far." << endl;

                cout << "Is that all nodes? (y/n): ";
                cin >> input;

                if (input == "y") {
                    Node center = node.create_center_node(well_nodes, ID_num);
                    ID_num += 1;
                    nodes.push_back(center);
                    fout << center.to_string() << '\n';
                    cout << "Full well added to file." << endl;
                    quality_flag = false;
                    coordinate_flag = false;
                }
            }
        }
    }
    fout.close();

    return 0;
}








/*
450 Oil 267123 115156 0
900 Oil -263342 72360 0
1350 Oil -43239 -6519 0
1650 Nitrogen 214660 137994 0
3000 Nitrogen 87991 105052 0
2100 Nitrogen -144342 124610 0
1950 Nitrogen -249449 -188216 0
1800 Nitrogen 225242 -309813 0
*/