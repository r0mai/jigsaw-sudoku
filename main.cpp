#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>

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
    int size = p.size;
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            os << p.NumberAt(r, c) << ' ';
        }
        os << '\n';
    }

    os << '\n';

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            os << p.ShapeAt(r, c) << ' ';
        }
        os << '\n';
    }

    os << '\n';

    for (auto& g : p.blockGraph) {
        for (int idx : g) {
            os << idx << ' ';
        }
        os << '\n';
    }
    return os;
}

int main() {
    Puzzle p;
    if (!ParsePuzzle(std::cin, p)) {
        return 1;
    }

    std::cout << p << std::endl;
}
