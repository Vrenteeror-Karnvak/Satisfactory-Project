// Uncompressed JSON output:
    json chain_object = json::object();
    json chain_array = json::array();
    // converts the output vector into uncompressed json
    for (size_t i = 0; i < output_recipes.size(); i++) {
        chain_object = output_recipes[i].to_json();
        chain_array.push_back(chain_object);
    }
    output_chain = chain_array;
    output_array.push_back(output_chain);



// Compressed JSON output:
    bool first = true; // is this the first item being output in the given array?
    json output_chain = json::object(); // the current recipe chain being processed
    json output_array = json::array(); // the recipes being output into the file
    json output_object = json::object(); // the json object containing all recipe chains being sent to the file
    output_chain = output.to_compressed_json();
    output_object["Category"] = test_item;
    output_object["Data"] = output_array;

    // preps the array to be output
    output_array.clear();
    for (size_t i = 0; i < output_vector.size(); i++) {
        output_vector[i].set_machine_speed(60.0);
        output_chain = output_vector[i].to_compressed_json();
        output_array.push_back(output_chain);
    }

    // outputs the array to the file
    if (!first) {
        results << "," << endl;
    }
    first = false;

    // Outputs recipe by recipe without a buffer
    results << output_object.dump(4);
    results.flush();

    // Outputs recipe by recipe with a buffer
    string output_string = output_object.dump(4); // output_object dumped into a string for line by line writing
    string buffer = "    ";
    for (char c : output_string) {
        buffer += c;
        if (c == '\n') buffer += "    ";
    }
    results << buffer;
    buffer.clear();

    // Outputs all at once
    output_array.clear();
    output_vector = (*recipes_ptr).at(test_item);
    for (size_t i = 0; i < output_vector.size(); i++) {
        output_chain = output_vector[i].to_compressed_json();
        output_array.push_back(output_chain);
    }
    recipe_root[m]["Data"] = output_array;

// Output size reducer:
    // If the output_vector is still too large, halve the max output allowed to reduce combinations
    if (output_vector.size() > absolute_max_output) {
        if ((*minimum).get_machine_speed() == filter_map.at(test_item) || ((*minimum).get_machine_speed() < 10 && filter_map.at(test_item) == 10)) {
            cout << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
            cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
            status_log << test_item << " filter at smallest possible value. Made using max_output of " << max_output << "." << endl;
            status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
            max_output = base_max_output;
        }
        else {
            cout << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
            cout << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
            status_log << test_item << " filter needs to be remade. Made using max_output of " << max_output << "." << endl;
            status_log << "Uses a filter value of " << filter_map.at(test_item) << " to produce " << output_vector.size() << " combinations." << endl;
            redo_filter = true;
            filter_map.at(test_item) = 0;
            max_output /= 2;
            true_unfiltered -= unfiltered;
            true_machine_filtered -= machine_filtered;
            k--;
            continue;
        }
    }
    else {
        max_output = base_max_output;
    }