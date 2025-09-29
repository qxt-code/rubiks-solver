#include "solver.h"
#include "coordinate.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <chrono>

namespace RubiksSolver {

Solver::Solver(const TableManager& tables) : tables_(tables) {
}

std::vector<Move> Solver::solve(const Cube& scrambled_cube) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Move> phase1_solution;
    std::vector<Move> phase2_solution;
    
    // 第一阶段：使用IDA*搜索到达G1子群
    Phase1Coord p1_coord(scrambled_cube);
    if (!ida_star<1>(p1_coord, phase1_solution, 12)) {
        throw std::runtime_error("Phase 1 solution not found within depth limit");
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start);
    // 打印第一阶段的解
    std::erase_if(phase1_solution,
                  [this](Move m) { return m == Move::COUNT; });
    std::cout << "Phase 1 completed with " << phase1_solution.size() << " moves in " << duration1.count() << " ms" << std::endl;
    std::cout << "Phase 1 Solution: ";
    for (const auto& move : phase1_solution) {
        std::cout << move << " ";
    }
    std::cout << std::endl;

    // 应用第一阶段的解，得到G1状态的魔方
    Cube intermediate_cube = scrambled_cube;
    intermediate_cube.apply_sequence(phase1_solution);
    
    
    // 第二阶段：在G1子群内搜索到复原状态
    Phase2Coord p2_coord(intermediate_cube);
    int max_phase2_moves = std::max(8, 25 - static_cast<int>(phase1_solution.size()));

    if (!ida_star<2>(p2_coord, phase2_solution, max_phase2_moves)) {
        throw std::runtime_error("Phase 2 solution not found");
    }

    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - end1);
    // 打印第二阶段的解
    std::erase_if(phase2_solution,
                  [this](Move m) { return m == Move::COUNT; });
    std::cout << "Phase 2 completed with " << phase2_solution.size() << " moves in " << duration2.count() << " ms" << std::endl;
    std::cout << "Phase 2 Solution: ";
    for (const auto& move : phase2_solution) {
        std::cout << move << " ";
    }
    std::cout << std::endl;
    
    // 合并两个阶段的解
    phase1_solution.insert(phase1_solution.end(), 
                          phase2_solution.begin(), phase2_solution.end());
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start);
    std::cout << "Total solving time: " << duration.count() << " ms" << std::endl;

    return phase1_solution;
}

inline uint8_t Solver::heuristic_phase1(const Phase1Coord& coord) const {
    return tables_.get_phase1_pruning(coord);
}

inline uint8_t Solver::heuristic_phase2(const Phase2Coord& coord) const {
    return tables_.get_phase2_pruning(coord);
}

inline uint8_t Solver::heuristic_phase1(uint16_t x1, uint16_t x2, uint16_t x3) const {
    uint8_t h1 = tables_.get_co_pruning(x1);
    uint8_t h2 = tables_.get_eo_pruning(x2);
    uint8_t h3 = tables_.get_uds_pruning(x3);
    
    return std::max({h1, h2, h3});
}

inline uint8_t Solver::heuristic_phase2(uint16_t x1, uint16_t x2, uint16_t x3) const {
    uint8_t h1 = tables_.get_cp_pruning(x1);
    uint8_t h2 = tables_.get_udep_pruning(x2);
    uint8_t h3 = tables_.get_sep_pruning(x3);
    
    return std::max({h1, h2, h3});
}

inline bool Solver::is_valid_move(Move current, Move last) const {
    if (last == Move::COUNT) return true; // 第一步
    
    uint8_t current_face = static_cast<uint8_t>(get_face(current));
    uint8_t last_face = static_cast<uint8_t>(get_face(last));
    
    // 同一面的连续转动可以合并为一次转动
    if (current_face == last_face) {
        return false;
    }

    return true;
}

} // namespace RubiksSolver
