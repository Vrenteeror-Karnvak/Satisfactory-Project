#include "node.h"

using namespace std;

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