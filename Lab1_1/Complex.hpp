#pragma once
#include <cmath>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

// Иммутабельный класс комплексного числа: после создания состояние не меняется,
// все методы const, сеттеров нет, операции возвращают новые объекты.
class Complex {
public:
    // Точность сравнения вещественных чисел (== и != сравнивают с допуском).
    static constexpr double EPS = 1e-9;

    // Конструктор намеренно не explicit: позволяет писать Complex z = 2.0 и 2.0 * z.
    Complex(double re = 0.0, double im = 0.0) : re_(re), im_(im) {
        if (!std::isfinite(re) || !std::isfinite(im)) {
            throw std::invalid_argument("Complex: NaN и бесконечность недопустимы");
        }
    }

    double re() const { return re_; }
    double im() const { return im_; }

    Complex operator-() const { return Complex(-re_, -im_); }

    // Модуль и сопряжённое число
    double abs() const { return std::hypot(re_, im_); }
    Complex conj() const { return Complex(re_, -im_); }

    // Главное значение квадратного корня (re >= 0).
    Complex sqrt() const {
        const double r = abs();
        const double a = std::sqrt((r + re_) / 2.0);
        const double b = std::sqrt((r - re_) / 2.0);
        return Complex(a, im_ < 0.0 ? -b : b);
    }

    std::string toString() const {
        std::ostringstream os;
        os << re_ << (im_ < 0.0 ? " - " : " + ") << std::fabs(im_) << "i";
        return os.str();
    }

private:
    double re_;
    double im_;
};

inline Complex operator+(const Complex& l, const Complex& r) {
    return Complex(l.re() + r.re(), l.im() + r.im());
}

inline Complex operator-(const Complex& l, const Complex& r) {
    return Complex(l.re() - r.re(), l.im() - r.im());
}

inline Complex operator*(const Complex& l, const Complex& r) {
    return Complex(l.re() * r.re() - l.im() * r.im(),
                   l.re() * r.im() + l.im() * r.re());
}

inline Complex operator/(const Complex& l, const Complex& r) {
    const double den = r.re() * r.re() + r.im() * r.im();
    if (den == 0.0) {
        throw std::domain_error("Complex: деление на ноль");
    }
    return Complex((l.re() * r.re() + l.im() * r.im()) / den,
                   (l.im() * r.re() - l.re() * r.im()) / den);
}

inline bool operator==(const Complex& l, const Complex& r) {
    return std::fabs(l.re() - r.re()) < Complex::EPS &&
           std::fabs(l.im() - r.im()) < Complex::EPS;
}

inline bool operator!=(const Complex& l, const Complex& r) { return !(l == r); }

inline std::ostream& operator<<(std::ostream& os, const Complex& z) {
    return os << z.toString();
}
