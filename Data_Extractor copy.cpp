#include "json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;
using json = nlohmann::json;

int main() {
    vector<json> filteredObjects;
    json filtered;

    ifstream file("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\en-US.json");
    ofstream fout("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\new_filtered_recipes.txt");
    string line;
    int lineNumber = 0;

    string objectText;
    bool inObject = false;

    while (getline(file, line)) {
        lineNumber++;

        if (lineNumber < 28292) continue;
        if (lineNumber > 40275) break;

        if (line.find('{') != string::npos) {
        inObject = true;
        objectText.clear();
        }

        if (inObject) {
            objectText += line + "\n";
        }

        if (line.find('}') != string::npos && inObject) {
            inObject = false;
            fout << objectText << "\n\n";

            // 🔽 Parse objectText here
            try {
                json j = json::parse(objectText);
                
                if (j.contains("mProducedIn") && j["mProducedIn"].dump().find("BP_BuildGun") != std::string::npos) continue;
                
                filtered["mDisplayName"] = j.value("mDisplayName", nullptr);
                filtered["mIngredients"] = j.value("mIngredients", json::array());
                filtered["mProduct"] = j.value("mProduct", json::array());
                filtered["mManufactoringDuration"] = j.value("mManufactoringDuration", 0.0);
                filtered["mProducedIn"] = j.value("mProducedIn", json::array());

                filteredObjects.push_back(filtered);
            }
            catch (...) {
                // ignore malformed objects
            }
        }
    }

    fout.close();

    ofstream nfout("C:\\Users\\AdamB\\OneDrive\\Desktop\\Satisfactory Coding Project\\json_filtered_recipes.txt");
    int i = 0;

    for (const auto& obj : filteredObjects) {
        nfout << filteredObjects[i];
        i++;
    }
    
    nfout.close();
}

