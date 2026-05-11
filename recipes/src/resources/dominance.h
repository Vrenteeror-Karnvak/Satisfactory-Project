#ifndef DOMINANCE_H
#define DOMINANCE_H

#include <string>
#include <vector>

#include "recipe.h"
#include "resource.h"
#include "fraction.h"

using namespace std;

struct DominanceKey {
    Fraction product_amount;
    vector<string> ingredient_names;

    bool operator>(const DominanceKey& other) const;
    bool operator<(const DominanceKey& other) const;
};

enum class Dominance {
    A_DOMINATES,
    B_DOMINATES,
    NEITHER
};

Dominance does_dominate(const Recipe& a, const Recipe& b);

#endif