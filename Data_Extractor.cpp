#include "json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using json = nlohmann::json;

int main() {
    vector<string> clusters;

    ifstream file("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\en-US.json");
    ofstream fout("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\recipes.txt");
    string line;
    int lineNumber = 0;

    bool inCluster = false;
    string currentCluster;

    while (getline(file, line)) {
        lineNumber++;

        if (lineNumber < 28292) continue;
        if (lineNumber > 40275) break;

        if (line.find('{') != string::npos) {
            inCluster = true;
            currentCluster.clear();
            continue;
        }

        if (line.find('}') != string::npos && inCluster) {
            inCluster = false;
            clusters.push_back(currentCluster);
            fout << currentCluster << "\n\n\n";
            continue;
        }

        if (inCluster) {
            currentCluster += line + "\n";
        }
    }

    fout.close();
}

