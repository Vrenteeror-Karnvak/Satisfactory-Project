#ifndef STATS_CC
#define STATS_CC

#include "stats.h"

using namespace std;

namespace Stats {
    uint64_t resource_same_name_calls = 0;
    uint64_t recipe_same_name_calls = 0;
    uint64_t combine_recipe_calls = 0;
    uint64_t merge_recipe_calls = 0;
    uint64_t correct_recipe_calls = 0;
    uint64_t ingredients_merged = 0;
    uint64_t products_merged = 0;
    uint64_t ingredients_cancelled = 0;
    uint64_t products_cancelled = 0;
    uint64_t total_ingredients_before_cleanup = 0;
    uint64_t total_products_before_cleanup = 0;
    uint64_t total_ingredients_after_cleanup = 0;
    uint64_t total_products_after_cleanup = 0;
    uint64_t ingredient_samples = 0;
    uint64_t product_samples = 0;
    uint64_t max_ingredients_before_cleanup = 0;
    uint64_t max_products_before_cleanup = 0;
    uint64_t max_ingredients_after_cleanup = 0;
    uint64_t max_products_after_cleanup = 0;
    uint64_t chains_generated = 0;
    uint64_t recipes_pushed_to_stack = 0;
    uint64_t terminal_hits = 0;
    uint64_t max_chain_depth = 0;
    uint64_t total_output_recipes = 0;
    uint64_t output_recipe_samples = 0;
    uint64_t max_output_recipes = 0;
    uint64_t lcm_calls = 0;
    uint64_t fraction_reductions = 0;
    uint64_t max_speed_lm = 0;
    uint64_t max_item_lm = 0;
    uint64_t total_item_lm = 0;
    uint64_t item_lm_1 = 0;
    uint64_t item_lm_2_10 = 0;
    uint64_t item_lm_11_100 = 0;
    uint64_t item_lm_over_100 = 0;
    uint64_t recipe_map_searches = 0;
    uint64_t recipe_map_lookups = 0;
    uint64_t recipe_map_updates = 0;
    uint64_t incrementor_map_lookups = 0;
    uint64_t terminal_map_searches = 0;
    uint64_t recipes_map_lookups = 0;
    double max_machine_count = 0;
    uint64_t total_machine_filtered_count = 0;
    uint64_t machine_filtered_200_count = 0;
    uint64_t machine_filtered_count = 0;
    uint64_t machine_accepted_count = 0;
    chrono::duration<double> total_generation_time;
    chrono::duration<double> chain_generation_time;
    chrono::duration<double> merge_time;
    chrono::duration<double> combine_time;
    chrono::duration<double> correct_time;
    chrono::duration<double> clean_time;
    chrono::duration<double> lcm_time;

