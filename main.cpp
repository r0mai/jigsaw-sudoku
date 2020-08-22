#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <set>

struct Puzzle {
    Puzzle(int size = 9)
        : size(size)
        , numbers(size * size)
        , shape(size* size)
    {}

    int size = -1;
    std::vector<int> numbers;
    std::vector<int> shape;

    // each vector here is a set of indices (of size size)
    // where each number must occur once
    std::vector<std::vector<int>> blockGraph;

    void BuildBlockGraph() {
        std::unordered_map<int, int> shapeNameIdxMap;
        for (int i = 0; i < shape.size(); ++i) {
            int shapeName = shape[i];
            auto it = shapeNameIdxMap.find(shapeName);
            if (it == shapeNameIdxMap.end()) {
                it = shapeNameIdxMap.insert({shapeName, blockGraph.size()}).first;
                blockGraph.emplace_back();
            }
            int idx = it->second;
            blockGraph[idx].push_back(i);
        }

        for (int r = 0; r < size; ++r) {
            blockGraph.emplace_back();
            for (int c = 0; c < size; ++c) {
                blockGraph.back().push_back(IndexOf(r, c));
            }
        }

        for (int c = 0; c < size; ++c) {
            blockGraph.emplace_back();
            for (int r = 0; r < size; ++r) {
                blockGraph.back().push_back(IndexOf(r, c));
            }
        }

        // sanity check
        for (auto& g : blockGraph) {
            assert(g.size() == size);
        }
    }

    void PrintNumbers1D(std::ostream& os = std::cout, int idx = -1) const {
        int row = idx / size;
        int col = idx % size;

        PrintNumbers(os, row, col);
    }

    void PrintNumbers(std::ostream& os = std::cout, int hr = -1, int hc = -1) const {
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                if (r == hr && c == hc) {
                    os << "\e[31m";
                }
                os << NumberAt(r, c) << ' ';
                if (r == hr && c == hc) {
                    os << "\e[0m";
                }
            }
            os << '\n';
        }
    }

    void PrintShape(std::ostream& os = std::cout) const {
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                os << ShapeAt(r, c) << ' ';
            }
            os << '\n';
        }
    }

    int IndexOf(int r, int c) const {
        assert(r >= 0 && r < size);
        assert(c >= 0 && c < size);
        return r * size + c;
    }

    int& NumberAt(int r, int c) {
        return numbers[IndexOf(r, c)];
    }

    int NumberAt(int r, int c) const {
        return numbers[IndexOf(r, c)];
    }

    int ShapeAt(int r, int c) const {
        return shape[IndexOf(r, c)];
    }

    int& ShapeAt(int r, int c) {
        return shape[IndexOf(r, c)];
    }
};

struct Coord {
    Coord(int idx)
        : row(idx / 9) // TODO hardcoded
        , col(idx % 9) // TODO hardcoded
    {}

    int row = -1;
    int col = -1;
};

std::ostream& operator<<(std::ostream& os, const Coord& c) {
    os << '(' << c.row << ", " << c.col << ')';
    return os;
}

bool ParsePuzzle(std::istream& in, Puzzle& puzzle) {
    int size;
    if (!(std::cin >> size)) {
        std::cerr << "Failed to read size" << std::endl;
        return false;
    }

    if (size != 9) { //  todo perfect square
        std::cerr << "Only size 9 is supported" << std::endl;
        return false;
    }

    puzzle = Puzzle(size);

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            if (!(std::cin >> puzzle.NumberAt(r, c))) {
                std::cerr << "failed to read number" << std::endl;
                return false;
            }
        }
    }

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            if (!(std::cin >> puzzle.ShapeAt(r, c))) {
                std::cerr << "failed to read shape" << std::endl;
                return false;
            }
        }
    }

    puzzle.BuildBlockGraph();

    return true;
}

std::ostream& operator<<(std::ostream& os, const Puzzle& p) {
    p.PrintNumbers(os);
    os << '\n';
    p.PrintShape(os);
    os << '\n';

    for (auto& g : p.blockGraph) {
        for (int idx : g) {
            os << idx << ' ';
        }
        os << '\n';
    }
    return os;
}

struct Solver {
    Solver(Puzzle& puzzle) : puzzle(puzzle) {}
    Puzzle& puzzle;

    bool SolveNaive() {
        bool changed = false;

        std::set<int> allNumbers;
        for (int i = 1; i <= puzzle.size; ++i) {
            allNumbers.insert(i);
        }
        std::vector<std::set<int>> possibles(puzzle.numbers.size(), allNumbers);

        for (auto& g : puzzle.blockGraph) {
            for (int i = 0; i < g.size(); ++i) {
                int idx = g[i];
                int number = puzzle.numbers[idx];
                if (number == 0) {
                    continue;
                }
                for (int j = 0; j < g.size(); ++j) {
                    if (i == j) {
                        continue;
                    }
                    int idx2 = g[j];
                    int number2 = puzzle.numbers[idx2];
                    if (number2 != 0) {
                        continue;
                    }
                    possibles[idx2].erase(number);
                }
            }
        }

        // fill those that only appear once
        for (int i = 0; i < puzzle.numbers.size(); ++i) {
            // already filled out
            if (puzzle.numbers[i] != 0) {
                continue;
            }

            if (possibles[i].size() == 0) {
                std::cerr << "Impossible to solve. No possible option at " << Coord{i} << std::endl;
                puzzle.PrintNumbers();
                exit(1);
                return false;
            }

            if (possibles[i].size() == 1) {
                int number = *possibles[i].begin();
                puzzle.numbers[i] = number;
                std::cout << "Filled(method 1) out " << Coord{i} << " with " << number << std::endl;
                puzzle.PrintNumbers1D(std::cout, i);
                changed = true;
            }
        }

        if (changed) {
            return true;
        }

        // fill those that have only a single place
        for (auto& g : puzzle.blockGraph) {
            std::unordered_map<int, int> occurences;
            for (int i = 0; i < g.size(); ++i) {
                int idx = g[i];
                int number = puzzle.numbers[idx];
                if (number != 0) {
                    continue;
                }
                for (int pos : possibles[idx]) {
                    occurences[pos]++;
                }
            }
            for (auto [num, count] : occurences) {
                if (count != 1) {
                    continue;
                }
                for (int i = 0; i < g.size(); ++i) {
                    int idx = g[i];
                    int& number = puzzle.numbers[idx];
                    if (number != 0) {
                        continue;
                    }
                    if (possibles[idx].count(num)) {
                        number = num;
                        std::cout << "Filled(method 2) out " << Coord{idx} << " with " << number << std::endl;

                        puzzle.PrintNumbers1D(std::cout, idx);
                        changed = true;
                    }
                }
            }
        }
        return changed;
    }

    void Solve() {
        while (SolveNaive()) {}
    }
};

int main() {
    Puzzle p;
    if (!ParsePuzzle(std::cin, p)) {
        return 1;
    }

    std::cout << p << std::endl;

    Solver solver{p};
    solver.Solve();

    std::cout << p << std::endl;
}
