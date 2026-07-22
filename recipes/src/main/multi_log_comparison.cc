#include "../lib/json.hpp"
#include <cmath>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

#include <chrono>

#include "../resources/recipe.h"
#include "../resources/resource.h"
#include "../resources/fraction.h"

#include "../resources/stats.h"

using namespace std;
using json = nlohmann::ordered_json;

void output_raw_lines(ofstream& status_log, vector<ifstream>& log_files, size_t num);
void int_compare(ofstream& status_log, vector<ifstream>& log_files, size_t num);
void double_compare(ofstream& status_log, vector<ifstream>& log_files, size_t num);
void timing_break_down(ofstream& status_log, vector<ifstream>& log_files, size_t num);

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    ofstream status_log(exePath / "int" / "multi_log_comparison.log");

    size_t number_of_items = 0;
    {
        ifstream recipe_in(exePath / "dat" / "recipes.json");
        json recipe_root;
        recipe_in >> recipe_root;
        recipe_in.close();
        number_of_items = recipe_root.size();
    }

    ifstream test_recipe_in(exePath / "dat" / "test_input.json");
    json test_recipe_root;
    test_recipe_in >> test_recipe_root;
    test_recipe_in.close();
    size_t quantity = test_recipe_root[5].value("quantity", 0); // The number of log files

    vector<ifstream> log_files;
    filesystem::path folder = test_recipe_root[5].value("folder_for_all", "");
    bool error = false;
    for (size_t i = 1; i <= quantity; i++) {
        string current_json_index = "file" + to_string(i);
        filesystem::path file = test_recipe_root[5].value(current_json_index, "N/A"); // The current log file
        log_files.emplace_back(exePath / folder / file);
        if (!log_files.back().is_open()) {
            cerr << "Failed to open " << file << ".\n";
            error = true;
        }
        else {
            cerr << "Successfully opened " << file << ".\n";
        }
    }

    if (error) {
        return 0;
    }
    
    string line; // The current line from the output stream
    vector<string> current_lines(quantity, "");

    // Handles the item by item data
    for (size_t i = 0; i < number_of_items; i++) {
        // Checks if the item was skipped
        for (size_t k = 0; k < quantity; k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        line = current_lines[0];
        if (line.find("Skipping") != string::npos) {
            status_log << line << endl << endl;
            for (size_t k = 0; k < quantity; k++) {
                ifstream& log = log_files[k];
                getline(log, current_lines[k]);
            }
            continue;
        }
        
        // Removes the next two lines
        for (size_t j = 0; j < 2; j++) {
            for (size_t k = 0; k < quantity; k++) {
                ifstream& log = log_files[k];
                getline(log, current_lines[k]);
            }
        }

        // Handles the potential of a filter getting made and removing the extra data
        for (size_t k = 0; k < quantity; k++) {
            if (current_lines[k].find("filter") != string::npos) {
                ifstream& log = log_files[k];
                for (size_t j = 0; j < 3; j++) {
                    getline(log, current_lines[k]);
                }
            }
        }

        // Handles the potential of an error regarding the predition and the total processed
        for (size_t k = 0; k < quantity; k++) {
            if (current_lines[k].find("total") != string::npos) {
                ifstream& log = log_files[k];
                getline(log, current_lines[k]);
            }
        }

        // Outputs the final line from the lines gathered, which is the item name
        status_log << current_lines[0] << endl;

        // Handles potention differences in the number of combinations processed and output
        {
            vector<int> processed(quantity, 0);
            vector<int> output(quantity, 0);
            vector<int> filtered(quantity, 0);

            for (size_t k = 0; k < quantity; k++) {
                ifstream& log = log_files[k];

                log >> processed[k];
                getline(log, current_lines[k]);

                log >> output[k];
                getline(log, current_lines[k]);

                log >> filtered[k];
                getline(log, current_lines[k]);
            }

            bool difference_detected = false;
            for (size_t k = 1; k < quantity; k++) {
                if (processed[k - 1] != processed[k] || output[k - 1] != output[k] || filtered[k - 1] != filtered[k]) {
                    difference_detected = true;
                    break;
                }
            }

            if (difference_detected) {
                status_log << "There has been changes in the number of combinations processed and output." << endl;
                // Changes in the number of combinations processed
                for (size_t k = 0; k < quantity; k++) {
                    status_log << processed[k];
                    if (k == (quantity - 1)) {
                        status_log << " combinations have been processed." << endl;
                    }
                    else {
                        status_log << " >> ";
                    }
                }

                // Changes in the number of combinations output
                for (size_t k = 0; k < quantity; k++) {
                    status_log << output[k];
                    if (k == (quantity - 1)) {
                        status_log << " recipes were output." << endl;
                    }
                    else {
                        status_log << " >> ";
                    }
                }

                // Changes in the number of combinations filtered
                for (size_t k = 0; k < quantity; k++) {
                    status_log << filtered[k];
                    if (k == (quantity - 1)) {
                        status_log << " recipes were filtered due to number of machines." << endl;
                    }
                    else {
                        status_log << " >> ";
                    }
                }
            }
            else {
                status_log << "No changes detected in the number of combinations processed and output." << endl;
            }
        }

        // Handles potention differences in the amount of time to process the item
        {
            vector<double> execution(quantity, 0.0);
            vector<double> output(quantity, 0.0);

            for (size_t k = 0; k < quantity; k++) {
                ifstream& log = log_files[k];

                getline(log, current_lines[k], ':');
                log >> execution[k];
                getline(log, current_lines[k]);

                getline(log, current_lines[k], ':');
                log >> output[k];
                getline(log, current_lines[k]);
            }

            status_log << "There has been changes in the amount of time to process the item." << endl;

            // Changes in the total execution time
            status_log << "Execution time: ";
            for (size_t k = 0; k < quantity; k++) {
                status_log << execution[k];
                if (k == (quantity - 1)) {
                    status_log << " seconds." << endl;
                }
                else {
                    status_log << " >> ";
                }
            }

            // Changes in the output time
            status_log << "Output time: ";
            for (size_t k = 0; k < quantity; k++) {
                status_log << output[k];
                if (k == (quantity - 1)) {
                    status_log << " seconds." << endl;
                }
                else {
                    status_log << " >> ";
                }
            }
        }

        for (size_t k = 0; k < quantity; k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        status_log << endl;
    }

    // Handles the overall data at the bottom
    {
        vector<int> processed(quantity, 0);
        vector<int> output(quantity, 0);
        vector<int> filtered(quantity, 0);
        vector<double> execution(quantity, 0.0);

        for (size_t k = 0; k < quantity; k++) {
            ifstream& log = log_files[k];

            log >> processed[k];
            getline(log, current_lines[k]);

            log >> output[k];
            getline(log, current_lines[k]);

            log >> filtered[k];
            getline(log, current_lines[k]);

            getline(log, current_lines[k], ':');
            log >> execution[k];
            getline(log, current_lines[k]);
        }
        
        // Changes in the number of combinations processed
        for (size_t k = 0; k < quantity; k++) {
            status_log << processed[k];
            if (k == (quantity - 1)) {
                status_log << " combinations have been processed." << endl;
            }
            else {
                status_log << " >> ";
            }
        }

        // Changes in the number of combinations output
        for (size_t k = 0; k < quantity; k++) {
            status_log << output[k];
            if (k == (quantity - 1)) {
                status_log << " recipes were output." << endl;
            }
            else {
                status_log << " >> ";
            }
        }

        // Changes in the number of combinations filtered
        for (size_t k = 0; k < quantity; k++) {
            status_log << filtered[k];
            if (k == (quantity - 1)) {
                status_log << " recipes were filtered due to number of machines." << endl;
            }
            else {
                status_log << " >> ";
            }
        }

        // Changes in the total execution time
        status_log << "Execution time: ";
        for (size_t k = 0; k < quantity; k++) {
            status_log << execution[k];
            if (k == (quantity - 1)) {
                status_log << " seconds." << endl;
            }
            else {
                status_log << " >> ";
            }
        }
    }

    // Handles the stats data all the way at the bottom
    for (size_t i = 0; i < 4; i++) {
        // Summary
        output_raw_lines(status_log, log_files, 8);
        int_compare(status_log, log_files, 4);
        double_compare(status_log, log_files, 3);
        int_compare(status_log, log_files, 4);

        // Filters
        output_raw_lines(status_log, log_files, 2);
        int_compare(status_log, log_files, 8);

        // Function Calls
        output_raw_lines(status_log, log_files, 2);
        int_compare(status_log, log_files, 6);

        // Average Workload
        output_raw_lines(status_log, log_files, 2);
        double_compare(status_log, log_files, 3);

        // Timing Breakdown
        output_raw_lines(status_log, log_files, 2);
        timing_break_down(status_log, log_files, 10);

        // Function Cost
        output_raw_lines(status_log, log_files, 2);
        double_compare(status_log, log_files, 4);

        // Peak Values
        output_raw_lines(status_log, log_files, 2);
        int_compare(status_log, log_files, 5);
    }

    cout << "Log comparison complete." << endl;
    status_log << "Log comparison complete." << endl;

    return 0;
}



void output_raw_lines(ofstream& status_log, vector<ifstream>& log_files, size_t num) {
    vector<string> current_lines(log_files.size(), "");

    for (size_t j = 0; j < num; j++) {
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        status_log << current_lines[0] << endl;
    }
}

void int_compare(ofstream& status_log, vector<ifstream>& log_files, size_t num) {
    vector<int> value(log_files.size(), 0);
    vector<string> current_lines(log_files.size(), "");

    for (size_t j = 0; j < num; j++) {
        // Before the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k], ':');
        }
        status_log << current_lines[0] << ": ";

        // The value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            log >> value[k];
            status_log << value[k];
            if (k < (log_files.size() - 1)) {
                status_log << " >> ";
            }
        }

        // After the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        status_log << endl;
    }
}

void double_compare(ofstream& status_log, vector<ifstream>& log_files, size_t num) {
    vector<double> value(log_files.size(), 0);
    vector<string> current_lines(log_files.size(), "");

    for (size_t j = 0; j < num; j++) {
        // Before the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k], ':');
        }
        status_log << current_lines[0] << ": ";

        // The value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            log >> value[k];
            status_log << value[k];
            if (k < (log_files.size() - 1)) {
                status_log << " >> ";
            }
        }

        // After the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        status_log << current_lines[0] << endl;
    }
}

void timing_break_down(ofstream& status_log, vector<ifstream>& log_files, size_t num) {
    vector<double> value(log_files.size(), 0);
    vector<string> current_lines(log_files.size(), "");
    
    for (size_t j = 0; j < num; j++) {
        // Before the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k], ':');
        }
        status_log << current_lines[0] << ": ";

        // The value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            log >> value[k];
            status_log << value[k] << "%";
            if (k < (log_files.size() - 1)) {
                status_log << " >> ";
            }
        }

        // After the value
        for (size_t k = 0; k < log_files.size(); k++) {
            ifstream& log = log_files[k];
            getline(log, current_lines[k]);
        }
        status_log << endl;
    }
}