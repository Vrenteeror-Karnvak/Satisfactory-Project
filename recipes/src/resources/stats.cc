#ifndef STATS_CC
#define STATS_CC

#include "stats.h"

using namespace std;

namespace Stats {
    /*
    ==================================================
    CATEGORY 1: PIPELINE FUNNEL
    ==================================================
    Purpose:
    Track how many combinations survive each stage.
    This tells where work is being eliminated.
    */

    uint64_t combinations_processed = 0;

    uint64_t ID_filtered = 0;
    uint64_t survived_id_filter = 0;

    uint64_t speed_filtered = 0;
    uint64_t survived_speed_filter = 0;

    uint64_t merge_filtered = 0;
    uint64_t survived_merge_filter = 0;

    uint64_t machine_accepted_count = 0;



    /*
    ==================================================
    CATEGORY 2: CHAIN CONSTRUCTION
    ==================================================
    Purpose:
    Understand actual chain complexity.
    */

    uint64_t chains_generated = 0;
    uint64_t incrementor_rebuild_count = 0;
    uint64_t merge_ID_calls = 0;
    uint64_t current_ID_entries = 0;
    uint64_t total_ID_entries_used = 0;
    uint64_t max_ID_entries_used = 0;

    uint64_t recipes_pushed_to_stack = 0;

    uint64_t terminal_hits = 0;

    uint64_t total_output_recipes = 0;
    uint64_t output_recipe_samples = 0;

    uint64_t max_chain_depth = 0;
    uint64_t max_output_recipes = 0;



    /*
    ==================================================
    CATEGORY 3: RECIPE MERGING
    ==================================================
    Purpose:
    Measure merge effectiveness.
    */

    uint64_t merge_recipe_calls = 0;
    uint64_t combine_recipe_calls = 0;

    uint64_t ingredients_merged = 0;
    uint64_t products_merged = 0;

    uint64_t ingredients_cancelled = 0;
    uint64_t products_cancelled = 0;



    /*
    ==================================================
    CATEGORY 4: RECIPE SIZE STATISTICS
    ==================================================
    Purpose:
    Determine actual recipe sizes.
    */

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
    
    

    /*
    ==================================================
    CATEGORY 5: FRACTION WORK
    ==================================================
    Purpose:
    Measure cost of fraction arithmetic.
    */

    uint64_t lcm_calls = 0;
    uint64_t fraction_reductions = 0;

    uint64_t max_speed_lm = 0;
    


    /*
    ==================================================
    CATEGORY 6: MAP ACTIVITY
    ==================================================
    Purpose:
    Measure lookup pressure.
    */

    uint64_t recipe_map_searches = 0;
    uint64_t recipe_map_lookups = 0;
    uint64_t recipe_map_updates = 0;

    uint64_t incrementor_map_lookups = 0;

    uint64_t terminal_map_searches = 0;

    uint64_t recipes_map_lookups = 0;

    

    /*
    ==================================================
    CATEGORY 7: COMPARISONS
    ==================================================
    Purpose:
    Measure comparison workload.
    */

    uint64_t resource_same_name_calls = 0;
    uint64_t recipe_same_name_calls = 0;



    /*
    ==================================================
    CATEGORY 8: MACHINE COUNTS
    ==================================================
    Purpose:
    Understand machine scaling behavior.
    */

    double max_machine_count = 0;
    double total_machine_count = 0;
    uint64_t machine_count_samples = 0;



    /*
    ==================================================
    CATEGORY 9: TIMING
    ==================================================
    Purpose:
    Identify bottlenecks.
    */

    chrono::duration<double> total_generation_time;
    chrono::duration<double> count_time;
    chrono::duration<double> ID_total_time;
    chrono::duration<double> ID_prep_time;
    chrono::duration<double> ID_build_time;
    chrono::duration<double> chain_generation_time;
    chrono::duration<double> merge_time;
    chrono::duration<double> combine_time;
    chrono::duration<double> correct_time;
    chrono::duration<double> lcm_time;
    chrono::duration<double> incrementor_time;
    chrono::duration<double> incrementor_rebuild_time;
    chrono::duration<double> output_time;

