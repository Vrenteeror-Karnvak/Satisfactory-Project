#include "json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::ordered_json;

class Resource {
    public:
        Resource(string name_in, int amount_in) {
            name = name_in;
            amount = amount_in;
        }
        string get_name() {return name;}
        int get_amount() {return amount;}
    private:
        string name;
        int amount;
};

class recipe {
    public:

    private:
        vector<Resource> inputs;
        vector<Resource> outputs;
        double speed;
};

void variableInitializer(string path);

int main() {
    string name_in = "C:\\Users\\adamb\\OneDrive\\Documents\\GitHub\\Satisfactory-Project\\names.json";
    string recipe_in = "C:\\Users\\adamb\\OneDrive\\Documents\\GitHub\\Satisfactory-Project\\recipes.json";

    variableInitializer(name_in);
    variableInitializer(recipe_in);

    ifstream fin(recipe_in);
    ofstream fout("values.txt");

    json data;
    fin >> data;

    int input;
    double output;

    for (const auto& block : data) {
        input = stoi(data.value("ManufactoringDuration", ""));
        output = 60/input;
        fout << output << "\n";
    }

    fin.close();
    fout.close();

    return 0;
}

void variableInitializer(string path) {
    ifstream fin(path);
    
    json data;
    fin >> data;
    
    fin.close();
}