    void print(std::ostream& out) {
        out << "\n=== Profiling Statistics ===\n";

        /**************************************************/
        // Category 1: Comparison Counts
        /**************************************************/

        out << "Resource same_name calls: " << resource_same_name_calls << " (raw count)\n";
        out << "Recipe same_name calls: " << recipe_same_name_calls << " (raw count)\n";

        /**************************************************/
        // Category 2: Recipe Merging
        /**************************************************/

        out << "combine_recipes calls: " << combine_recipe_calls << " (raw count)\n";
        out << "merge_recipes calls: " << merge_recipe_calls << " (raw count)\n";
        out << "merges skiped: " << machine_filtered_200_count << " (raw count)\n";
        out << "merge skip rate: " << (100.0 * machine_filtered_200_count / max<uint64_t>(1, machine_filtered_200_count + merge_recipe_calls)) << "%\n";
        out << "correct_recipe calls: " << correct_recipe_calls << " (raw count)\n";
        out << "Ingredients merged: " << ingredients_merged << " (raw count)\n";
        out << "Products merged: " << products_merged << " (raw count)\n";
        out << "Ingredients cancelled: " << ingredients_cancelled << " (raw count)\n";
        out << "Products cancelled: " << products_cancelled << " (raw count)\n";

        /**************************************************/
        // Category 3: Vector Sizes
        /**************************************************/

        out << "Average ingredients before cleanup: " << static_cast<double>(total_ingredients_before_cleanup) / max<uint64_t>(1, combine_recipe_calls) << " ingredients per combine_recipes call\n";
        out << "Maximum ingredients before cleanup: " << max_ingredients_before_cleanup << " ingredients\n";
        out << "Average products before cleanup: " << static_cast<double>(total_products_before_cleanup) / max<uint64_t>(1, combine_recipe_calls) << " products per combine_recipes call\n";
        out << "Maximum products before cleanup: " << max_products_before_cleanup << " products\n";
        out << "Average ingredients after cleanup: " << static_cast<double>(total_ingredients_after_cleanup) / max<uint64_t>(1, combine_recipe_calls) << " ingredients per combine_recipes call\n";
        out << "Maximum ingredients after cleanup: " << max_ingredients_after_cleanup << " ingredients\n";
        out << "Average products after cleanup: " << static_cast<double>(total_products_after_cleanup) / max<uint64_t>(1, combine_recipe_calls) << " products per combine_recipes call\n";
        out << "Maximum products after cleanup: " << max_products_after_cleanup << " products\n";

        /**************************************************/
        // Category 4: Chain Construction
        /**************************************************/

        out << "Chains generated: " << chains_generated << " (raw count)\n";
        out << "Recipes pushed to stack: " << recipes_pushed_to_stack<< " (raw count)\n";
        out << "Terminal resource hits: " << terminal_hits << " (raw count)\n";
        out << "Maximum chain depth: " << max_chain_depth << " recipes\n";
        out << "Average output recipes per chain: " << static_cast<double>(total_output_recipes) / max<uint64_t>(1, chains_generated) << " recipes per chain\n";
        out << "Maximum output recipes in a chain: " << max_output_recipes << " recipes\n";

        /**************************************************/
        // Category 5: Fraction / LCM Work
        /**************************************************/

        out << "LCM calls: " << lcm_calls << " (raw count)\n";
        out << "Fraction reductions: " << fraction_reductions << " (raw count)\n";
        out << "Maximum speed LCM: " << max_speed_lm << '\n';
        out << "Maximum item LCM: " << max_item_lm << '\n';
        out << "Total item_lm: " << total_item_lm << '\n';
        out << "Item_lm equal 1: " << item_lm_1 << '\n';
        out << "Item_lm between 2 and 10: " << item_lm_2_10 << '\n';
        out << "Item_lm between 11 and 100: " << item_lm_11_100 << '\n';
        out << "Item_lm greater than 100: " << item_lm_over_100 << '\n';

        /**************************************************/
        // Category 6: Map Activity
        /**************************************************/

        out << "recipe_map searches: " << recipe_map_searches << " (raw count)\n";
        out << "recipe_map lookups: " << recipe_map_lookups << " (raw count)\n";
        out << "recipe_map updates: " << recipe_map_updates << " (raw count)\n";
        out << "incrementor_map lookups: " << incrementor_map_lookups << " (raw count)\n";
        out << "terminal_map searches: " << terminal_map_searches << " (raw count)\n";
        out << "recipes map lookups: " << recipes_map_lookups << " (raw count)\n";

        /**************************************************/
        // Category 7: Machine Count Statistics
        /**************************************************/
        total_machine_filtered_count = machine_filtered_count + machine_filtered_200_count;

        out << "Maximum machine count observed: " << max_machine_count << " machines\n";
        out << "Machine accepted count: " << machine_accepted_count << " (raw count)\n";
        out << "Total machine filtered count: " << total_machine_filtered_count << " (raw count)\n";
        out << "Machine filtered count from 200 speed_lm filter: " << machine_filtered_200_count << " (raw count)\n";
        out << "Machine filtered count after 200 speed_lm filter: " << machine_filtered_count << " (raw count)\n";
        out << "Machine filter rate: " << (100.0 * total_machine_filtered_count / max<uint64_t>(1, total_machine_filtered_count + machine_accepted_count)) << "%\n";

        /**************************************************/
        // Category 8: Timing Breakdown
        /**************************************************/

        out << "Average chain generation time: " << chain_generation_time.count() / max<uint64_t>(1, chains_generated) << " seconds per chain\n";
        out << "Average merge time: " << merge_time.count() / max<uint64_t>(1, chains_generated) << " seconds per chain\n";
        out << "Average combine_recipes time: " << combine_time.count() / max<uint64_t>(1, combine_recipe_calls) << " seconds per combine_recipes call\n";
        out << "Average set_primary_product time: " << correct_time.count() / max<uint64_t>(1, correct_recipe_calls) << " seconds per set_primary_product call\n";
        out << "Average cleanup time: " << clean_time.count() / max<uint64_t>(1, combine_recipe_calls) << " seconds per combine_recipes call\n";
        out << "Average LCM time: " << lcm_time.count() / max<uint64_t>(1, chains_generated) << " seconds per chain\n";
        out << "Average total generation time: " << total_generation_time.count() / max<uint64_t>(1, chains_generated) << " seconds per chain\n";

        /**************************************************/
        // Category 9: Extra statistics
        /**************************************************/
        out << "Average Resource comparisons per chain: " << static_cast<double>(resource_same_name_calls) /  max<uint64_t>(1, chains_generated) << '\n';
        out << "Average Recipe comparisons per chain: " << static_cast<double>(recipe_same_name_calls) / max<uint64_t>(1, chains_generated) << '\n';
        out << "Average LCM calls per chain: " << static_cast<double>(lcm_calls) / max<uint64_t>(1, chains_generated) << '\n';
        out << "Average recipes pushed per chain: " << static_cast<double>(recipes_pushed_to_stack) / max<uint64_t>(1, chains_generated) << '\n';
    }
}

#endif