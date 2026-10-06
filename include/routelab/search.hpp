#pragma once
#include "grid.hpp"
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <tuple>

namespace routelab {
enum class Algorithm { BFS, Dijkstra, AStar };
inline std::string name(Algorithm algorithm) {
    return algorithm == Algorithm::BFS ? "BFS" : algorithm == Algorithm::Dijkstra ? "Dijkstra" : "A*";
}
struct Result {
    Algorithm algorithm;
    bool found = false;
    int cost = 0;
    std::vector<int> path, explored;
    long long microseconds = 0;
};
inline Result search(const Grid& grid, Algorithm algorithm) {
    const auto begin = std::chrono::steady_clock::now();
    Result result{algorithm, false, 0, {}, {}, 0};
    std::vector<int> parent(grid.size(), -1), distance(grid.size(), std::numeric_limits<int>::max());
    std::vector<bool> closed(grid.size(), false);
    // (estimated total cost, insertion order, node, actual search cost)
    using Entry = std::tuple<int,int,int,int>;
    std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> queue;
    int order = 0;
    const auto heuristic = [&grid,algorithm](int node) {
        if (algorithm != Algorithm::AStar) return 0;
        return std::abs(node / grid.cols() - grid.goal / grid.cols()) + std::abs(node % grid.cols() - grid.goal % grid.cols());
    };
    distance[grid.start] = 0;
    if (algorithm == Algorithm::BFS) {
        std::queue<int> frontier;
        frontier.push(grid.start);
        while (!frontier.empty()) {
            const int node = frontier.front(); frontier.pop();
            result.explored.push_back(node);
            if (node == grid.goal) { result.found = true; break; }
            for (int next : grid.neighbors(node)) if (distance[next] == std::numeric_limits<int>::max()) {
                distance[next] = distance[node] + 1;
                parent[next] = node;
                frontier.push(next);
            }
        }
    } else queue.emplace(heuristic(grid.start), order++, grid.start, 0);
    while (!queue.empty()) {
        const auto [priority, sequence, node, known] = queue.top();
        (void)priority; (void)sequence;
        queue.pop();
        if (closed[node] || known != distance[node]) continue;
        closed[node] = true;
        result.explored.push_back(node);
        if (node == grid.goal) { result.found = true; break; }
        for (int next : grid.neighbors(node)) {
            const int next_distance = known + grid.cost(next);
            if (!closed[next] && next_distance < distance[next]) {
                distance[next] = next_distance;
                parent[next] = node;
                queue.emplace(next_distance + heuristic(next), order++, next, next_distance);
            }
        }
    }
    if (result.found) {
        for (int node = grid.goal; node != -1; node = parent[node]) result.path.push_back(node);
        std::reverse(result.path.begin(), result.path.end());
        for (std::size_t i=1; i<result.path.size(); ++i) result.cost += grid.cost(result.path[i]);
    }
    result.microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-begin).count();
    return result;
}
}
