#pragma once
// Текстовое представление лабиринта (только ASCII, чтобы не зависеть от кодировки консоли).
// # - стена, S - старт, E - конец, * - найденный маршрут.
// Каждый символ печатается дважды по ширине, чтобы лабиринт не выглядел вытянутым.

#include <string>
#include <vector>
#include "Maze.hpp"

template <typename T>
std::string mazeToString(const Maze<T>& maze, const Position& start, const Position& end,
                         const std::vector<Position>& path = std::vector<Position>()) {
    maze.index(start);
    maze.index(end);

    int height = 2 * maze.rows() + 1;
    int width = 2 * maze.cols() + 1;
    std::vector<std::string> grid(height, std::string(width, '#'));

    for (int r = 0; r < maze.rows(); ++r) {
        for (int c = 0; c < maze.cols(); ++c) {
            Position p(r, c);
            grid[2 * r + 1][2 * c + 1] = ' ';
            if (c + 1 < maze.cols() && maze.hasPassage(p, Position(r, c + 1)))
                grid[2 * r + 1][2 * c + 2] = ' ';
            if (r + 1 < maze.rows() && maze.hasPassage(p, Position(r + 1, c)))
                grid[2 * r + 2][2 * c + 1] = ' ';
        }
    }

    for (size_t i = 0; i < path.size(); ++i) {
        grid[2 * path[i].row + 1][2 * path[i].col + 1] = '*';
        if (i > 0) { // клетка-проход между двумя соседними клетками пути
            grid[path[i].row + path[i - 1].row + 1][path[i].col + path[i - 1].col + 1] = '*';
        }
    }
    grid[2 * start.row + 1][2 * start.col + 1] = 'S';
    grid[2 * end.row + 1][2 * end.col + 1] = 'E';

    std::string result;
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            result += grid[r][c];
            result += grid[r][c];
        }
        result += '\n';
    }
    return result;
}
