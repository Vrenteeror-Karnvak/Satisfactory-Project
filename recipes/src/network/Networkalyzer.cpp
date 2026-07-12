#include "../lib/json.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <set>

using namespace std;
namespace fs = filesystem;

using json = nlohmann::ordered_json;

struct ingredient {
    string name;
    float amount;

    bool operator<(const ingredient& other) const {
        return tie(name, amount) < tie(other.name, other.amount);
    }
};

struct recipe {
    string displayName;
    vector<ingredient> ingredients;
    vector<ingredient> products;
    string producedIn;
    float manufacturingDuration;

    bool operator<(const recipe& other) const {
        return tie(displayName, ingredients, products, producedIn, manufacturingDuration)
        < tie(other.displayName, other.ingredients, other.products, other.producedIn, other.manufacturingDuration);
    }
};

#define is_open(file) if (!file.is_open()) {cerr << "Failed to open a file\n"; return 1;}

int main(int argc, char** argv) {
    fs::path exePath = fs::absolute(argv[0]).parent_path();

    ifstream recipesIn(exePath / "dat" / "recipes.json");
    is_open(recipesIn)
    json recipes;
    recipesIn >> recipes;

    ofstream rawDataRecipesOut(exePath / "int" / "rawDataRecipes.txt");

    map<string,vector<recipe>> m;

    for (auto category : recipes) {
        string c = category["Category"];

        for (auto o : category["Data"]) {
            recipe r;
            r.displayName = o["DisplayName"];
            cout << r.displayName << ' ';

            for (auto oo : o["Ingredients"]) {
                ingredient i;
                i.name = oo["ItemClass"];
                i.amount = std::stof(static_cast<string>((oo["Amount"])));
                r.ingredients.push_back(i);
                cout << i.name << ' ' << i.amount << ' ';
            }
            for (auto oo : o["Product"]) {
                ingredient i;
                i.name = oo["ItemClass"];
                i.amount = std::stof(static_cast<string>(oo["Amount"]));
                r.products.push_back(i);
                cout << i.name << ' ' << i.amount << ' ';
            }
            r.producedIn = o["ProducedIn"];
            r.manufacturingDuration = std::stof(static_cast<string>(o["ManufactoringDuration"]));
            m[c].push_back(r);

            cout << r.producedIn << ' ' << r.manufacturingDuration << endl;
        }
    }

    string query = "Ficsonium Fuel Rod";
    cout << m[query].size() << endl;
    // return 0;

    queue<string> q;
    queue<size_t> qL;
    q.push(query);

    set<string> seen;
    vector<set<recipe>> layers;

    while (q.size()>0) {
        string category = q.front();
        q.pop();
        size_t ll = qL.front();
        qL.pop();
        if (seen.count(category)>0) continue;
        seen.insert(category);
        if (ll>=layers.size()) layers.emplace_back();

        cout << "Layer " << ll << '\t' << category << endl;

        vector<recipe> recipes = m[category];
        for (auto o : recipes) {
            layers[ll].insert(o);
            for (auto oo : o.ingredients) {
                q.push(oo.name);
                qL.push(ll+1);
            }
        }
    }
}