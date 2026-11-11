#include <iostream>
#include <memory>
#include <vector>
#include "Complex.hpp"
#include "EquationCollection.hpp"
#include "LinearEquation.hpp"
#include "QuadraticEquation.hpp"
#ifdef _WIN32
#include <windows.h>
#endif

// ---------- мини-фреймворк для тестов ----------
static int g_total = 0;
static int g_failed = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_total;                                                               \
        if (!(cond)) {                                                           \
            ++g_failed;                                                          \
            std::cout << "  [FAIL] " << #cond << " (строка " << __LINE__ << ")\n"; \
        }                                                                        \
    } while (0)

template <class Ex, class F>
bool throwsAs(F&& f) {
    try { f(); } catch (const Ex&) { return true; } catch (...) { return false; }
    return false;
}
#define CHECK_THROWS(Ex, ...) CHECK(throwsAs<Ex>([&] { __VA_ARGS__; }))

static bool contains(const std::vector<Complex>& v, const Complex& x) {
    for (const auto& e : v) if (e == x) return true;
    return false;
}

// ---------- тесты ----------
void testComplex() {
    std::cout << "Тесты Complex\n";
    const Complex a(1, 2), b(3, 4);

    CHECK(a + b == Complex(4, 6));
    CHECK(a - b == Complex(-2, -2));
    CHECK(a * b == Complex(-5, 10));
    CHECK(a / b == Complex(0.44, 0.08));
    CHECK(-a == Complex(-1, -2));
    CHECK(a.conj() == Complex(1, -2));
    CHECK(std::fabs(b.abs() - 5.0) < Complex::EPS);

    CHECK(a == Complex(1, 2));
    CHECK(a != b);
    CHECK(!(a != Complex(1, 2)));
    CHECK(Complex(1, 2) == Complex(1 + 1e-12, 2));      // допуск EPS
    CHECK(Complex(1, 2) != Complex(1.001, 2));

    // Иммутабельность: операции не меняют операнды
    const Complex c = a + b;
    CHECK(a == Complex(1, 2) && b == Complex(3, 4) && c == Complex(4, 6));

    // Неявное преобразование из double
    CHECK(2.0 * a == Complex(2, 4));
    CHECK(a + 1.0 == Complex(2, 2));

    // Деление на ноль
    CHECK_THROWS(std::domain_error, a / Complex(0, 0));
    CHECK_THROWS(std::domain_error, a / Complex());

    // Корректность создания
    CHECK_THROWS(std::invalid_argument, Complex(std::nan(""), 0));
    CHECK_THROWS(std::invalid_argument, Complex(0, INFINITY));

    // Квадратный корень
    CHECK(Complex(-1, 0).sqrt() == Complex(0, 1));
    CHECK(Complex(3, 4).sqrt() == Complex(2, 1));
    CHECK(Complex(4, 0).sqrt() == Complex(2, 0));
    CHECK(Complex(0, 0).sqrt() == Complex(0, 0));

    CHECK(Complex(1, 2).toString() == "1 + 2i");
    CHECK(Complex(1, -2).toString() == "1 - 2i");
}

void testLinear() {
    std::cout << "Тесты LinearEquation\n";
    CHECK_THROWS(std::invalid_argument, LinearEquation(0, 5));
    CHECK_THROWS(std::invalid_argument, LinearEquation(std::nan(""), 1));
    CHECK_THROWS(std::invalid_argument, LinearEquation(1, INFINITY));

    const LinearEquation eq(2, -4);                       // 2x - 4 = 0
    const auto roots = eq.solve();
    CHECK(roots.size() == 1);
    CHECK(roots[0] == Complex(2, 0));
    CHECK(eq(Complex(2, 0)) == Complex(0, 0));            // функтор
    CHECK(eq(Complex(0, 1)) == Complex(-4, 2));           // комплексный аргумент
    CHECK(eq.isRoot(Complex(2, 0)));
    CHECK(!eq.isRoot(Complex(0, 1)));

    const LinearEquation zeroB(3, 0);                     // b = 0 допустим
    CHECK(zeroB.solve()[0] == Complex(0, 0));
}

