#ifndef DOMINANCE_CC
#define DOMINANCE_CC

#include "dominance.h"

using namespace std;

bool DominanceKey::operator>(const DominanceKey& other) const {
    if (product_amount != other.product_amount) {
        return (product_amount > other.product_amount);
    }

    return (ingredient_names > other.ingredient_names);
}

bool DominanceKey::operator<(const DominanceKey& other) const {
    if (product_amount != other.product_amount) {
        return (product_amount < other.product_amount);
    }

    return (ingredient_names < other.ingredient_names);
}

Dominance does_dominate(const Recipe& a, const Recipe& b) {
    bool a_better = false;
    bool b_better = false;

    // Compares each of the ingredients in A and B
    // The ingredients are equivalent and in the same order
    for (size_t i = 0; i < a.get_ingredients().size(); i++) {
        // Checks if the ingredient in A is less than the ingredient in B
        if (a.get_ingredients().at(i) < b.get_ingredients().at(i)) {
            // A is better if it is
            a_better = true;
        }
        // Checks if the ingredient in B is less than the ingredient in A
        else if (b.get_ingredients().at(i) < a.get_ingredients().at(i)) {
            // B is better if it is
            b_better = true;
        }

        // If both A and B are better for certain ingredients, neither can dominate
        if (a_better && b_better) {
            return Dominance::NEITHER;
        }
    }

    // Ingredient comparison resolution
    // If only A had better comparisons, A dominates
    if (a_better) {
        return Dominance::A_DOMINATES;
    }
    // If only B had better comparisons, A dominates
    else if (b_better) {
        return Dominance::B_DOMINATES;
    }

    
    // If every ingredient ties, use machine count instead
    // If A has a smaller number of machines, A dominates
    if (a.get_machine_speed() < b.get_machine_speed()) {
        return Dominance::A_DOMINATES;
    }
    // If B has a smaller number of machines, B dominates
    else if (a.get_machine_speed() > b.get_machine_speed()) {
        return Dominance::B_DOMINATES;
    }

    // If machine count ties as well, check for byproducts
    // If A has a byproduct and B does not, A dominates.
    if (a.get_products().size() > 1 && b.get_products().size() == 1) {
        return Dominance::A_DOMINATES;
    }
    // If B has a byproduct and A does not, B dominates.
    else if(a.get_products().size() == 1 && b.get_products().size() > 1) {
        return Dominance::B_DOMINATES;
    }
    // If both A and B have a byproduct, further comparison is needed
    else if(a.get_products().size() > 1 && b.get_products().size() > 1) {
        // If both A and B have a byproduct of water, a special condition applies
        if (a.get_product(1).get_name() == "Water" && b.get_product(1).get_name() == "Water") {
            // If A has a smaller byproduct, A dominates.
            if (a.get_product(1).get_amount() < b.get_product(1).get_amount()) {
                return Dominance::A_DOMINATES;
            }
            // If B has a smaller byproduct, B dominates.
            else if (a.get_product(1).get_amount() > b.get_product(1).get_amount()) {
                return Dominance::B_DOMINATES;
            }
        }
        // If only A has water as a byproduct, B dominates
        else if (a.get_product(1).get_name() == "Water") {
            return Dominance::B_DOMINATES;
        }
        // If only B has water as a byproduct, A dominates
        else if (b.get_product(1).get_name() == "Water") {
            return Dominance::A_DOMINATES;
        }
        // If no special conditions apply, the greater byproduct wins
        else {
            // If A has a larger byproduct, A dominates.
            if (a.get_product(1).get_amount() > b.get_product(1).get_amount()) {
                return Dominance::A_DOMINATES;
            }
            // If B has a larger byproduct, B dominates.
            else if (a.get_product(1).get_amount() < b.get_product(1).get_amount()) {
                return Dominance::B_DOMINATES;
            }
        }
    }

    // In a complete tie, A dominates
    return Dominance::A_DOMINATES;
}

#endif