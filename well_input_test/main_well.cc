#include <fstream>

#include "node_well.h"

using namespace std;

int main() {
    string PATH = "C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\satisfactory_node_data_test.txt";
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