void testQuadratic() {
    std::cout << "Тесты QuadraticEquation\n";
    CHECK_THROWS(std::invalid_argument, QuadraticEquation(0, 1, 1));
    CHECK_THROWS(std::invalid_argument, QuadraticEquation(std::nan(""), 1, 1));
    CHECK_THROWS(std::invalid_argument, QuadraticEquation(1, INFINITY, 1));
    CHECK_THROWS(std::invalid_argument, QuadraticEquation(1, 1, INFINITY));

    {   // D > 0: x^2 - 5x + 6
        const QuadraticEquation eq(1, -5, 6);
        const auto r = eq.solve();
        CHECK(r.size() == 2);
        CHECK(contains(r, Complex(2, 0)) && contains(r, Complex(3, 0)));
        for (const auto& x : r) CHECK(eq.isRoot(x));
    }
    {   // D = 0: x^2 - 2x + 1
        const QuadraticEquation eq(1, -2, 1);
        const auto r = eq.solve();
        CHECK(r.size() == 1);
        CHECK(r[0] == Complex(1, 0));
        CHECK(eq.isRoot(r[0]));
    }
    {   // D < 0: x^2 + 1
        const QuadraticEquation eq(1, 0, 1);
        const auto r = eq.solve();
        CHECK(r.size() == 2);
        CHECK(contains(r, Complex(0, 1)) && contains(r, Complex(0, -1)));
        for (const auto& x : r) CHECK(eq.isRoot(x));
        CHECK(eq(Complex(1, 1)) == Complex(1, 2));        // (1+i)^2 + 1
    }
    {   // D < 0, комплексные корни: x^2 + 2x + 5 => -1 ± 2i
        const QuadraticEquation eq(1, 2, 5);
        const auto r = eq.solve();
        CHECK(contains(r, Complex(-1, 2)) && contains(r, Complex(-1, -2)));
        for (const auto& x : r) CHECK(eq.isRoot(x));
    }
    {   // нецелый a, отрицательный a
        const QuadraticEquation eq(-0.5, 3, -4);
        for (const auto& x : eq.solve()) CHECK(eq.isRoot(x));
    }
    {   // c = 0 и b = 0
        const QuadraticEquation eq(2, 0, 0);              // 2x^2 = 0
        CHECK(eq.solve().size() == 1 && eq.solve()[0] == Complex(0, 0));
    }
}

void testCollection() {
    std::cout << "Тесты EquationCollection\n";
    EquationCollection col;
    CHECK(col.empty());
    CHECK(col.size() == 0);
    CHECK(col.solveAll().empty());
    CHECK(col.evaluateAll(Complex(1, 0)).empty());
    CHECK(col.findWithRoot(Complex(1, 0)).empty());
    CHECK_THROWS(std::out_of_range, col.at(0));
    CHECK_THROWS(std::invalid_argument, col.add(nullptr));

    col.add(std::make_unique<LinearEquation>(2, -2));          // x = 1
    col.add(std::make_unique<QuadraticEquation>(1, 0, 1));     // x = ±i
    col.add(std::make_unique<QuadraticEquation>(1, -3, 2));    // x = 1, 2
    CHECK(col.size() == 3);
    CHECK(!col.empty());

    const auto all = col.solveAll();
    CHECK(all.size() == 3);
    CHECK(all[0].size() == 1 && all[1].size() == 2 && all[2].size() == 2);
    CHECK(contains(all[2], Complex(1, 0)) && contains(all[2], Complex(2, 0)));

    const auto vals = col.evaluateAll(Complex(1, 0));
    CHECK(vals.size() == 3);
    CHECK(vals[0] == Complex(0, 0));
    CHECK(vals[1] == Complex(2, 0));
    CHECK(vals[2] == Complex(0, 0));

    const auto with1 = col.findWithRoot(Complex(1, 0));
    CHECK(with1.size() == 2);
    CHECK(with1[0] == &col.at(0) && with1[1] == &col.at(2));

    const auto withI = col.findWithRoot(Complex(0, 1));
    CHECK(withI.size() == 1 && withI[0] == &col.at(1));

    CHECK(col.findWithRoot(Complex(100, 0)).empty());
}

// ---------- демонстрация ----------
void demo() {
    std::cout << "\n=== Демонстрация ===\n";
    EquationCollection col;
    col.add(std::make_unique<LinearEquation>(2, -4));
    col.add(std::make_unique<QuadraticEquation>(1, -5, 6));
    col.add(std::make_unique<QuadraticEquation>(1, 2, 5));

    const auto roots = col.solveAll();
    for (std::size_t i = 0; i < col.size(); ++i) {
        std::cout << col.at(i).toString() << "  =>  ";
        for (const auto& r : roots[i]) std::cout << "[" << r << "] ";
        std::cout << "\n";
    }
    const Complex x(2, 0);
    std::cout << "\nf(" << x << "): ";
    for (const auto& v : col.evaluateAll(x)) std::cout << "[" << v << "] ";
    std::cout << "\nУравнения с корнем " << x << ":\n";
    for (const auto* eq : col.findWithRoot(x)) std::cout << "  " << eq->toString() << "\n";
}

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   // вывод в UTF-8
    SetConsoleCP(CP_UTF8);         // ввод в UTF-8 (пригодится, если появится cin)
    #endif

    testComplex();
    testLinear();
    testQuadratic();
    testCollection();

    std::cout << "\nИтого проверок: " << g_total << ", провалено: " << g_failed << "\n";
    if (g_failed != 0) return 1;
    demo();
    return 0;
}
