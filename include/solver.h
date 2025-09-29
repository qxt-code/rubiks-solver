#ifndef SOLVER_H
#define SOLVER_H

#include "cube.h"
#include "table_manager.h"
#include <vector>
#include <stack>
#include <tuple>
#include <algorithm>
#include <array>

namespace RubiksSolver {

class Solver {
public:
    Solver(const TableManager& tables);

    std::vector<Move> solve(const Cube& scrambled_cube);

private:
    TableManager const& tables_;

    template<uint8_t PHASE, typename C>
    bool ida_star(C start_coord, std::vector<Move>& solution, int limit) {
        if (start_coord.is_solved()) {
            return true;
        }

        uint16_t x1, x2, x3;
        if constexpr (PHASE == 1) {
            x1 = start_coord.get_corner_orientation();
            x2 = start_coord.get_edge_orientation();
            x3 = start_coord.get_ud_slice_position();
        } else if constexpr (PHASE == 2) {
            x1 = start_coord.get_corner_permutation();
            x2 = start_coord.get_ud_edge_permutation();
            x3 = start_coord.get_slice_edge_permutation();
        }
        int min_depth = heuristic<PHASE>(x1, x2, x3);
        
        std::vector<SearchState> stack;
        stack.reserve(limit + 1);
        
        for (int max_depth = min_depth; max_depth <= limit; ++max_depth) {
            solution.clear();
            solution.resize(limit + 1);
            stack.clear();
            stack.push_back({x1, x2, x3, Move::COUNT, 0, min_depth});

            if (search_iterative<PHASE>(stack, solution, max_depth, start_coord.AVAILABLE_MOVES)) {
                return true;
            }
        }
        
        return false;
    }

    // 迭代搜索的状态结构
    struct SearchState {
        uint16_t x1, x2, x3;
        Move last_move;
        int depth, h;
    };

    template<uint8_t PHASE, size_t N>
    bool search_iterative(std::vector<SearchState>& stack, std::vector<Move>& path, int max_depth, std::array<Move, N> MOVES) {
        const int ENDGAME_DB_MAX_DEPTH = []() {
            if constexpr (PHASE == 1) {
                return 6;
            } else if constexpr (PHASE == 2) {
                return 7;
            }
        }();

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            path[current.depth] = current.last_move;
            if (current.h <= ENDGAME_DB_MAX_DEPTH) {
                std::vector<Move> endgame_path;
                
                if (tables_.search_endgame_db<PHASE>(current.x1, current.x2, current.x3, endgame_path)) {
                    std::cout << "Found endgame solution for (" 
                              << current.x1 << ", " << current.x2 << ", " << current.x3 << ") at depth " 
                              << current.depth 
                              << " in maxdepth " << max_depth
                              << " with " << endgame_path.size() << " moves."
                              << std::endl;

                    path.resize(current.depth + 1);
                    path.insert(path.end(), endgame_path.begin(), endgame_path.end());
                    return true;
                }
                // 增强启发函数，会严格限制解的长度
                // 不开启增强可以获得长度大于当前max_depth的解，可以提前获得深度更高时才能获得的解
#ifdef USE_ENHANCED_HEURISTIC
                if (current.depth + ENDGAME_DB_MAX_DEPTH > max_depth) {
                    continue; // 超过最大深度，跳过
                } else {
                    current.h = ENDGAME_DB_MAX_DEPTH + 1;
                }
#endif
            }
            
            if (current.x1 == 0 && current.x2 == 0 && current.x3 == 0) {
                // 截取到当前深度的路径，path[0]是起始状态，需要去除
                path.resize(current.depth + 1);

                return true;
            }
            
            // 基于启发值，对所有可能的移动进行排序，优先搜索启发值低的移动
            std::array<SearchState, 18> scored_moves;
            int valid_moves = 0;
            
            for (Move move : MOVES) {
                if (!is_valid_move(move, current.last_move)) {
                    continue;
                }
                
                uint16_t next_x1 = current.x1, next_x2 = current.x2, next_x3 = current.x3;
                get_next_coord<PHASE>(next_x1, next_x2, next_x3, move);

#ifdef USE_ENHANCED_HEURISTIC
                int next_h = std::max(heuristic<PHASE>(next_x1, next_x2, next_x3), static_cast<uint8_t>(current.h - 1));
#else
                int next_h = heuristic<PHASE>(next_x1, next_x2, next_x3);
#endif
                if (current.depth + 1 + next_h <= max_depth) {
                    scored_moves[valid_moves++] = {next_x1, next_x2, next_x3, move, current.depth + 1, next_h};
                }
            }
            
            std::sort(scored_moves.begin(), scored_moves.begin() + valid_moves,
                     [](const auto& a, const auto& b) { return a.h < b.h; });
            
            // 按排序后的顺序添加到栈中（逆序，因为栈是LIFO）
            for (int i = valid_moves - 1; i >= 0; --i) {
                stack.push_back(std::move(scored_moves[i]));
            }
        }
        
        
        return false;
    }
                    
    // 启发函数
    inline uint8_t heuristic_phase1(const Phase1Coord& coord) const;
    inline uint8_t heuristic_phase2(const Phase2Coord& coord) const;
    template<uint8_t PHASE>
    inline uint8_t heuristic(uint16_t x1, uint16_t x2, uint16_t x3) const {
        if constexpr (PHASE == 1) {
            return heuristic_phase1(x1, x2, x3);
        } else {
            return heuristic_phase2(x1, x2, x3);
        }
    }
    inline uint8_t heuristic_phase1(uint16_t x1, uint16_t x2, uint16_t x3) const;
    inline uint8_t heuristic_phase2(uint16_t x1, uint16_t x2, uint16_t x3) const;

    template<uint8_t PHASE>
    inline void get_next_coord(uint16_t& x1, uint16_t& x2, uint16_t& x3, Move move) const {
        if constexpr (PHASE == 1) {
            tables_.get_phase1_moves(x1, x2, x3, move, x1, x2, x3);
        } else {
            tables_.get_phase2_moves(x1, x2, x3, move, x1, x2, x3);
        }
    }
    
    // 移动合法性检查
    inline bool is_valid_move(Move current, Move last) const;
};

} // namespace RubiksSolver

#endif // SOLVER_H