#ifndef STATS_CC
#define STATS_CC

#include "stats.h"

using namespace std;

bool MethodStats::has_data() const {
    return item_count != 0 || combinations_processed != 0 || chains_generated != 0 || ID_filtered != 0 || speed_filtered != 0 || merge_filtered != 0 || total_time.count() != 0 || count_time.count() != 0 || ID_total_time.count() != 0 || ID_build_time.count() != 0 || chain_generation_time.count() != 0 || merge_time.count() != 0 || combine_time.count() != 0 || lcm_time.count() != 0 || incrementor_time.count() != 0 || incrementor_rebuild_time.count() != 0 || output_time.count() != 0;
}

void MethodStats::print(std::ostream& out, const string& method) {
    const auto survived_id_filter = combinations_processed - ID_filtered;
    const auto survived_speed_filter = survived_id_filter - speed_filtered;
    const auto survived_merge_filter = survived_speed_filter - merge_filtered;
    
    out << "\n\n\n";
    out << "========================================\n";
    out << "=== " << method << " Method" << " ===\n";
    out << "========================================\n";
    out << "\n=== Summary ===\n";
    out << "Items processed: " << item_count << " (raw count)\n";
    out << "Combinations processed: " << combinations_processed << " (raw count)\n";
    out << "Chains generated: " << chains_generated << " (raw count)\n";
    out << "Recipes output: " << survived_merge_filter << " (" << (combinations_processed > 0 ? 100.0 * survived_merge_filter / combinations_processed : 0.0) << "%)\n";
    out << "Runtime: " << total_time.count() << " seconds\n";
    out << "Seconds per combination: " << (combinations_processed > 0 ? total_time.count() / combinations_processed : 0.0) << " seconds \n";
    out << "Combinations per second: " << (total_time.count() > 0 ? combinations_processed / total_time.count() : 0.0) << " per second\n";
    out << "Resource same_name calls: " << resource_same_name_calls << "\n";
    out << "Recipe same_name calls: " << recipe_same_name_calls << "\n";
    out << "Resource same_product_ID calls: " << resource_same_product_ID_calls << "\n";
    out << "Recipe same_product_ID calls: " << recipe_same_product_ID_calls << "\n";
    out << "\n";

    out << "=== Filters ===\n";
    out << "Passed ID filter: " << survived_id_filter << " (" << (combinations_processed > 0 ? 100.0 * survived_id_filter / combinations_processed : 0.0) << "%)\n";
    out << "Passed speed filter: " << survived_speed_filter << " (" << (combinations_processed > 0 ? 100.0 * survived_speed_filter / combinations_processed : 0.0) << "%)\n";
    out << "Passed merge filter: " << survived_merge_filter << " (" << (combinations_processed > 0 ? 100.0 * survived_merge_filter / combinations_processed : 0.0) << "%)\n";
    out << "Filtered by ID conflict: " << ID_filtered << " (" << (combinations_processed > 0 ? 100.0 * ID_filtered / combinations_processed : 0.0) << "% of combinations)\n";
    out << "Filtered by speed filter: " << speed_filtered << " (" << (survived_id_filter > 0 ? 100.0 * speed_filtered / survived_id_filter : 0.0) << "% of ID-valid combinations)\n";
    out << "Filtered after merge: " << merge_filtered << " (" << (survived_speed_filter > 0 ? 100.0 * merge_filtered / survived_speed_filter : 0.0) << "% of speed-valid combinations)\n";
    out << "\n";

    out << "=== Function Calls ===\n";
    out << "merge_ID calls: " << merge_ID_calls << "\n";
    out << "merge_recipe calls: " << merge_recipe_calls << "\n";
    out << "combine_recipe calls: " << combine_recipe_calls << "\n";
    out << "LCM calls: " << lcm_calls << "\n";
    out << "Fraction reductions: " << fraction_reductions << "\n";
    out << "Incrementor rebuild count: " << incrementor_rebuild_count << "\n";
    out << "\n";

    out << "=== Average Workload ===\n";
    out << "Average ID entries per chain: " << (chains_generated > 0 ? static_cast<double>(total_ID_entries_used) / chains_generated : 0.0) << "\n";
    out << "Average LCMs per chain: " << (chains_generated > 0 ? static_cast<double>(lcm_calls) / chains_generated : 0.0) << "\n";
    out << "Average reductions per chain: " << (chains_generated > 0 ? static_cast<double>(fraction_reductions) / chains_generated : 0.0) << "\n";
    out << "\n";

    out << "=== Timing Breakdown ===\n";
    out << "Count: " << (total_time.count() > 0 ? 100.0 * count_time.count() / total_time.count() : 0.0) << "% (" << (combinations_processed > 0 ? count_time.count() / combinations_processed : 0.0) << " seconds per combination)\n";
    out << "ID Total: " << (total_time.count() > 0 ? 100.0 * ID_total_time.count() / total_time.count() : 0.0) << "% (" << (combinations_processed > 0 ? ID_total_time.count() / combinations_processed : 0.0) << " seconds per combination)\n";
    out << "- ID Build: " << (ID_total_time.count() > 0 ? 100.0 * ID_build_time.count() / ID_total_time.count() : 0.0) << "% (" << (combinations_processed > 0 ? ID_build_time.count() / combinations_processed : 0.0) << " seconds per combination)\n";
    out << "Chain Generation: " << (total_time.count() > 0 ? 100.0 * chain_generation_time.count() / total_time.count() : 0.0) << "% (" << (survived_id_filter > 0 ? chain_generation_time.count() / survived_id_filter : 0.0) << " seconds per surviving chain)\n";
    out << "LCM: " << (total_time.count() > 0 ? 100.0 * lcm_time.count() / total_time.count() : 0.0) << "% (" << (survived_id_filter > 0 ? lcm_time.count() / survived_id_filter : 0.0) << " seconds per surviving chain)\n";
    out << "Merge: " << (total_time.count() > 0 ? 100.0 * merge_time.count() / total_time.count() : 0.0) << "% (" << (survived_speed_filter > 0 ? merge_time.count() / survived_speed_filter : 0.0) << " seconds per surviving chain)\n";
    out << "- Combine: " << (merge_time.count() > 0 ? 100.0 * combine_time.count() / merge_time.count() : 0.0) << "% (" << (survived_speed_filter > 0 ? combine_time.count() / survived_speed_filter : 0.0) << " seconds per surviving chain)\n";
    out << "Incrementor: " << (total_time.count() > 0 ? 100.0 * incrementor_time.count() / total_time.count() : 0.0) << "% (" << (combinations_processed > 0 ? incrementor_time.count() / combinations_processed : 0.0) << " seconds per combination)\n";
    out << "- Rebuild: " << (incrementor_time.count() > 0 ? 100.0 * incrementor_rebuild_time.count() / incrementor_time.count() : 0.0) << "% (" << (incrementor_rebuild_count > 0 ? incrementor_rebuild_time.count() / incrementor_rebuild_count : 0.0) << " seconds per rebuild)\n";
    out << "Output: " << (total_time.count() > 0 ? 100.0 * output_time.count() / total_time.count() : 0.0) << "% (" << (survived_merge_filter > 0 ? output_time.count() / survived_merge_filter : 0.0) << " seconds per output chain)\n";
    out << "\n";

    out << "=== Function Cost ===\n";
    out << "Time/merge_ID: " << (merge_ID_calls > 0 ? ID_build_time.count() / merge_ID_calls : 0.0) << " seconds\n";
    out << "Time/combine_recipe: " << (combine_recipe_calls > 0 ? combine_time.count() / combine_recipe_calls : 0.0) << " seconds\n";
    out << "Time/LCM: " << (lcm_calls > 0 ? lcm_time.count() / lcm_calls : 0.0) << " seconds\n";
    out << "Time/rebuild: " << (incrementor_rebuild_count > 0 ? incrementor_rebuild_time.count() / incrementor_rebuild_count : 0.0) << " seconds\n";
    out << "\n";

    out << "=== Peak Values ===\n";
    out << "Max machine count: " << max_machine_count << "\n";
    out << "Max LCM: " << max_speed_lm << "\n";
    out << "Max ID entries: " << max_ID_entries_used << "\n";
    out << "item_lm = 1: " << item_lm_1 << "\n";
    out << "item_lm > 1: " << item_lm_over_1 << "\n";
}

