#pragma once
// Поиск пути и проверки свойств лабиринта. Не зависит от генерации и от содержимого клеток.
//
// Изменяемое правило вынесено в параметр CanMove - любой вызываемый объект
// (лямбда, функтор, функция) вида bool(const Position& from, const Position& to).
// Он решает, можно ли сделать шаг from -> to (кроме наличия самого прохода).

#include <vector>
#include <queue>
#include <algorithm>
#include "Maze.hpp"

// Правило по умолчанию: любой шаг через проход разрешён.
struct AlwaysAllowed {
    bool operator()(const Position&, const Position&) const { return true; }
};

// Поиск в ширину: находит кратчайший (по числу шагов) путь.
// Возвращает клетки от from до to включительно; пустой вектор, если пути нет.
template <typename T, typename CanMove>
std::vector<Position> findPath(const Maze<T>& maze, const Position& from, const Position& to,
                               CanMove canMove) {
    int fromIndex = maze.index(from); // заодно проверка границ
    int toIndex = maze.index(to);

    std::vector<int> parent(maze.cellCount(), -1);
    std::vector<bool> visited(maze.cellCount(), false);
    std::queue<Position> queue;

    visited[fromIndex] = true;
    queue.push(from);

    while (!queue.empty() && !visited[toIndex]) {
        Position current = queue.front();
        queue.pop();
        std::vector<Position> next = maze.openNeighbors(current);
        for (size_t i = 0; i < next.size(); ++i) {
            int ni = maze.index(next[i]);
            if (visited[ni] || !canMove(current, next[i])) continue;
            visited[ni] = true;
            parent[ni] = maze.index(current);
            queue.push(next[i]);
        }
    }

    std::vector<Position> path;
    if (!visited[toIndex]) return path;

    // Восстановление пути от конца к началу по массиву parent
    for (int i = toIndex; i != -1; i = parent[i]) {
        path.push_back(maze.positionOf(i));
    }
    std::reverse(path.begin(), path.end());
    return path;
}

template <typename T>
std::vector<Position> findPath(const Maze<T>& maze, const Position& from, const Position& to) {
    return findPath(maze, from, to, AlwaysAllowed());
}

// Сколько клеток достижимо из start (включая её саму).
template <typename T>
int countReachable(const Maze<T>& maze, const Position& start) {
    std::vector<bool> visited(maze.cellCount(), false);
    std::queue<Position> queue;
    visited[maze.index(start)] = true;
    queue.push(start);
    int count = 1;
    while (!queue.empty()) {
        Position current = queue.front();
        queue.pop();
        std::vector<Position> next = maze.openNeighbors(current);
        for (size_t i = 0; i < next.size(); ++i) {
            int ni = maze.index(next[i]);
            if (!visited[ni]) {
                visited[ni] = true;
                ++count;
                queue.push(next[i]);
            }
        }
    }
    return count;
}

// Связность: из клетки (0,0) достижимы все клетки.
template <typename T>
bool isConnected(const Maze<T>& maze) {
    return countReachable(maze, Position(0, 0)) == maze.cellCount();
}

// Идеальный лабиринт = связный граф без циклов = дерево: проходов ровно (клетки - 1).
template <typename T>
bool isPerfect(const Maze<T>& maze) {
    return isConnected(maze) && maze.passageCount() == maze.cellCount() - 1;
}
