#pragma once
// Структура лабиринта: клетки + проходы между ними.
// Здесь НЕТ алгоритмов генерации и поиска пути - только хранение и контракты.
//
// МОДЕЛЬ ВЛАДЕНИЯ:
//   Maze<T> единолично владеет всеми клетками - они лежат по значению в std::vector.
//   Указателей нет. Снаружи клетки адресуются координатами (Position), а не ссылками,
//   поэтому "висячих" указателей быть не может. Копирование лабиринта = глубокая копия.
//
// ИНВАРИАНТЫ:
//   1) размер не меньше 2x2;
//   2) проход существует только между клетками, имеющими общую сторону;
//   3) проход симметричен: если A->B открыт, то и B->A открыт;
//   4) выйти за границы лабиринта нельзя (любая такая попытка - исключение).
//   Все поля приватные, поэтому нарушить инварианты снаружи невозможно.

#include <vector>
#include <stdexcept>

struct Position {
    int row;
    int col;
    Position(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const Position& other) const { return row == other.row && col == other.col; }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

// Направления: 0 - вверх, 1 - вправо, 2 - вниз, 3 - влево.
// Противоположное направление = (d + 2) % 4.
inline int directionTo(const Position& from, const Position& to) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;
    if (dr == -1 && dc == 0) return 0;
    if (dr == 0 && dc == 1) return 1;
    if (dr == 1 && dc == 0) return 2;
    if (dr == 0 && dc == -1) return 3;
    return -1; // клетки не соседние
}

inline Position shifted(const Position& p, int direction) {
    switch (direction) {
        case 0: return Position(p.row - 1, p.col);
        case 1: return Position(p.row, p.col + 1);
        case 2: return Position(p.row + 1, p.col);
        default: return Position(p.row, p.col - 1);
    }
}

template <typename T>
class Maze {
public:
    Maze(int rows, int cols) : rows_(rows), cols_(cols), passageCount_(0) {
        if (rows < 2 || cols < 2) {
            throw std::invalid_argument("Размер лабиринта должен быть не меньше 2x2");
        }
        cells_.resize(static_cast<size_t>(rows) * cols);
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int cellCount() const { return rows_ * cols_; }
    int passageCount() const { return passageCount_; }

    bool inBounds(const Position& p) const {
        return p.row >= 0 && p.row < rows_ && p.col >= 0 && p.col < cols_;
    }

    // Линейный номер клетки (удобно для массивов visited/parent в алгоритмах).
    int index(const Position& p) const {
        if (!inBounds(p)) {
            throw std::out_of_range("Клетка вне границ лабиринта");
        }
        return p.row * cols_ + p.col;
    }

    Position positionOf(int index) const { return Position(index / cols_, index % cols_); }

    // Данные клетки (тип задаётся параметром шаблона).
    T& data(const Position& p) { return cells_[index(p)].value; }
    const T& data(const Position& p) const { return cells_[index(p)].value; }

    // Все соседи по общей стороне (внутри границ), независимо от наличия прохода.
    std::vector<Position> neighbors(const Position& p) const {
        index(p); // проверка границ
        std::vector<Position> result;
        for (int d = 0; d < 4; ++d) {
            Position n = shifted(p, d);
            if (inBounds(n)) result.push_back(n);
        }
        return result;
    }

    // Только те соседи, в которые из клетки есть проход.
    std::vector<Position> openNeighbors(const Position& p) const {
        const Cell& cell = cells_[index(p)];
        std::vector<Position> result;
        for (int d = 0; d < 4; ++d) {
            if (cell.open[d]) result.push_back(shifted(p, d));
        }
        return result;
    }

    void openPassage(const Position& a, const Position& b) {
        int ia = index(a);
        int ib = index(b);
        int d = directionTo(a, b);
        if (d == -1) {
            throw std::invalid_argument("Проход можно создать только между соседними клетками");
        }
        if (!cells_[ia].open[d]) {
            cells_[ia].open[d] = true;
            cells_[ib].open[(d + 2) % 4] = true;
            ++passageCount_;
        }
    }

    bool hasPassage(const Position& a, const Position& b) const {
        int ia = index(a);
        index(b);
        int d = directionTo(a, b);
        return d != -1 && cells_[ia].open[d];
    }

private:
    struct Cell {
        T value;
        bool open[4];
        Cell() : value() {
            for (int i = 0; i < 4; ++i) open[i] = false;
        }
    };

    int rows_;
    int cols_;
    int passageCount_;
    std::vector<Cell> cells_;
};
