/**
 * @brief - description of process
 * 
 * @param input
 * 
 * @return output
 * 
 * Note that any field preceeded by dat is the final processed form,
 * while any preceeded by int is an intermediate form.
 */

/**
 * @brief extracts desired recipe information from raw satisfactory recipe list
 *
 * @param dat/en-US.json - the raw satisfactory recipe list
 *      located at <satisfactory game folder>
 * 
 * @return dat/name_pairs.json - [{internal class name, Display name}]
 * 
 * @return dat/terminal_resources.json - [{Display Name}] of items with no ingredients
 * 
 * @return int/recipe_raw.json -
 *      [{
 *      Display Name
 *          //(dat)
 * 
 *      Ingredients : [{internal class name of ingredient, amount needed}, ...]
 *          //{
 *              (int) will become Display Name,
 *              (int) will become ipm
 *             }
 * 
 *      Product : [{internal class name of product, amount produced}, ...]
 *          //{
 *              (int) will become Display Name,
 *              (int) will become ipm
 *             }
 * 
 *      ProducedIn
 *          //(int) in internal class form
 * 
 *      ManufacturingDuration
 *          //(int) used to calculate ipm
 *      }]
 * 
 * Note that items per minute (ipm) can be calculated using stoichiometry on
 * c0*a + c1*b + ... -> C0*A + C1*B + ...
 * where a,b,... are ingredients,
 *      A,b,... are products
 *      c0,c1,... and C0,C1,... are the amounts needed/produced
 * then multiply by 60 (s/min) / ManufacturingDuration (second per production)
 */
Recipe_Extractor (dat/en-US.json)
    -> dat/name_pairs.json
    -> dat/terminal_resources.json
    -> int/recipe_raw.json

/**
 * @brief replaces all internal class names with Display Names
 * 
 * @param dat/name_pairs.json - [{internal class name, Display Name}]
 * 
 * @param int/recipe_raw.json - [{Display Name, Ingredients, Product, ProducedIn, Manufacturing Duration}]
 * 
 * @return int/recipes_fixed.json -
 *      [{
 *      Display Name
 *          //(dat)
 *      
 *      Ingredients : [{Item Class, Amount}]
 *          //{
 *              (dat) in Display Name form
 *              (int) will become ipm
 *             }
 * 
 *      Product : [{Item Class, Amount}]
 *          //{
 *              (dat) in Display Name form
 *              (int) will become ipm
 *             }
 * 
 *      ProducedIn
 *          //(dat) in Display Name form
 * 
 *      Manufacturing Duration
 *          //(int) used to calculate ipm
 * 
 * @return int/original_recipe_names.txt - Display Names of fixed recipes
 */
Recipe_Fixer (dat/name_pairs.json, int/recipe_raw.json)
    -> int/recipes_fixed.json
    -> int/original_recipe_names.txt


/**
 * @brief filters fixed recipes and adds certain dummy recipes
 * 
 * @param int/recipes_fixed.json - recipe list containing no internal class names
 * 
 * @return int/recipes_trimmed.json - filtered recipes
 *              //(int) only Amount and ManufacturingDuration
 * 
 * @return int/recipes_removed.json - removed recipes
 *              //(int) Amount and ManufacturingDuration
 *              //will be discarded
 * 
 * @return int/consumable_recipes.json - consumable recipes
 *              //(int) only Amount and ManufacturingDuration
 *              //will be discarded
 * 
 * @return int/recipe_names.txt - Display Names of filtered recipes
 *              //will be discarded
 */
Recipe_Trimmer (int/recipes_fixed.json)
    -> int/recipes_trimmed.json
    -> int/recipes_removed.json
    -> int/consumable_recipes.json
    -> int/recipe_names.txt

/**
 * @brief categorizes recipes by primary product
 * 
 * @param dat/name_pairs.json - [{internal class name, Display name}]

 * @param int/recipes_trimmed.json - filtered recipes
 * 
 * @return int/recipes_sorted.json - [{Display Name, [Product, Alternate Products, ...}]
 *                  //(int) Amount and ManufacturingDuration
 *                  //(int) recipes will be sorted by least dependencies to most
 */
Recipe_Sorter (dat/name_pairs.json, int/recipes_trimmed.json)
    -> int/recipes_sorted.json

/**
 * @brief Uses Kahn's Algorithm to identify nodes with the least dependencies.
 *  Does a combined sort of primary and alternate recipes
 *  Then outputs only primary recipes in their sorted order
 * 
 * @param int/recipes_sorted.json - recipes categorized by primary product
 * 
 * @return dat/recipes.json - recipes sorted topologically
 * 
 * @return int/item_list.txt - recipe categories not to analysis
 *                  //(int) for testing
 * 
 * @return int/item_analysis.txt - recipe categories to analysis
 *                  //(int) for testing queries
 */
Recipe_Sorter_Sorter (int/recipes_sorted.json)
    -> dat/recipes.json
    -> int/item_list.txt
    -> int/item_analysis.txt

/**
 * @brief Used to search recipe list for possible terminal resources
 * 
 * @param dat/recipes.json - formatted recipe list
 * 
 * @return int/byproducts.json - secondary products
 */
Initializer (dat/recipes.json)
    -> int/byproducts.json

/**
 * @brief Examines recipe chains
 * 
 * @param dat/recipes.json - formatted recipe list
 * 
 * @param dat/test_input.json - Recipe and Item input followed by options and filters
 *          [
 *              Recipe Object,                          //pulled from dat/recipes.json
 *              {
 *                  Category: Input Item,
 *                  ItemClass                           //Display Name
 *              },
 *              {
 *                  Category: Termination Data
 *                  max_loops,
 *                  max_time,
 *                  unit_of_time,
 *                  number_items_to_test,               //?
 *                  old_number_items_to_test            //?
 *              },
 *              {
 *                  Category: Status Update Data,
 *                  update_frequency,
 *                  unit_of_time
 *              },
 *              {
 *                  Category: Filter Data,
 *                  max_product,                        //Excludes recipe chains that produce more than this
 *                  remake_filters                      //?
 *              }
 *          ]
 * 
 * @param dat/terminal_resources.json - [{Display Name}] of items with no ingredients
 * 
 * @return dat/test_results.json - Recipe chains
 *          [{
 *              Category,               //primary product
 *              Data: [{                //recipes in chain
 *                  DisplayName,        //recipe name
 *                  ID,                 //?
 *                  Ingredients: [{
 *                      ItemClass,      //display form
 *                      Amount          //calculated whole number amount form
 *                  }, ...],
 *                  Product: [{
 *                      ItemClass,      //display form
 *                      Amount          //calculated whole number amount form
 *                  }, ...]
 *              }, ...]
 *          }, ...]
 * 
 * @return dat/test_status.log
 * 
 */
main (dat/recipes.json, dat/test_recipe.json, dat/terminal_resources.json)
    -> dat/test_results.json
    -> dat/test_status.log
