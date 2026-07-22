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

void output_raw_lines(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void int_more(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void int_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void second_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void per_second_more(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void average_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void timing_break_down(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);
void double_max_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num);

int main(int argc, char* argv[]) {
    filesystem::path exePath = filesystem::absolute(argv[0]).parent_path();
    ofstream status_log(exePath / "int" / "speed_test_results.log");

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
    filesystem::path folder = test_recipe_root[5].value("folder_for_all", "");
    filesystem::path file1 = test_recipe_root[5].value("file1", ""); // The first log file
    filesystem::path file2 = test_recipe_root[5].value("file2", ""); // The second log file
    test_recipe_in.close();

    ifstream log_file_1(exePath/ folder / file1);
    ifstream log_file_2(exePath/ folder / file2);

    bool error = false;
    if (!log_file_1.is_open()) {
        cout << "Log file 1 did not open." << endl;
        error = true;
    }
    if (!log_file_2.is_open()) {
        cout << "Log file 2 did not open." << endl;
        error = true;
    }

    if (error) {
        return 0;
    }
    
    string line1; // The current line from log_file_1
    string line2; // The current line from log_file_2

    // Handles the item by item data
    for (size_t i = 0; i < number_of_items; i++) {
        // Checks if the item was skipped
        getline(log_file_1, line1);
        getline(log_file_2, line2);
        if (line1.find("Skipping") != string::npos) {
            status_log << line1 << endl << endl;
            getline(log_file_1, line1);
            getline(log_file_2, line2);
            continue;
        }
        
        // Removes the next two lines
        for (size_t j = 0; j < 2; j++) {
            getline(log_file_1, line1);
            getline(log_file_2, line2);
        }

        // Handles the potential of a filter getting made and removing the extra data
        if (line1.find("filter") != string::npos) {
            for (size_t j = 0; j < 3; j++) {
                getline(log_file_1, line1);
            }
        }
        if (line2.find("filter") != string::npos) {
            for (size_t j = 0; j < 3; j++) {
                getline(log_file_2, line2);
            }
        }

        // Handles the potential of an error regarding the predition and the total processed
        if (line1.find("total") != string::npos) {
            getline(log_file_1, line1);
        }
        if (line2.find("total") != string::npos) {
            getline(log_file_2, line2);
        }

        // Outputs the final line from the lines gathered, which is the item name
        status_log << line1 << endl;

        // Handles potention differences in the number of combinations processed and output
        {
            int processed_1 = 0;
            int output_1 = 0;
            int filtered_1 = 0;
            log_file_1 >> processed_1;
            getline(log_file_1, line1);
            log_file_1 >> output_1;
            getline(log_file_1, line1);
            log_file_1 >> filtered_1;
            getline(log_file_1, line1);

            int processed_2 = 0;
            int output_2 = 0;
            int filtered_2 = 0;
            log_file_2 >> processed_2;
            getline(log_file_2, line2);
            log_file_2 >> output_2;
            getline(log_file_2, line2);
            log_file_2 >> filtered_2;
            getline(log_file_2, line2);

            if (processed_1 != processed_2 || output_1 != output_2 || filtered_1 != filtered_2) {
                status_log << "There has been changes in the number of combinations processed and output." << endl;
                status_log << processed_2 - processed_1 << " more combinations have been processed." << endl;
                status_log << output_2 - output_1 << " more recipes were output." << endl;
                status_log << filtered_2 - filtered_1 << " more recipes were filtered due to number of machines." << endl;
            }
            else {
                status_log << "No changes detected in the number of combinations processed and output." << endl;
            }
        }

        // Handles potention differences in the amount of time to process the item
        {
            string word;

            double execution_1 = 0;
            double output_1 = 0;
            getline(log_file_1, line1, ':');
            log_file_1 >> execution_1;
            getline(log_file_1, line1);
            getline(log_file_1, line1, ':');
            log_file_1 >> output_1;
            getline(log_file_1, line1);

            double execution_2 = 0;
            double output_2 = 0;
            getline(log_file_2, line2, ':');
            log_file_2 >> execution_2;
            getline(log_file_2, line2);
            getline(log_file_2, line2, ':');
            log_file_2 >> output_2;
            getline(log_file_2, line2);

            if ((execution_2/execution_1 <= 0.97 || execution_2/execution_1 >= 1.03) || (output_2/output_1 <= 0.97 || output_2/output_1 >= 1.03)) {
                status_log << "There has been changes in the amount of time to process the item." << endl;
                status_log << "Execution time: " << execution_1 - execution_2 << " less seconds." << endl;
                status_log << "Output time: " << output_1 - output_2 << " less seconds." << endl;
            }
            else {
                status_log << "No changes detected in the amount of time to process the item." << endl;
            }
        }

        getline(log_file_1, line1);
        getline(log_file_2, line2);
        status_log << endl;
    }

    // Handles the overall data at the bottom
    {
        int processed_1 = 0;
        int output_1 = 0;
        int filtered_1 = 0;
        log_file_1 >> processed_1;
        getline(log_file_1, line1);
        log_file_1 >> output_1;
        getline(log_file_1, line1);
        log_file_1 >> filtered_1;
        getline(log_file_1, line1);

        int processed_2 = 0;
        int output_2 = 0;
        int filtered_2 = 0;
        log_file_2 >> processed_2;
        getline(log_file_2, line2);
        log_file_2 >> output_2;
        getline(log_file_2, line2);
        log_file_2 >> filtered_2;
        getline(log_file_2, line2);

        string word;

        double execution_1 = 0;
        getline(log_file_1, line1, ':');
        log_file_1 >> execution_1;
        getline(log_file_1, line1);

        double execution_2 = 0;
        getline(log_file_2, line2, ':');
        log_file_2 >> execution_2;
        getline(log_file_2, line2);

        status_log << processed_2 - processed_1 << " more total combinations have been processed." << endl;
        status_log << output_2 - output_1 << " more total recipes were output." << endl;
        status_log << filtered_2 - filtered_1 << " more total recipes were filtered due to number of machines." << endl;
        status_log << "Total Execution time: " << execution_1 - execution_2 << " less seconds." << endl;
    }

    // Handles the stats data all the way at the bottom
    for (size_t i = 0; i < 4; i++) {
        // Summary
        output_raw_lines(status_log, log_file_1, log_file_2, 8);
        int_more(status_log, log_file_1, log_file_2, 4);
        second_less(status_log, log_file_1, log_file_2, 2);
        per_second_more(status_log, log_file_1, log_file_2, 1);
        int_less(status_log, log_file_1, log_file_2, 2);
        int_more(status_log, log_file_1, log_file_2, 2);

        // Filters
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        int_more(status_log, log_file_1, log_file_2, 4);
        int_less(status_log, log_file_1, log_file_2, 4);

        // Function Calls
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        int_less(status_log, log_file_1, log_file_2, 6);

        // Average Workload
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        average_less(status_log, log_file_1, log_file_2, 3);

        // Timing Breakdown
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        timing_break_down(status_log, log_file_1, log_file_2, 10);

        // Function Cost
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        second_less(status_log, log_file_1, log_file_2, 4);

        // Peak Values
        output_raw_lines(status_log, log_file_1, log_file_2, 2);
        double_max_less(status_log, log_file_1, log_file_2, 5);
    }

    cout << "Log comparison complete." << endl;
    status_log << endl << endl << endl << "Log comparison complete." << endl;

    // Checks how long an integer comparison is
    volatile bool result = false;
    chrono::duration<double, nano> speed_elapsed;
    size_t N = 1000000000;
    int int_a = 1000000000;
    int int_b = 1000000000;
    auto int_speed_start = chrono::steady_clock::now();
    for (size_t i = 0; i < N; i++) {
        result = (int_a == int_b);
    }
    auto int_speed_end = chrono::steady_clock::now();
    speed_elapsed = int_speed_end - int_speed_start;
    cout << (speed_elapsed).count()/N << " nano seconds per integer comparison operation." << endl;
    status_log << endl << endl << (speed_elapsed).count()/N << " nanoseconds per integer comparison operation." << endl;

    // Checks how long a 20 character string comparison is
    string string_a = "Heavy_Modular_Frame!";
    string string_b = "Heavy_Modular_Frame!";
    auto string_speed_start = chrono::steady_clock::now();
    for (size_t i = 0; i < N; i++) {
        result = (string_a == string_b);
    }
    auto string_speed_end = chrono::steady_clock::now();
    speed_elapsed = string_speed_end - string_speed_start;
    cout << (speed_elapsed).count()/N << " nano seconds per 20 character string comparison operation." << endl;
    status_log << (speed_elapsed).count()/N << " nano seconds per 20 character string comparison operation." << endl;

    if (result) {
        cout << "Things worked right" << endl;
    }
}



void output_raw_lines(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    string line;
    for (size_t j = 0; j < num; j++) {
        getline(log_file_1, line);
        getline(log_file_2, line);
        status_log << line << endl;
    }
}

void int_more(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        int value1 = 0;
        int value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value2 - value1 << " more." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void int_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        int value1 = 0;
        int value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value1 - value2 << " less." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void second_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        double value1 = 0;
        double value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value1 - value2 << " less seconds." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void per_second_more(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        double value1 = 0;
        double value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value2 - value1 << " more per second." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void average_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        double value1 = 0;
        double value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value1 - value2 << " less on average." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void timing_break_down(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        double value1 = 0;
        double value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value1 << "% -> " << value2 << "%" << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}

void double_max_less(ofstream& status_log, ifstream& log_file_1, ifstream& log_file_2, size_t num) {
    for (size_t j = 0; j < num; j++) {
        double value1 = 0;
        double value2 = 0;
        string line;

        getline(log_file_1, line, ':');
        getline(log_file_2, line, ':');
        status_log << line << ": ";

        log_file_1 >> value1;
        log_file_2 >> value2;
        status_log << value1 - value2 << " less." << endl;

        getline(log_file_1, line);
        getline(log_file_2, line);
    }
}