    void print(std::ostream& out) {
        survived_id_filter = combinations_processed - ID_filtered;
        survived_speed_filter = survived_id_filter - speed_filtered;
        survived_merge_filter = survived_speed_filter - merge_filtered;
        machine_accepted_count = survived_merge_filter;

        out << "\n=== Profiling Statistics ===\n";

        out << "=== Pipeline Summary ===\n";
        out << "Combinations processed: " << combinations_processed << " (raw count)\n";
        out << "Passed ID filter: " << survived_id_filter << " (" << 100.0 * survived_id_filter / combinations_processed << "%)\n";
        out << "Passed speed filter: " << survived_speed_filter << " (" << 100.0 * survived_speed_filter / combinations_processed << "%)\n";
        out << "Passed merge filter: " << survived_merge_filter << " (" << 100.0 * survived_merge_filter / combinations_processed << "%)\n";
        out << "Recipes output: " << machine_accepted_count << " (" << 100.0 * machine_accepted_count / combinations_processed << "%)\n";
        out << "\n";

        out << "=== Filter Statistics ===\n";
        out << "Filtered by ID conflict: " << ID_filtered << " (" << 100.0 * ID_filtered / combinations_processed << "% of combinations)\n";
        out << "Filtered by speed filter: " << speed_filtered << " (" << 100.0 * speed_filtered / survived_id_filter << "% of ID-valid combinations)\n";
        out << "Filtered after merge: " << merge_filtered << " (" << 100.0 * merge_filtered / survived_speed_filter << "% of speed-valid combinations)\n";
        out << "\n";

        /*
        out << "=== Chain Construction ===\n";
        out << "Chains generated: " << chains_generated << " (raw count)\n";
        out << "Incrementor rebuild count: " << incrementor_rebuild_count << " (" << (static_cast<double>(incrementor_rebuild_count) / combinations_processed) << "%)\n";
        out << "Recipes pushed to stack: " << recipes_pushed_to_stack << " (raw count)\n";
        out << "Average recipes pushed per chain: " << (static_cast<double>(recipes_pushed_to_stack) / chains_generated) << '\n';
        out << "Terminal resource hits: " << terminal_hits << " (raw count)\n";
        out << "Average terminal hits per chain: " << (static_cast<double>(terminal_hits) / chains_generated) << '\n';
        out << "Maximum chain depth: " << max_chain_depth << " recipes\n";
        out << "\n";
        */

        out << "=== Merge Statistics ===\n";
        out << "merge_ID calls: " << merge_ID_calls << " (raw count)\n";
        out << "Average ID entries: " << (static_cast<double>(total_ID_entries_used) / combinations_processed) << '\n';
        out << "Max ID entries: " << max_ID_entries_used << '\n';
        out << "Average ID merges per combination: " << (static_cast<double>(merge_ID_calls) / combinations_processed) << '\n';
        out << "\n";
        out << "merge_recipes calls: " << merge_recipe_calls << " (raw count)\n";
        out << "combine_recipes calls: " << combine_recipe_calls << " (raw count)\n";
        out << "Merge skip rate: " << 100.0 * speed_filtered / combinations_processed << "%\n";
        out << "\n";
        /*
        out << "Ingredients merged: " << ingredients_merged << '\n';
        out << "Products merged: " << products_merged << '\n';
        out << "Ingredients cancelled: " << ingredients_cancelled << '\n';
        out << "Products cancelled: " << products_cancelled << '\n';
        out << "\n";
        out << "Average ingredients merged per combine: " << static_cast<double>(ingredients_merged) / combine_recipe_calls << '\n';
        out << "Average products merged per combine: " << static_cast<double>(products_merged) / combine_recipe_calls << '\n';
        out << "\n";
        */

        /*
        out << "=== Recipe Size Statistics ===\n";
        out << "Average ingredients before cleanup: " << static_cast<double>(total_ingredients_before_cleanup) / survived_speed_filter << '\n';
        out << "Maximum ingredients before cleanup: " << max_ingredients_before_cleanup << '\n';
        out << "Average products before cleanup: " << static_cast<double>(total_products_before_cleanup) / survived_speed_filter << '\n';
        out << "Maximum products before cleanup: " << max_products_before_cleanup << '\n';
        out << '\n';
        out << "Average ingredients after cleanup: " << static_cast<double>(total_ingredients_after_cleanup) / survived_speed_filter << '\n';
        out << "Maximum ingredients after cleanup: " << max_ingredients_after_cleanup << '\n';
        out << "Average products after cleanup: " << static_cast<double>(total_products_after_cleanup) / survived_speed_filter << '\n';
        out << "Maximum products after cleanup: " << max_products_after_cleanup << '\n';
        out << '\n';
        */

        /*
        out << "=== Output Recipe Statistics ===\n";
        out << "Average output recipes per chain: " << static_cast<double>(total_output_recipes) / survived_id_filter << '\n';
        out << "Maximum output recipes in a chain: " << max_output_recipes << '\n';
        out << "\n";
        */

        out << "=== Fraction Statistics ===\n";
        /*
        out << "LCM calls: " << lcm_calls << " (raw count)\n";
        out << "Fraction reductions: " << fraction_reductions << " (raw count)\n";
        */
        out << "Average LCM calls per generated chain: " << static_cast<double>(lcm_calls / chains_generated) << '\n';
        out << "Average fraction reductions per generated chain: " << static_cast<double>(fraction_reductions / chains_generated) << '\n';
        out << "Maximum speed LCM: " << max_speed_lm << '\n';
        out << "\n";

        /*
        out << "=== Comparison Statistics ===\n";
        out << "Resource same_name calls: " << resource_same_name_calls << '\n';
        out << "Recipe same_name calls: " << recipe_same_name_calls << '\n';
        out << "Average Resource comparisons per generated chain: " << static_cast<double>(resource_same_name_calls / chains_generated) << '\n';
        out << "Average Recipe comparisons per generated chain: " << static_cast<double>(recipe_same_name_calls / chains_generated) << '\n';
        out << "\n";
        */

        out << "=== Machine Statistics ===\n";
        out << "Maximum machine count observed: " << max_machine_count << '\n';
        out << '\n';

        double total = total_generation_time.count();
        out << "=== Timing Breakdown ===\n";
        out << "Total Generation Time: " << total << " seconds.\n";
        out << "Count Check: " << 100.0 * count_time.count() / total << "% (" << count_time.count() / combinations_processed << " seconds per combination)\n";
        out << "ID Total: " << 100.0 * ID_total_time.count() / total << "% (" << ID_total_time.count() / combinations_processed << " seconds per combination)\n";
        out << "- Prep: " << 100.0 * ID_prep_time.count() / ID_total_time.count() << "% (" << ID_prep_time.count() / combinations_processed << " seconds per combination)\n";
        out << "- Build: " << 100.0 * ID_build_time.count() / ID_total_time.count() << "% (" << ID_build_time.count() / combinations_processed << " seconds per combination)\n";
        out << "Chain generation: " << 100.0 * chain_generation_time.count() / total << "% (" << chain_generation_time.count() / chains_generated << " seconds per chain)\n";
        out << "Merge: " << 100.0 * merge_time.count() / total << "% (" << merge_time.count() / chains_generated << " seconds per surviving chain)\n";
        out << "- Combine: " << 100.0 * combine_time.count() / merge_time.count() << "% (" << combine_time.count() / survived_speed_filter << " seconds per surviving chain)\n";
        out << "LCM: " << 100.0 * lcm_time.count() / total << "% (" << lcm_time.count() / chains_generated << " seconds per chain)\n";
        out << "Incrementor: " << 100.0 * incrementor_time.count() / total << "% (" << incrementor_time.count() / chains_generated << " seconds per combination)\n";
        out << "- Rebuild: " << 100.0 * incrementor_rebuild_time.count() / incrementor_time.count() << "% (" << incrementor_rebuild_time.count() / chains_generated << " seconds per combination)\n";
        out << "Output: " << 100.0 * output_time.count() / total << "% (" << output_time.count() / machine_accepted_count << " seconds per outputed chain)\n";
        out << "Total generation: " << total_generation_time.count() / chains_generated << " seconds per chain\n";
        out << "Total generation: " << total_generation_time.count() / combinations_processed << " seconds per combination\n";
        out << "\n";
    }
}

#endif