#ifndef INCREMENTOR_H
#define INCREMENTOR_H

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class Incrementor {
    vector<size_t> selected_alternate;
    vector<size_t> alternate_count;
    unordered_map<string,size_t> name_to_index;
    public:
        /**
         * @brief add a digit to the incrementor
         * 
         * @param name category, for map indexing
         * @param alternate selected alternate recipe
         * @param alternate_count the amount of alternate recipes
         */
        void push_back(string name, size_t alternate, size_t alternate_count) {
            name_to_index.insert({name,selected_alternate.size()});
            selected_alternate.push_back(alternate);
            this->alternate_count.push_back(alternate_count);
        }

        size_t get_index(string name) const {
            return name_to_index.at(name);
        }
        size_t& alternate(size_t index) {
            return selected_alternate[index];
        }
        size_t& alternate_max(size_t index) {
            return alternate_count[index];
        }
        size_t size() const {
            return selected_alternate.size();
        }

        bool all_zeros() const {
            for (size_t i = 0; i<selected_alternate.size(); i++) {
                if (selected_alternate[i]!=0) {
                    return false;
                }
            }
            return true;
        }
};

#endif
