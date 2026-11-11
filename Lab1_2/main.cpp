// Лабораторная 1, задание 2, вариант 2: генерация и прохождение лабиринта.
// Файл должен быть сохранён в кодировке UTF-8.

#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "Maze.hpp"
#include "MazeGenerator.hpp"
#include "MazeSearch.hpp"
#include "MazePrinter.hpp"

// ---------- Мини-система тестов ----------
static int g_total = 0;
static int g_failed = 0;

#define CHECK(cond)                                                                   \
    do {                                                                              \
        ++g_total;                                                                    \
        if (!(cond)) {                                                                \
            ++g_failed;                                                               \
            std::cout << "  [ОШИБКА] " << #cond << " (строка " << __LINE__ << ")\n";  \
        }                                                                             \
    } while (0)

#define CHECK_THROWS(expr)                                                            \
    do {                                                                              \
        ++g_total;                                                                    \
        bool thrown = false;                                                          \
        try { expr; } catch (const std::exception&) { thrown = true; }               \
        if (!thrown) {                                                                \
            ++g_failed;                                                               \
            std::cout << "  [ОШИБКА] нет исключения: " << #expr                       \
                      << " (строка " << __LINE__ << ")\n";                            \
        }                                                                             \
    } while (0)

// ---------- Тесты структуры ----------
void testMazeStructure() {
    std::cout << "Тесты структуры лабиринта...\n";

    CHECK_THROWS(Maze<int> m(1, 5));
    CHECK_THROWS(Maze<int> m(5, 1));
    CHECK_THROWS(Maze<int> m(0, 0));
    CHECK_THROWS(Maze<int> m(-3, 4));

    Maze<int> maze(3, 4);
    CHECK(maze.rows() == 3);
    CHECK(maze.cols() == 4);
    CHECK(maze.cellCount() == 12);
    CHECK(maze.passageCount() == 0);

    // Соседи: угол - 2, край - 3, центр - 4
    CHECK(maze.neighbors(Position(0, 0)).size() == 2);
    CHECK(maze.neighbors(Position(0, 1)).size() == 3);
    CHECK(maze.neighbors(Position(1, 1)).size() == 4);
    CHECK_THROWS(maze.neighbors(Position(3, 0)));

    // Границы
    CHECK(maze.inBounds(Position(2, 3)));
    CHECK(!maze.inBounds(Position(3, 3)));
    CHECK(!maze.inBounds(Position(-1, 0)));
    CHECK_THROWS(maze.index(Position(0, 4)));

    // Проходы
    Position a(0, 0), b(0, 1), c(1, 1);
    CHECK(!maze.hasPassage(a, b));
    maze.openPassage(a, b);
    CHECK(maze.hasPassage(a, b));
    CHECK(maze.hasPassage(b, a)); // симметричность
    CHECK(maze.passageCount() == 1);
    maze.openPassage(b, a); // повторное открытие не увеличивает счётчик
    CHECK(maze.passageCount() == 1);
    CHECK(maze.openNeighbors(a).size() == 1);
    CHECK(maze.openNeighbors(b).size() == 1);

    CHECK_THROWS(maze.openPassage(a, c));                  // по диагонали
    CHECK_THROWS(maze.openPassage(a, Position(0, 2)));     // через клетку
    CHECK_THROWS(maze.openPassage(a, a));                  // сама с собой
    CHECK_THROWS(maze.openPassage(Position(0, 0), Position(-1, 0))); // за границу
    CHECK(!maze.hasPassage(a, c));

    // Данные клетки (параметр типа)
    maze.data(Position(2, 2)) = 42;
    CHECK(maze.data(Position(2, 2)) == 42);
    CHECK(maze.data(Position(0, 0)) == 0);

    Maze<std::string> named(2, 2);
    named.data(Position(1, 1)) = "сокровище";
    CHECK(named.data(Position(1, 1)) == "сокровище");

    // Минимальный размер допустим
    Maze<int> tiny(2, 2);
    CHECK(tiny.cellCount() == 4);
}

