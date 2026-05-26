// #include "../lib/json.hpp"

// #include <cmath>
// #include <climits>

#include <string>
// #include <vector>
// #include <stack>
// #include <map>
// #include <unordered_map>
// #include <algorithm>
// #include <numeric>

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;
// using json = nlohmann::ordered_json;

#define ANSI_RED "\033[31m"

#define ANSI_RESET "\033[0m"

#define ifstream(stream, file) ifstream stream(file);\
    if (!stream.is_open()) {\
        cerr << ANSI_RED << "File failed to open:" << endl << '\t' << file << ANSI_RESET << endl;\
        return 1;\
    }

#define ofstream(stream, file) ofstream stream(file);\
    if (!stream.is_open()) {\
        cerr << ANSI_RED << "File failed to open:" << endl << '\t' << file << ANSI_RESET << endl;\
        return 1;\
    }

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();

    ifstream(recipe_in, exePath / "dat" / "recipes.json");
    ifstream(test_recipe_in, exePath / "dat" / "test_input.json");
    ifstream(terminal_recipe_in, exePath / "dat" / "terminal_resources.json");
    ifstream(filters_in, exePath / "dat" / "100_combination_filter.json");
    ofstream(results, exePath / "dat" / "test_results.json");
    ofstream(status_log, exePath / "dat" / "test_status.log");

}