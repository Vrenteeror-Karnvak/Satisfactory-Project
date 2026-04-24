#ifndef FRACTION_CC
#define FRACTION_CC

#include "fraction.h"

using namespace std;
using json = nlohmann::ordered_json;

/**
 * @brief Default constructor.
 *
 * Initializes the fraction to 0/1.
 */
Fraction::Fraction() {
    numerator = 0;
    denominator = 1;
}

/**
 * @brief Construct a fraction with a numerator and denominator.
 *
 * @param n The numerator.
 * @param d The denominator (the default is 1).
 */
Fraction::Fraction(const int n, const int d) {
    numerator = n;
    denominator = d;
}

Fraction::Fraction(const double value) {
    numerator = value * 1000000;
    denominator = 1000000;
    *this *= 1;
}

/**
 * @brief Set the numerator.
 *
 * @param n The new numerator.
 */
void Fraction::set_numerator(const int n) {
    numerator = n;
}

/**
 * @brief Set the denominator.
 *
 * @param d The new denominator.
 */
void Fraction::set_denominator(const int d) {
    denominator = d;
}

/**
 * @brief Get the numerator.
 *
 * @return The current numerator.
 */
int Fraction::get_numerator() const {
    return numerator;
}

/**
 * @brief Get the denominator.
 *
 * @return The current denominator.
 */
int Fraction::get_denominator() const {
    return denominator;
}

/**************************************************/
// Operator Overloads
/**************************************************/

/**
 * @brief Compare two fractions for equality.
 *
 * @param other The fraction to compare with.
 * @return true if both fractions represent the same value.
 */
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

/**
 * @brief Compare two fractions for inequality.
 *
 * @param other The fraction to compare with.
 * @return true if the fractions are not equal.
 */
bool Fraction::operator!=(const Fraction& other) const {
    return !(*this == other);
}

/**
 * @brief Compare if this fraction is less than another.
 *
 * @param other The fraction to compare with.
 * @return true if this fraction is smaller.
 */
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

/**
 * @brief Compare if this fraction is less than or equal to another.
 *
 * @param other The fraction to compare with.
 * @return true if this fraction is smaller or equal.
 */
bool Fraction::operator<=(const Fraction& other) const {
    return (*this < other || *this == other);
}

/**
 * @brief Compare if this fraction is greater than another.
 *
 * @param other The fraction to compare with.
 * @return true if this fraction is larger.
 */
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

/**
 * @brief Compare if this fraction is greater than or equal to another.
 *
 * @param other The fraction to compare with.
 * @return true if this fraction is larger or equal.
 */
bool Fraction::operator>=(const Fraction& other) const {
    return (*this > other || *this == other);
}

/**
 * @brief Add another fraction to this fraction.
 *
 * @param other The fraction to add.
 * @return A reference to this fraction after addition.
 */
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

/**
 * @brief Return the sum of two fractions.
 *
 * @param other The fraction to add.
 * @return A new Fraction representing the sum.
 */
Fraction Fraction::operator+(const Fraction& other) const {
    Fraction result = *this;
    result += other;
    return result;
}

/**
 * @brief Subtract another fraction from this fraction.
 *
 * @param other The fraction to subtract.
 * @return A reference to this fraction after subtraction.
 */
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

/**
 * @brief Return the difference between two fractions.
 *
 * @param other The fraction to subtract.
 * @return A new Fraction representing the difference.
 */
Fraction Fraction::operator-(const Fraction& other) const {
    Fraction result = *this;
    result -= other;
    return result;
}

/**
 * @brief Multiply this fraction by another.
 *
 * @param other The fraction to multiply by.
 * @return A reference to this fraction after multiplication.
 */
Fraction& Fraction::operator*=(const Fraction& other) {
    int gd = 1; // the greatest common divisor

    numerator *= other.get_numerator();
    denominator *= other.get_denominator();

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

/**
 * @brief Return the product of two fractions.
 *
 * @param other The fraction to multiply by.
 * @return A new Fraction representing the product.
 */
Fraction Fraction::operator*(const Fraction& other) const {
    Fraction result = *this;
    result *= other;
    return result;
}

/**
 * @brief Divide this fraction by another.
 *
 * @param other The fraction to divide by.
 * @return A reference to this fraction after division.
 */
Fraction& Fraction::operator/=(const Fraction& other) {
    int gd = 1; // the greatest common divisor

    numerator *= other.get_denominator();
    denominator *= other.get_numerator();

    gd = gcd(numerator, denominator);
    numerator /= gd;
    denominator /= gd;

    return *this;
}

/**
 * @brief Return the quotient of two fractions.
 *
 * @param other The fraction to divide by.
 * @return A new Fraction representing the quotient.
 */
Fraction Fraction::operator/(const Fraction& other) const {
    Fraction result = *this;
    result /= other;
    return result;
}

#endif