// ---------- Тесты генерации ----------
void testGeneration() {
    std::cout << "Тесты генерации...\n";

    int sizes[][2] = {{2, 2}, {2, 7}, {7, 2}, {5, 5}, {10, 20}, {30, 30}};
    for (int i = 0; i < 6; ++i) {
        for (unsigned seed = 1; seed <= 5; ++seed) {
            Maze<int> maze(sizes[i][0], sizes[i][1]);
            generatePerfectMaze(maze, seed);
            CHECK(isConnected(maze));
            CHECK(isPerfect(maze));
            CHECK(maze.passageCount() == maze.cellCount() - 1);
        }
    }

    // Одинаковый seed - одинаковый лабиринт
    Maze<int> m1(6, 6), m2(6, 6);
    generatePerfectMaze(m1, 123);
    generatePerfectMaze(m2, 123);
    CHECK(mazeToString(m1, Position(0, 0), Position(5, 5)) ==
          mazeToString(m2, Position(0, 0), Position(5, 5)));

    // Повторная генерация и неверный старт - ошибки
    CHECK_THROWS(generatePerfectMaze(m1, 5));
    Maze<int> m3(3, 3);
    CHECK_THROWS(generatePerfectMaze(m3, 5, Position(9, 9)));

    // Генерация не зависит от типа данных клетки
    Maze<std::string> strMaze(5, 5);
    generatePerfectMaze(strMaze, 7);
    CHECK(isPerfect(strMaze));
}

// ---------- Тесты свойств и поиска ----------
void testSearch() {
    std::cout << "Тесты поиска пути...\n";

    // Лабиринт без проходов не связный
    Maze<int> empty(3, 3);
    CHECK(!isConnected(empty));
    CHECK(!isPerfect(empty));
    CHECK(countReachable(empty, Position(0, 0)) == 1);
    CHECK(findPath(empty, Position(0, 0), Position(2, 2)).empty());

    // Лабиринт с циклом: связный, но не идеальный
    Maze<int> cyc(2, 2);
    cyc.openPassage(Position(0, 0), Position(0, 1));
    cyc.openPassage(Position(0, 1), Position(1, 1));
    cyc.openPassage(Position(1, 1), Position(1, 0));
    CHECK(isPerfect(cyc)); // 3 прохода на 4 клетки - дерево
    cyc.openPassage(Position(1, 0), Position(0, 0));
    CHECK(isConnected(cyc));
    CHECK(!isPerfect(cyc)); // 4 прохода - есть цикл

    // Путь в сгенерированном лабиринте
    Maze<int> maze(12, 15);
    generatePerfectMaze(maze, 2024);
    Position start(0, 0), end(11, 14);
    std::vector<Position> path = findPath(maze, start, end);
    CHECK(!path.empty());
    CHECK(path.front() == start);
    CHECK(path.back() == end);
    bool stepsOk = true;
    for (size_t i = 1; i < path.size(); ++i) {
        if (!maze.hasPassage(path[i - 1], path[i])) stepsOk = false;
    }
    CHECK(stepsOk);

    // Все клетки пути различны (путь простой)
    bool simple = true;
    for (size_t i = 0; i < path.size(); ++i)
        for (size_t j = i + 1; j < path.size(); ++j)
            if (path[i] == path[j]) simple = false;
    CHECK(simple);

    // Путь из клетки в саму себя
    std::vector<Position> self = findPath(maze, Position(3, 3), Position(3, 3));
    CHECK(self.size() == 1);

    // Путь симметричен по длине
    CHECK(findPath(maze, end, start).size() == path.size());

    // Неверные координаты
    CHECK_THROWS(findPath(maze, Position(-1, 0), end));
    CHECK_THROWS(findPath(maze, start, Position(12, 0)));

    // Изменяемое правило: запрещаем заходить в клетку (3,3) - если она на пути, пути не будет
    Position blocked = path[path.size() / 2];
    std::vector<Position> none = findPath(maze, start, end,
        [blocked](const Position&, const Position& to) { return to != blocked; });
    CHECK(none.empty()); // в идеальном лабиринте путь единственный

    // Правило, которое всё разрешает, даёт тот же результат, что и версия по умолчанию
    std::vector<Position> same = findPath(maze, start, end,
        [](const Position&, const Position&) { return true; });
    CHECK(same.size() == path.size());

    // Правило через данные клеток (поиск не знает, что в клетках int)
    Maze<int> data(4, 4);
    generatePerfectMaze(data, 9);
    data.data(Position(3, 3)) = 1; // "яма"
    std::vector<Position> toPit = findPath(data, Position(0, 0), Position(3, 3),
        [&data](const Position&, const Position& to) { return data.data(to) != 1; });
    CHECK(toPit.empty());
}

