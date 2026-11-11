#pragma once
#include <sstream>
#include "Equation.hpp"

// a*x + b = 0, a != 0
class LinearEquation : public Equation {
public:
    LinearEquation(double a, double b) : a_(a), b_(b) {
        detail::requireFinite(a, "a");
        detail::requireFinite(b, "b");
        if (a == 0.0) {
            throw std::invalid_argument("LinearEquation: коэффициент a не может быть равен 0");
        }
    }

    Complex operator()(const Complex& x) const override {
        return Complex(a_) * x + Complex(b_);
    }

    std::vector<Complex> solve() const override {
        return { Complex(-b_ / a_) };
    }

    std::string toString() const override {
        std::ostringstream os;
        os << a_ << "*x + " << b_ << " = 0";
        return os.str();
    }

private:
    double a_;
    double b_;
};
