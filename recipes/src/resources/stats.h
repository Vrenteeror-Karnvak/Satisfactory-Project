#ifndef STATS_H
#define STATS_H

#include <algorithm>
#include <cstdint>
#include <ostream>
#include <chrono>

using namespace std;

struct MethodStats {
    // Counts
    uint64_t item_count = 0;

    uint64_t combinations_processed = 0;
    uint64_t chains_generated = 0;

    uint64_t ID_filtered = 0;
    uint64_t speed_filtered = 0;
    uint64_t merge_filtered = 0;
    uint64_t output_filtered = 0;

    uint64_t merge_ID_calls = 0;
    uint64_t merge_recipe_calls = 0;
    uint64_t combine_recipe_calls = 0;

    uint64_t lcm_calls = 0;
    uint64_t fraction_reductions = 0;

    uint64_t item_lm_1 = 0;
    uint64_t item_lm_over_1 = 0;

    uint64_t incrementor_rebuild_count = 0;

    uint64_t total_ID_entries_used = 0;
    uint64_t max_ID_entries_used = 0;

    uint64_t max_speed_lm = 0;
    uint64_t resource_same_name_calls = 0;
    uint64_t recipe_same_name_calls = 0;
    uint64_t resource_same_product_ID_calls = 0;
    uint64_t recipe_same_product_ID_calls = 0;
    double max_machine_count = 0;

    
    // Timers
    chrono::duration<double> total_time{};

    chrono::duration<double> count_time{};
    chrono::duration<double> ID_total_time{};
    chrono::duration<double> ID_build_time{};
    chrono::duration<double> chain_generation_time{};
    chrono::duration<double> merge_time{};
    chrono::duration<double> combine_time{};
    chrono::duration<double> lcm_time{};
    chrono::duration<double> incrementor_time{};
    chrono::duration<double> incrementor_rebuild_time{};
    chrono::duration<double> output_time{};

    void print(std::ostream& out, const string& method);
    bool has_data() const;
    MethodStats& operator+=(const MethodStats& other);
    MethodStats operator+(const MethodStats& other) const;
};

namespace Stats {
    extern MethodStats total;
    extern MethodStats base;
    extern MethodStats compressed;
    extern MethodStats nuclear;
    extern MethodStats* active_method_stats;

    MethodStats& current_method_stats();
    void print(std::ostream& out);
}

#endif