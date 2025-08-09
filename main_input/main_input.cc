#include <vector>
#include <fstream>
#include "node_input.h"

using namespace std;

int main() {
    string PATH = "C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\satisfactory_node_data.txt";
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
            int count = 0;
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
                nodes.push_back(node);
                fout << node.to_string() << '\n';
                cout << "Added to file. " << (++count) << " values entered so far." << endl;
            }
        }
    }
    fout.close();

    return 0;
}