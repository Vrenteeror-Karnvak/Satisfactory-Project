#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include <ostream>
#include <chrono>

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

    extern uint64_t combinations_processed; // Added

    extern uint64_t ID_filtered; // Added
    extern uint64_t survived_id_filter; // Added

    extern uint64_t speed_filtered; // Added
    extern uint64_t survived_speed_filter; // Added

    extern uint64_t merge_filtered; // Added
    extern uint64_t survived_merge_filter; // Added

    extern uint64_t machine_accepted_count; // Added



    /*
    ==================================================
    CATEGORY 2: CHAIN CONSTRUCTION
    ==================================================
    Purpose:
    Understand actual chain complexity.
    */

    extern uint64_t chains_generated; // Added
    extern uint64_t incrementor_rebuild_count; // Added
    extern uint64_t merge_ID_calls; // Added
    extern uint64_t current_ID_entries; // Added
    extern uint64_t total_ID_entries_used; // Added
    extern uint64_t max_ID_entries_used; // Added

    extern uint64_t recipes_pushed_to_stack; // Added

    extern uint64_t terminal_hits; // Added

    extern uint64_t total_output_recipes; // Added
    extern uint64_t output_recipe_samples; // Added

    extern uint64_t max_chain_depth; // Added
    extern uint64_t max_output_recipes; // Added



    /*
    ==================================================
    CATEGORY 3: RECIPE MERGING
    ==================================================
    Purpose:
    Measure merge effectiveness.
    */

    extern uint64_t merge_recipe_calls; // Added
    extern uint64_t combine_recipe_calls; // Added

    extern uint64_t ingredients_merged; // Added
    extern uint64_t products_merged; // Added

    extern uint64_t ingredients_cancelled; // Added
    extern uint64_t products_cancelled; // Added



    /*
    ==================================================
    CATEGORY 4: RECIPE SIZE STATISTICS
    ==================================================
    Purpose:
    Determine actual recipe sizes.
    */

    extern uint64_t total_ingredients_before_cleanup; // Added
    extern uint64_t total_products_before_cleanup; // Added

    extern uint64_t total_ingredients_after_cleanup; // Added
    extern uint64_t total_products_after_cleanup; // Added

    extern uint64_t ingredient_samples; // Added
    extern uint64_t product_samples; // Added

    extern uint64_t max_ingredients_before_cleanup; // Added
    extern uint64_t max_products_before_cleanup; // Added

    extern uint64_t max_ingredients_after_cleanup; // Added
    extern uint64_t max_products_after_cleanup; // Added
    
    

    /*
    ==================================================
    CATEGORY 5: FRACTION WORK
    ==================================================
    Purpose:
    Measure cost of fraction arithmetic.
    */

    extern uint64_t lcm_calls; // Added
    extern uint64_t fraction_reductions; // Added

    extern uint64_t max_speed_lm; // Added
    


    /*
    ==================================================
    CATEGORY 6: MAP ACTIVITY
    ==================================================
    Purpose:
    Measure lookup pressure.
    */

    extern uint64_t recipe_map_searches; // Added
    extern uint64_t recipe_map_lookups; // Added
    extern uint64_t recipe_map_updates; // Added

    extern uint64_t incrementor_map_lookups; // Added

    extern uint64_t terminal_map_searches; // Added

    extern uint64_t recipes_map_lookups; // Added

    

    /*
    ==================================================
    CATEGORY 7: COMPARISONS
    ==================================================
    Purpose:
    Measure comparison workload.
    */

    extern uint64_t resource_same_name_calls; // Added
    extern uint64_t recipe_same_name_calls; // Added



    /*
    ==================================================
    CATEGORY 8: MACHINE COUNTS
    ==================================================
    Purpose:
    Understand machine scaling behavior.
    */

    extern double max_machine_count; // Added
    extern double total_machine_count; // Added
    extern uint64_t machine_count_samples; // Added



    /*
    ==================================================
    CATEGORY 9: TIMING
    ==================================================
    Purpose:
    Identify bottlenecks.
    */

    extern chrono::duration<double> total_generation_time; // Added
    extern chrono::duration<double> count_time; // Added
    extern chrono::duration<double> ID_total_time; // Added
    extern chrono::duration<double> ID_prep_time; // Added
    extern chrono::duration<double> ID_build_time; // Added
    extern chrono::duration<double> chain_generation_time; // Added
    extern chrono::duration<double> merge_time; // Added
    extern chrono::duration<double> combine_time; // Added
    extern chrono::duration<double> correct_time; // Added
    extern chrono::duration<double> lcm_time; // Added
    extern chrono::duration<double> incrementor_time; // Added
    extern chrono::duration<double> incrementor_rebuild_time; // Added
    extern chrono::duration<double> output_time; // Added

    
    void print(std::ostream& out);
}

#endif