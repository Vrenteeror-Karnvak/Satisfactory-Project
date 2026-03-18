#ifndef FRACTION_H
#define FRACTION_H

#include "../lib/json.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>

using namespace std;
using json = nlohmann::ordered_json;

class Fraction {
    public:
        Fraction();
        Fraction(const int n, const int d = 1);
        void set_numerator(const int n);
        void set_denominator(const int d);
        int get_numerator() const;
        int get_denominator() const;

        /**************************************************/
        // Operator Overloads
        /**************************************************/

        bool operator==(const Fraction& other) const;
        bool operator!=(const Fraction& other) const;
        bool operator<(const Fraction& other) const;
        bool operator<=(const Fraction& other) const;
        bool operator>(const Fraction& other) const;
        bool operator>=(const Fraction& other) const;
        Fraction& operator+=(const Fraction& other);
        Fraction operator+(const Fraction& other) const;
        Fraction& operator-=(const Fraction& other);
        Fraction operator-(const Fraction& other) const;
        Fraction& operator*=(const Fraction& other);
        Fraction operator*(const Fraction& other) const;
        Fraction& operator/=(const Fraction& other);
        Fraction operator/(const Fraction& other) const;

    private:
        int numerator;
        int denominator;

};

#endif