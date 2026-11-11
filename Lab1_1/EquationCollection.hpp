#pragma once
#include <memory>
#include <stdexcept>
#include <vector>
#include "Equation.hpp"

// Коллекция уравнений. Владеет ими (unique_ptr), работает только через тип Equation.
class EquationCollection {
public:
    void add(std::unique_ptr<Equation> eq) {
        if (!eq) {
            throw std::invalid_argument("EquationCollection: нельзя добавить пустое уравнение");
        }
        items_.push_back(std::move(eq));
    }

    std::size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }

    const Equation& at(std::size_t i) const {
        if (i >= items_.size()) throw std::out_of_range("EquationCollection: индекс вне диапазона");
        return *items_[i];
    }

    // Корни каждого уравнения (в порядке добавления).
    std::vector<std::vector<Complex>> solveAll() const {
        std::vector<std::vector<Complex>> result;
        result.reserve(items_.size());
        for (const auto& eq : items_) result.push_back(eq->solve());
        return result;
    }

    // Значения всех уравнений в точке x.
    std::vector<Complex> evaluateAll(const Complex& x) const {
        std::vector<Complex> result;
        result.reserve(items_.size());
        for (const auto& eq : items_) result.push_back((*eq)(x));
        return result;
    }

    // Уравнения, для которых f(x) = 0 (проверка через operator()).
    std::vector<const Equation*> findWithRoot(const Complex& x) const {
        std::vector<const Equation*> result;
        for (const auto& eq : items_) {
            if (eq->isRoot(x)) result.push_back(eq.get());
        }
        return result;
    }

private:
    std::vector<std::unique_ptr<Equation>> items_;
};
