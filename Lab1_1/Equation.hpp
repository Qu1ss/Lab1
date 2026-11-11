#pragma once
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>
#include "Complex.hpp"

namespace detail {
// Проверка коэффициента: он должен быть конечным числом.
inline void requireFinite(double v, const char* name) {
    if (!std::isfinite(v)) {
        throw std::invalid_argument(std::string("Коэффициент ") + name +
                                    " должен быть конечным числом");
    }
}
}  // namespace detail

// Общий абстрактный тип уравнения (контракт).
class Equation {
public:
    virtual ~Equation() = default;

    // Функтор: значение функции f(x) в комплексной точке x.
    virtual Complex operator()(const Complex& x) const = 0;

    // Различные корни уравнения (кратный корень возвращается один раз).
    virtual std::vector<Complex> solve() const = 0;

    virtual std::string toString() const = 0;

    // x — корень, если f(x) = 0. Используется именно operator(), а не формулы.
    bool isRoot(const Complex& x) const { return (*this)(x) == Complex(0.0, 0.0); }
};
