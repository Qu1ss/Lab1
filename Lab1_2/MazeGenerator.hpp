#pragma once
// Генерация идеального лабиринта: поиск в глубину с возвратом (recursive backtracker),
// реализован через явный стек (без рекурсии, чтобы не переполнить стек вызовов).
//
// Почему этот алгоритм: он обходит ВСЕ клетки и каждую новую клетку соединяет ровно
// одним проходом с уже посещённой - получается остовное дерево клеток.
// Значит лабиринт связный и между любыми двумя клетками ровно один простой путь.
//
// Алгоритм работает только через публичный интерфейс Maze и не смотрит на содержимое клеток.

#include <vector>
#include <random>
#include "Maze.hpp"

template <typename T>
void generatePerfectMaze(Maze<T>& maze, unsigned seed, const Position& start = Position(0, 0)) {
    if (maze.passageCount() != 0) {
        throw std::invalid_argument("Генерация применима только к лабиринту без проходов");
    }
    std::mt19937 rng(seed);
    std::vector<bool> visited(maze.cellCount(), false);
    std::vector<Position> stack;

    visited[maze.index(start)] = true;
    stack.push_back(start);

    while (!stack.empty()) {
        Position current = stack.back();

        // Непосещённые соседи текущей клетки
        std::vector<Position> candidates;
        std::vector<Position> all = maze.neighbors(current);
        for (size_t i = 0; i < all.size(); ++i) {
            if (!visited[maze.index(all[i])]) candidates.push_back(all[i]);
        }

        if (candidates.empty()) {
            stack.pop_back(); // тупик - возвращаемся
        } else {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(candidates.size()) - 1);
            Position next = candidates[dist(rng)];
            maze.openPassage(current, next);
            visited[maze.index(next)] = true;
            stack.push_back(next);
        }
    }
}
