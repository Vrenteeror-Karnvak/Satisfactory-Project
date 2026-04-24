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

/**
 * @brief Represents a rational number as a fraction.
 *
 * The Fraction class stores a numerator and denominator and supports
 * comparison and arithmetic operations using operator overloads.
 */
class Fraction {
    public:
        /**
         * @brief Default constructor.
         *
         * Initializes the fraction to 0/1.
         */
        Fraction();

        /**
         * @brief Construct a fraction with a numerator and denominator.
         *
         * @param n The numerator.
         * @param d The denominator (default is 1).
         */
        Fraction(const int n, const int d = 1);

        Fraction(const double value);

        /**
         * @brief Set the numerator.
         *
         * @param n The new numerator.
         */
        void set_numerator(const int n);

        /**
         * @brief Set the denominator.
         *
         * @param d The new denominator.
         */
        void set_denominator(const int d);

        /**
         * @brief Get the numerator.
         *
         * @return The current numerator.
         */
        int get_numerator() const;

        /**
         * @brief Get the denominator.
         *
         * @return The current denominator.
         */
        int get_denominator() const;

        /**************************************************/
        // Operator Overloads
        /**************************************************/

        /**
         * @brief Compare two fractions for equality.
         *
         * @param other The fraction to compare with.
         * @return true if both fractions represent the same value.
         */
        bool operator==(const Fraction& other) const;

        /**
         * @brief Compare two fractions for inequality.
         *
         * @param other The fraction to compare with.
         * @return true if the fractions are not equal.
         */
        bool operator!=(const Fraction& other) const;

        /**
         * @brief Compare if this fraction is less than another.
         *
         * @param other The fraction to compare with.
         * @return true if this fraction is smaller.
         */
        bool operator<(const Fraction& other) const;

        /**
         * @brief Compare if this fraction is less than or equal to another.
         *
         * @param other The fraction to compare with.
         * @return true if this fraction is smaller or equal.
         */
        bool operator<=(const Fraction& other) const;

        /**
         * @brief Compare if this fraction is greater than another.
         *
         * @param other The fraction to compare with.
         * @return true if this fraction is larger.
         */
        bool operator>(const Fraction& other) const;

        /**
         * @brief Compare if this fraction is greater than or equal to another.
         *
         * @param other The fraction to compare with.
         * @return true if this fraction is larger or equal.
         */
        bool operator>=(const Fraction& other) const;

        /**
         * @brief Add another fraction to this fraction.
         *
         * @param other The fraction to add.
         * @return A reference to this fraction after addition.
         */
        Fraction& operator+=(const Fraction& other);

        /**
         * @brief Return the sum of two fractions.
         *
         * @param other The fraction to add.
         * @return A new Fraction representing the sum.
         */
        Fraction operator+(const Fraction& other) const;

        /**
         * @brief Subtract another fraction from this fraction.
         *
         * @param other The fraction to subtract.
         * @return A reference to this fraction after subtraction.
         */
        Fraction& operator-=(const Fraction& other);

        /**
         * @brief Return the difference between two fractions.
         *
         * @param other The fraction to subtract.
         * @return A new Fraction representing the difference.
         */
        Fraction operator-(const Fraction& other) const;

        /**
         * @brief Multiply this fraction by another.
         *
         * @param other The fraction to multiply by.
         * @return A reference to this fraction after multiplication.
         */
        Fraction& operator*=(const Fraction& other);

        /**
         * @brief Return the product of two fractions.
         *
         * @param other The fraction to multiply by.
         * @return A new Fraction representing the product.
         */
        Fraction operator*(const Fraction& other) const;

        /**
         * @brief Divide this fraction by another.
         *
         * @param other The fraction to divide by.
         * @return A reference to this fraction after division.
         */
        Fraction& operator/=(const Fraction& other);

        /**
         * @brief Return the quotient of two fractions.
         *
         * @param other The fraction to divide by.
         * @return A new Fraction representing the quotient.
         */
        Fraction operator/(const Fraction& other) const;

    private:
        int numerator;
        int denominator;

};

#endif