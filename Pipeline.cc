/**
 * @brief extracts desired recipe information from raw satisfactory recipe list
 *
 * @param dat/en-US.json - the raw satisfactory recipe list
 *      located at <satisfactory game folder>
 * 
 * @return dat/name_pairs.json - internal class name, display name
 * 
 * @return dat/terminal_resources.json - display names of items with no ingredients
 * 
 * @return int/recipe_raw.json
 * 
 *      Display Name
 *      Ingredients : [{internal class name of ingredient, amount needed}, ...]
 *      Product : [{internal class name of product, amount produced}, ...]
 *      ProducedIn
 *      ManufacturingDuration
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
 * @brief 
 * 
 * @param dat/name_pairs.json - internal class name, display name
 * 
 * @param int/recipe_raw.json
 * 
 * @return 
 */
Recipe_Fixer_A (dat/name_pairs.json, int/recipe_raw.json)
    -> int/recipes_fixed.json
    
/**
 * @brief 
 * 
 * @param dat/name_pairs.json - internal class name, display name
 */
Recipe_Fixer_B (dat/name_pairs.json)
    -> int/original_recipe_names.txt


recipe_trimmer
Recipe_Trimmer : int/recipes_fixed.json
    -> int/recipes_trimmed.json
    -> int/recipes_removed.json
    -> int/consumable_recipes.json
    -> int/recipe_names.txt


recipe_sorter
Recipe_Sorter : dat/name_pairs.json, int/recipes_trimmed.json
    -> int/recipes_sorted.json


recipe_sorter_sorter
Recipe_Sorter_Sorter : int/recipes_sorted.json
    -> dat/recipes.json
    -> int/item_list.txt
    -> int/item_analysis.txt