// ---------- Тесты вывода ----------
void testPrinter() {
    std::cout << "Тесты текстового представления...\n";

    Maze<int> maze(2, 2);
    maze.openPassage(Position(0, 0), Position(0, 1));
    maze.openPassage(Position(0, 1), Position(1, 1));
    std::vector<Position> path = findPath(maze, Position(0, 0), Position(1, 1));
    std::string text = mazeToString(maze, Position(0, 0), Position(1, 1), path);

    CHECK(text.find('S') != std::string::npos);
    CHECK(text.find('E') != std::string::npos);
    CHECK(text.find('*') != std::string::npos);
    int lines = 0;
    for (size_t i = 0; i < text.size(); ++i) if (text[i] == '\n') ++lines;
    CHECK(lines == 5); // 2 * 2 + 1
    CHECK_THROWS(mazeToString(maze, Position(5, 5), Position(0, 0)));
}

// ---------- Демонстрация ----------
void demo() {
    std::cout << "\n===== Демонстрация =====\n\n";

    // Лабиринт 1: 8 x 14 с маршрутом
    Maze<int> big(8, 14);
    generatePerfectMaze(big, 42);
    Position start(0, 0);
    Position end(big.rows() - 1, big.cols() - 1);
    std::vector<Position> path = findPath(big, start, end);

    std::cout << "Лабиринт 8x14 (S - старт, E - конец, * - маршрут):\n";
    std::cout << mazeToString(big, start, end, path);
    std::cout << "Длина маршрута: " << path.size() << " клеток\n";
    std::cout << "Лабиринт связный: " << (isConnected(big) ? "да" : "нет") << "\n";
    std::cout << "Лабиринт идеальный: " << (isPerfect(big) ? "да" : "нет") << "\n\n";

    // Лабиринт 2: другой размер и другой тип данных клеток
    Maze<std::string> small(4, 6);
    generatePerfectMaze(small, 7);
    std::cout << "Лабиринт 4x6 без маршрута:\n";
    std::cout << mazeToString(small, Position(0, 0), Position(3, 5));
    std::cout << "Проходов: " << small.passageCount()
              << " (клеток - 1 = " << small.cellCount() - 1 << ")\n\n";

    // Изменяемое правило поиска: запрет входить в одну клетку маршрута
    Position pit = path[path.size() / 2];
    std::vector<Position> blockedPath = findPath(big, start, end,
        [pit](const Position&, const Position& to) { return to != pit; });
    std::cout << "Поиск с запретом входа в клетку (" << pit.row << ", " << pit.col << "): "
              << (blockedPath.empty() ? "путь не найден" : "путь найден") << "\n";
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // корректный вывод русского языка в консоли Windows
#endif

    testMazeStructure();
    testGeneration();
    testSearch();
    testPrinter();

    std::cout << "\nВсего проверок: " << g_total << ", провалено: " << g_failed << "\n";
    if (g_failed != 0) {
        std::cout << "ТЕСТЫ НЕ ПРОЙДЕНЫ\n";
        return 1;
    }
    std::cout << "Все тесты пройдены\n";

    demo();
    return 0;
}
