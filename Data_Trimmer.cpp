#include "json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using json = nlohmann::json;

int main() {
    vector<string> clusters;

    ifstream file("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\recipes.txt");
    ofstream fout("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\filtered_recipes.txt");
    string line;
    int lineNumber = 0;

    string neededData;

    while (getline(file, line)) {
        lineNumber++;

        if (lineNumber == 3 || lineNumber == 4 || lineNumber == 5 || lineNumber == 7 || lineNumber == 9) {
            neededData += line + "\n";
        }
        
        if (lineNumber == 13 || lineNumber == 14 || lineNumber == 15) {
            neededData += "\n";
            if (lineNumber == 15) {
                lineNumber = 0;
                if (neededData.find("BuildGun") != string::npos) {
                    neededData.clear();
                    continue;
                }
                fout << neededData;
                neededData.clear();
            }
        }
    }

    fout.close();
}

