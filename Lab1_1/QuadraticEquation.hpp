#pragma once
#include <sstream>
#include "Equation.hpp"

// a*x^2 + b*x + c = 0, a != 0
class QuadraticEquation : public Equation {
public:
    QuadraticEquation(double a, double b, double c) : a_(a), b_(b), c_(c) {
        detail::requireFinite(a, "a");
        detail::requireFinite(b, "b");
        detail::requireFinite(c, "c");
        if (a == 0.0) {
            throw std::invalid_argument("QuadraticEquation: коэффициент a не может быть равен 0");
        }
    }

    // Схема Горнера: (a*x + b)*x + c
    Complex operator()(const Complex& x) const override {
        return (Complex(a_) * x + Complex(b_)) * x + Complex(c_);
    }

    std::vector<Complex> solve() const override {
        const double d = b_ * b_ - 4.0 * a_ * c_;
        const Complex twoA(2.0 * a_);
        if (std::fabs(d) < Complex::EPS) {
            return { Complex(-b_) / twoA };                // один (кратный) корень
        }
        const Complex sq = Complex(d).sqrt();              // для d < 0 даёт чисто мнимое число
        return { (Complex(-b_) + sq) / twoA,
                 (Complex(-b_) - sq) / twoA };
    }

    std::string toString() const override {
        std::ostringstream os;
        os << a_ << "*x^2 + " << b_ << "*x + " << c_ << " = 0";
        return os.str();
    }

private:
    double a_;
    double b_;
    double c_;
};
