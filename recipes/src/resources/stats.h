#ifndef STATS_H
#define STATS_H

#include <cstdint>
#include <ostream>
#include <chrono>

using namespace std;

namespace Stats {
    /*
    ==================================================
    CATEGORY 1: COMPARISON COUNTS
    ==================================================
    Purpose:
    Measure how often comparisons occur.
    Useful for determining whether string comparisons
    are becoming a bottleneck.

    Update Location:
    Resource::same_name()
    Recipe::same_name()
    */

    extern uint64_t resource_same_name_calls; // Added
    extern uint64_t recipe_same_name_calls; // Added



    /*
    ==================================================
    CATEGORY 2: RECIPE MERGING
    ==================================================
    Purpose:
    Measure how much merge work is occurring and
    whether it is actually accomplishing anything.

    Update Location:
    Recipe::combine_recipes()
    Recipe::merge_recipes()
    */

    extern uint64_t combine_recipe_calls; // Added
    extern uint64_t merge_recipe_calls; // Added
    extern uint64_t correct_recipe_calls; // Added

    // Resource matched existing resource
    extern uint64_t ingredients_merged; // Added
    extern uint64_t products_merged; // Added

    // Resource cancelled out by opposite side
    extern uint64_t ingredients_cancelled; // Added
    extern uint64_t products_cancelled; // Added



    /*
    ==================================================
    CATEGORY 3: VECTOR SIZE STATISTICS
    ==================================================
    Purpose:
    Determine actual data sizes being processed.

    Update Location:
    Recipe::combine_recipes()
    Immediately before cleanup.
    Immediately after cleanup.
    */

    // Current totals
    extern uint64_t total_ingredients_before_cleanup; // Added
    extern uint64_t total_products_before_cleanup; // Added

    extern uint64_t total_ingredients_after_cleanup; // Added
    extern uint64_t total_products_after_cleanup; // Added

    // Samples for averages
    extern uint64_t ingredient_samples; // Added
    extern uint64_t product_samples; // Added

    // Maximums
    extern uint64_t max_ingredients_before_cleanup; // Added
    extern uint64_t max_products_before_cleanup; // Added

    extern uint64_t max_ingredients_after_cleanup; // Added
    extern uint64_t max_products_after_cleanup; // Added



    /*
    ==================================================
    CATEGORY 4: CHAIN CONSTRUCTION
    ==================================================
    Purpose:
    Understand how large recipe chains actually are.

    Update Location:
    Main generation loop.
    Anywhere recipe_stack is modified.
    */

    extern uint64_t chains_generated; // Added

    extern uint64_t recipes_pushed_to_stack; // Added

    extern uint64_t terminal_hits; // Added

    // Maximum observed stack depth
    extern uint64_t max_chain_depth; // Added

    // output_recipes statistics
    extern uint64_t total_output_recipes; // Added
    extern uint64_t output_recipe_samples; // Added

    extern uint64_t max_output_recipes; // Added



    /*
    ==================================================
    CATEGORY 5: FRACTION / LCM WORK
    ==================================================
    Purpose:
    Determine whether fraction math is consuming
    significant runtime.

    Update Location:
    Fraction reduction function
    LCM function
    */

    // Counts
    extern uint64_t lcm_calls; // Added
    extern uint64_t fraction_reductions; // Added

    // Largest values observed
    extern uint64_t max_speed_lm; // Added
    extern uint64_t max_item_lm; // Added

    // item_lm stats
    extern uint64_t total_item_lm; // Added
    extern uint64_t item_lm_1; // Added
    extern uint64_t item_lm_2_10; // Added
    extern uint64_t item_lm_11_100; // Added
    extern uint64_t item_lm_over_100; // Added



    /*
    ==================================================
    CATEGORY 6: MAP ACTIVITY
    ==================================================
    Purpose:
    Measure lookup frequency.

    Update Location:
    Immediately before each lookup/update.
    */

    extern uint64_t recipe_map_searches; // Added
    extern uint64_t recipe_map_lookups; // Added
    extern uint64_t recipe_map_updates; // Added

    extern uint64_t incrementor_map_lookups; // Added

    extern uint64_t terminal_map_searches; // Added

    extern uint64_t recipes_map_lookups; // Added



    /*
    ==================================================
    CATEGORY 7: MACHINE COUNT STATISTICS
    ==================================================
    Purpose:
    Understand effectiveness of machine filtering.

    Update Location:
    Immediately after number_of_machines
    is calculated.
    */

    // Maximum observed
    extern double max_machine_count; // Added

    // Filter statistics
    extern uint64_t total_machine_filtered_count; // Added
    extern uint64_t machine_filtered_200_count; // Added
    extern uint64_t machine_filtered_count; // Added
    extern uint64_t machine_accepted_count; // Added



    /*
    ==================================================
    CATEGORY 8: TIMING BREAKDOWN
    ==================================================
    Purpose:
    Identify expensive sections.

    Update Location:
    Around major code blocks.
    */
    extern chrono::duration<double> total_generation_time; // Added

    // Recipe chain generation
    extern chrono::duration<double> chain_generation_time; // Added

    // merge_recipes()
    extern chrono::duration<double> merge_time; // Added

    // combine_recipes()
    extern chrono::duration<double> combine_time; // Added

    // set_primary_product()
    extern chrono::duration<double> correct_time; // Added

    // cleanup portion of combine_recipes()
    extern chrono::duration<double> clean_time; // Added

    // LCM generation and scaling
    extern chrono::duration<double> lcm_time; // Added

    void print(std::ostream& out);
}

#endif