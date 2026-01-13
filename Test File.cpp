#include "json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

int main(int argc, char* argv[]) {
    string str1 = "Packaged";
    string str2 = "Unpackage";

    if (str1.find("package") != string::npos) {
        cout << str1.find("package") << endl;
    }
    else { 
        cout << "\"package\" not found in \"Packaged\"." << endl;
    }

    if (str2.find("package") != string::npos) {
        cout << str2.find("package") << endl;
    }
    else { 
        cout << "\"package\" not found in \"Unpackage\"." << endl;
    }
}