MethodStats& MethodStats::operator+=(const MethodStats& other) {
    item_lm_1 += other.item_lm_1;
    item_lm_over_1 += other.item_lm_over_1;

    item_count += other.item_count;
    combinations_processed += other.combinations_processed;
    chains_generated += other.chains_generated;
    ID_filtered += other.ID_filtered;
    speed_filtered += other.speed_filtered;
    merge_filtered += other.merge_filtered;
    merge_ID_calls += other.merge_ID_calls;
    merge_recipe_calls += other.merge_recipe_calls;
    combine_recipe_calls += other.combine_recipe_calls;
    lcm_calls += other.lcm_calls;
    fraction_reductions += other.fraction_reductions;
    incrementor_rebuild_count += other.incrementor_rebuild_count;
    total_ID_entries_used += other.total_ID_entries_used;
    max_ID_entries_used = std::max(max_ID_entries_used, other.max_ID_entries_used);
    max_speed_lm = std::max(max_speed_lm, other.max_speed_lm);
    resource_same_name_calls += other.resource_same_name_calls;
    recipe_same_name_calls += other.recipe_same_name_calls;
    resource_same_product_ID_calls += other.resource_same_product_ID_calls;
    recipe_same_product_ID_calls += other.recipe_same_product_ID_calls;
    max_machine_count = std::max(max_machine_count, other.max_machine_count);
    total_time += other.total_time;
    count_time += other.count_time;
    ID_total_time += other.ID_total_time;
    ID_build_time += other.ID_build_time;
    chain_generation_time += other.chain_generation_time;
    merge_time += other.merge_time;
    combine_time += other.combine_time;
    lcm_time += other.lcm_time;
    incrementor_time += other.incrementor_time;
    incrementor_rebuild_time += other.incrementor_rebuild_time;
    output_time += other.output_time;
    return *this;
}

MethodStats MethodStats::operator+(const MethodStats& other) const {
    MethodStats result = *this;
    result += other;
    return result;
}



namespace Stats {
    MethodStats total;
    MethodStats base;
    MethodStats compressed;
    MethodStats nuclear;
    MethodStats* active_method_stats = nullptr;

    MethodStats& current_method_stats() {
        return active_method_stats ? *active_method_stats : total;
    }

    void print(std::ostream& out) {
        total = MethodStats{};
        bool base_has_data = false;
        bool compressed_has_data = false;
        bool nuclear_has_data = false;

        if (base.has_data()) {
            base.print(out, "Base");
            base_has_data = true;
            total += base;
        }
        if (compressed.has_data()) {
            compressed.print(out, "Compressed");
            compressed_has_data = true;
            total += compressed;
        }
        if (nuclear.has_data()) {
            nuclear.print(out, "Nuclear");
            nuclear_has_data = true;
            total += nuclear;
        }
        if (base_has_data || compressed_has_data || nuclear_has_data || total.has_data()) {
            total.print(out, "Combined");
        }
    }
}

#endif