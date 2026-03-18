#ifndef FRACTION_CC
#define FRACTION_CC

#include "fraction.h"

using namespace std;
using json = nlohmann::ordered_json;

Fraction::Fraction() {
    numerator = 0;
    denominator = 1;
}

Fraction::Fraction(const int n, const int d) {
    numerator = n;
    denominator = d;
}

void Fraction::set_numerator(const int n) {
    numerator = n;
}

void Fraction::set_denominator(const int d) {
    denominator = d;
}

int Fraction::get_numerator() const {
    return numerator;
}

int Fraction::get_denominator() const {
    return denominator;
}

/**************************************************/
// Operator Overloads
/**************************************************/

bool Fraction::operator==(const Fraction& other) const {
    int lm = 1; // the least common multiple
    bool result; // the result
    int temp_numerator = numerator;
    int temp_other_numerator = other.get_numerator();

    if (denominator == other.get_denominator()) {
        result = (numerator == other.get_numerator());
    }
    else {
        lm = lcm(denominator, other.get_denominator());
        temp_numerator *= (lm / denominator);
        temp_other_numerator *= (lm / other.get_denominator());
        result = (temp_numerator == temp_other_numerator);
    }

    return result;
}

bool Fraction::operator!=(const Fraction& other) const {
    return !(*this == other);
}

bool Fraction::operator<(const Fraction& other) const {
    int lm = 1; // the least common multiple
    bool result; // the result
    int temp_numerator = numerator;
    int temp_other_numerator = other.get_numerator();

    if (denominator == other.get_denominator()) {
        result = (numerator < other.get_numerator());
    }
    else {
        lm = lcm(denominator, other.get_denominator());
        temp_numerator *= (lm / denominator);
        temp_other_numerator *= (lm / other.get_denominator());
        result = (temp_numerator < temp_other_numerator);
    }

    return result;
}

bool Fraction::operator<=(const Fraction& other) const {
    return (*this < other || *this == other);
}

bool Fraction::operator>(const Fraction& other) const {
    int lm = 1; // the least common multiple
    bool result; // the result
    int temp_numerator = numerator;
    int temp_other_numerator = other.get_numerator();

    if (denominator == other.get_denominator()) {
        result = (numerator > other.get_numerator());
    }
    else {
        lm = lcm(denominator, other.get_denominator());
        temp_numerator *= (lm / denominator);
        temp_other_numerator *= (lm / other.get_denominator());
        result = (temp_numerator > temp_other_numerator);
    }

    return result;
}

bool Fraction::operator>=(const Fraction& other) const {
    return (*this > other || *this == other);
}

Fraction& Fraction::operator+=(const Fraction& other) {
    int lm = 1; // the least common multiple
    int gd = 1; // the greatest common divisor
    Fraction temp = other; // a temporary variable

    if (denominator == temp.get_denominator()) {
        numerator += temp.get_numerator();
    }
    else {
        lm = lcm(denominator, temp.get_denominator());
        numerator *= (lm / denominator);
        denominator = lm;
        temp.set_numerator(temp.get_numerator() * (lm / temp.get_denominator()));
        temp.set_denominator(lm);
        numerator += temp.get_numerator();
    }

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

Fraction Fraction::operator+(const Fraction& other) const {
    Fraction result = *this;
    result += other;
    return result;
}

Fraction& Fraction::operator-=(const Fraction& other) {
    int lm = 1; // the least common multiple
    int gd = 1; // the greatest common divisor
    Fraction temp = other; // a temporary variable

    if (denominator == temp.get_denominator()) {
        numerator -= temp.get_numerator();
    }
    else {
        lm = lcm(denominator, temp.get_denominator());
        numerator *= (lm / denominator);
        denominator = lm;
        temp.set_numerator(temp.get_numerator() * (lm / temp.get_denominator()));
        temp.set_denominator(lm);
        numerator -= temp.get_numerator();
    }

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

Fraction Fraction::operator-(const Fraction& other) const {
    Fraction result = *this;
    result -= other;
    return result;
}

Fraction& Fraction::operator*=(const Fraction& other) {
    int gd = 1; // the greatest common divisor

    numerator *= other.get_numerator();
    denominator *= other.get_denominator();

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

Fraction Fraction::operator*(const Fraction& other) const {
    Fraction result = *this;
    result *= other;
    return result;
}

Fraction& Fraction::operator/=(const Fraction& other) {
    int gd = 1; // the greatest common divisor

    numerator *= other.get_denominator();
    denominator *= other.get_numerator();

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

Fraction Fraction::operator/(const Fraction& other) const {
    Fraction result = *this;
    result /= other;
    return result;
}

#endif