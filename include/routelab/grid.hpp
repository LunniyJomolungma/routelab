#pragma once
#include <algorithm>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace routelab {
struct Grid {
    std::vector<std::string> cells;
    int start = -1, goal = -1;
    int rows() const { return static_cast<int>(cells.size()); }
    int cols() const { return cells.empty() ? 0 : static_cast<int>(cells.front().size()); }
    int size() const { return rows() * cols(); }
    char at(int id) const { return cells.at(id / cols()).at(id % cols()); }
    int cost(int id) const { const char c = at(id); return c >= '1' && c <= '9' ? c - '0' : 1; }
    std::vector<int> neighbors(int id) const {
        std::vector<int> result;
        const int row = id / cols(), col = id % cols();
        for (const auto& offset : std::vector<std::pair<int,int>>{{-1,0},{0,1},{1,0},{0,-1}}) {
            const int r = row + offset.first, c = col + offset.second;
            if (r >= 0 && c >= 0 && r < rows() && c < cols() && cells[r][c] != '#') result.push_back(r * cols() + c);
        }
        return result;
    }
};

inline Grid read_grid(std::istream& input) {
    Grid grid;
    std::string line;
    while (std::getline(input, line)) {
        if (grid.cells.empty() && line.compare(0,3,"\xEF\xBB\xBF") == 0) line.erase(0,3);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line.size() > 100 || grid.cells.size() >= 100) throw std::runtime_error("Map dimensions must be 1..100; empty rows are not allowed.");
        if (!grid.cells.empty() && line.size() != grid.cells.front().size()) throw std::runtime_error("Map rows must have equal width.");
        for (int c = 0; c < static_cast<int>(line.size()); ++c) {
            const char ch = line[c];
            if (ch != '#' && ch != '.' && ch != 'S' && ch != 'G' && !(ch >= '1' && ch <= '9')) throw std::runtime_error("Unknown map symbol on row " + std::to_string(grid.cells.size()+1));
            const int id = static_cast<int>(grid.cells.size() * line.size()) + c;
            if (ch == 'S') { if (grid.start != -1) throw std::runtime_error("Map has multiple starts."); grid.start = id; }
            if (ch == 'G') { if (grid.goal != -1) throw std::runtime_error("Map has multiple goals."); grid.goal = id; }
        }
        grid.cells.push_back(line);
    }
    if (input.bad()) throw std::runtime_error("Failed to read map.");
    if (grid.start < 0 || grid.goal < 0) throw std::runtime_error("Map needs exactly one S and one G.");
    return grid;
}
inline Grid load_grid(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) throw std::runtime_error("Cannot open map: " + filename);
    return read_grid(input);
}
inline Grid generate(int rows, int cols, unsigned seed) {
    if (rows < 3 || cols < 3 || rows > 100 || cols > 100) throw std::runtime_error("Generated dimensions must be 3..100.");
    std::mt19937 random(seed);
    Grid grid;
    grid.cells.assign(rows, std::string(cols,'.'));
    for (auto& row : grid.cells) for (char& cell : row) {
        const auto value = random() % 100;
        cell = value < 25 ? '#' : value < 48 ? static_cast<char>('2' + random()%8) : '.';
    }
    // Carve one guaranteed route while retaining random alternative routes.
    int r = 0, c = 0;
    grid.cells[r][c] = '.';
    while (r < rows-1 || c < cols-1) {
        if (r == rows-1 || (c < cols-1 && random()%2 == 0)) ++c; else ++r;
        grid.cells[r][c] = '.';
    }
    grid.start = 0; grid.goal = rows*cols-1;
    grid.cells[0][0] = 'S'; grid.cells.back().back() = 'G';
    return grid;
}
inline void save_grid(const Grid& grid, const std::string& filename) {
    std::ofstream output(filename);
    if (!output) throw std::runtime_error("Cannot write map: " + filename);
    for (const auto& row : grid.cells) output << row << '\n';
    output.flush();
    if (!output) throw std::runtime_error("Failed to save map.");
}
}
