#ifndef TABLE_MANAGER_H
#define TABLE_MANAGER_H

#include "coordinate.h"
#include "moves.h"
#include "persistence.h"
#include <array>
#include <string>
#include <functional>
#include <queue>

namespace RubiksSolver {

class TableManager {
public:
    // 获取单例实例
    static const TableManager& get_instance();

    // 移动表查询
    // Phase 1
    inline uint16_t get_co_move(uint16_t coord, Move m) const {
        return co_move_table[coord][static_cast<uint8_t>(m)];
    }
    inline uint16_t get_eo_move(uint16_t coord, Move m) const {
        return eo_move_table[coord][static_cast<uint8_t>(m)];
    }
    inline uint16_t get_uds_move(uint16_t coord, Move m) const {
        return uds_move_table[coord][static_cast<uint8_t>(m)];
    }
    // Phase 2
    inline uint16_t get_cp_move(uint16_t coord, Move m) const {
        return cp_move_table[coord][static_cast<uint8_t>(m)];
    }
    inline uint16_t get_udep_move(uint16_t coord, Move m) const {
        return udep_move_table[coord][static_cast<uint8_t>(m)];
    }
    inline uint16_t get_sep_move(uint16_t coord, Move m) const {
        return sep_move_table[coord][static_cast<uint8_t>(m)];
    }

    // 剪枝表查询
    inline uint8_t get_co_pruning(uint16_t co_coord) const {
        return co_pruning_table[co_coord];
    }
    inline uint8_t get_eo_pruning(uint16_t eo_coord) const {
        return eo_pruning_table[eo_coord];
    }
    inline uint8_t get_uds_pruning(uint16_t uds_coord) const {
        return uds_pruning_table[uds_coord];
    }

    inline uint8_t get_cp_pruning(uint16_t cp_coord) const {
        return cp_pruning_table[cp_coord];
    }
    inline uint8_t get_udep_pruning(uint16_t udep_coord) const {
        return udep_pruning_table[udep_coord];
    }
    inline uint8_t get_sep_pruning(uint16_t sep_coord) const {
        return sep_pruning_table[sep_coord];
    }
    
    // 批量查询
    inline void get_phase1_moves(uint16_t co, uint16_t eo, uint16_t uds, Move m,
                                 uint16_t& new_co, uint16_t& new_eo, uint16_t& new_uds) const {
        uint8_t move_idx = static_cast<uint8_t>(m);
        new_co = co_move_table[co][move_idx];
        new_eo = eo_move_table[eo][move_idx];
        new_uds = uds_move_table[uds][move_idx];
    }
    
    inline void get_phase2_moves(uint16_t cp, uint16_t udep, uint16_t sep, Move m,
                                 uint16_t& new_cp, uint16_t& new_udep, uint16_t& new_sep) const {
        uint8_t move_idx = static_cast<uint8_t>(m);
        new_cp = cp_move_table[cp][move_idx];
        new_udep = udep_move_table[udep][move_idx];
        new_sep = sep_move_table[sep][move_idx];
    }
    
    // 复合启发函数 - 取最大值
    uint8_t get_phase1_pruning(const Phase1Coord& coord) const;
    uint8_t get_phase2_pruning(const Phase2Coord& coord) const;

    // 获取Phase1或Phase2的终局数据库
    template<uint8_t PHASE>
    inline bool search_endgame_db(uint16_t x1, uint16_t x2, uint16_t x3, std::vector<Move>& path) const {
        const auto& endgame_db = get_endgame_db<PHASE>();

        uint64_t key = get_key(x1, x2, x3);

        auto it = endgame_db.find(key);
        if (it != endgame_db.end()) {
            path = it->second;
            return true;
        }
        return false;
    }
    

private:
    template<uint16_t N>
    using MoveTable = std::array<std::array<uint16_t, 18>, N>;
    template<uint16_t N>
    using PruningTable = std::array<uint8_t, N>;
    using EndgameDB = std::unordered_map<uint64_t, std::vector<Move>>;

    TableManager();

    void initialize();

    // 生成移动表
    template<typename C, typename Set, typename Get, size_t N>
    void generate_move_table(
                    const std::string& name, 
                    MoveTable<N>& table, 
                    Set&& set, 
                    Get&& get) {
        std::cout << "Generating " << name << " Move Table..." << std::endl;
        C coord;
        for (uint16_t i = 0; i < N; ++i) {
            set(coord, i);

            for (auto move : coord.AVAILABLE_MOVES) {
                C temp_coord = coord;
                temp_coord.apply_move(move);

                table[i][static_cast<int>(move)] = get(temp_coord);
            }
        }
        std::cout << name << " Move Table generated." << std::endl;
    }

    void generate_co_move_table();
    void generate_eo_move_table();
    void generate_uds_move_table();
    void generate_cp_move_table();
    void generate_udep_move_table();
    void generate_sep_move_table();

    // 生成剪枝表
    template<typename C, typename Get, size_t SIZE>
    void generate_pruning_table(
                                    const std::string& name,
                                    PruningTable<SIZE>& table,
                                    Get&& get_next_coord) {
        std::cout << "Generating Pruning Table: " << name << "..." << std::endl;

        table.fill(0xFF); // 用 0xFF (即-1的无符号等价值) 代表 "未访问"
        std::queue<uint16_t> q;

        table[0] = 0; // 目标状态距离为0
        q.push(0);

        int visited_count = 1;
        int current_depth = 0;

        while (!q.empty()) {
            int layer_size = q.size();
            std::cout << "  Depth " << current_depth << ": " << layer_size << " states" << std::endl;

            for (int i = 0; i < layer_size; ++i) {
                uint16_t current_coord = q.front();
                q.pop();

                for (auto move : C::AVAILABLE_MOVES) {
                    uint16_t next_coord = get_next_coord(current_coord, move);
                    if (next_coord >= 40320) {
                        throw std::out_of_range("Permutation rank exceeds maximum value for 8 pieces.");
                    }

                    if (table[next_coord] == 0xFF) { // 如果尚未访问
                        table[next_coord] = current_depth + 1;
                        q.push(next_coord);
                        visited_count++;
                    }
                }
            }
            current_depth++;
        }
        std::cout << name << " generated. Total states: " << visited_count << "." << std::endl;
    }

    
    // 生成终局数据库
    template<uint8_t PHASE, typename C>
    void generate_endgame_db() {

        const int MAX_DEPTH = []() {
            if constexpr (PHASE == 1) {
                return 5; // Phase 1 最大深度
            } else if constexpr (PHASE == 2) {
                return 6; // Phase 2 最大深度
            }
        }();

        std::cout << "Generating Endgame Database (Depth=" << MAX_DEPTH << ")..." << std::endl;

        std::queue<std::tuple<uint16_t, uint16_t, uint16_t, std::vector<Move>>> q;

        auto& endgame_db = get_endgame_db<PHASE>();
        
        endgame_db[0] = std::vector<Move>();
        q.push({0, 0, 0, std::vector<Move>()});

        int current_depth = 0;
        while(!q.empty()) {
            int layer_size = q.size();
            std::cout << "  Depth " << current_depth << ": " << layer_size << " states" << std::endl;

            for (int i = 0; i < layer_size; ++i) {

                auto [x1, x2, x3, path] = q.front();
                q.pop();

                for (auto move : C::AVAILABLE_MOVES) {
                    uint16_t next_x1 = x1, next_x2 = x2, next_x3 = x3;

                    if constexpr (PHASE == 1) {
                        get_phase1_moves(x1, x2, x3, move, next_x1, next_x2, next_x3);
                    } else if constexpr (PHASE == 2) {
                        get_phase2_moves(x1, x2, x3, move, next_x1, next_x2, next_x3);
                    }

                    uint64_t next_key = get_key(next_x1, next_x2, next_x3);

                    if (endgame_db.find(next_key) == endgame_db.end()) {
                        // 创建新的解法：它是父解法加上当前转动的“逆”
                        std::vector<Move> next_path = path;
                        next_path.push_back(invert_move(move));

                        if (current_depth < MAX_DEPTH) {
                            q.push({next_x1, next_x2, next_x3, next_path});
                        }

                        std::reverse(next_path.begin(), next_path.end());
                        endgame_db[next_key] = std::move(next_path);
                    }
                }
            }
            ++current_depth;
        }
        std::cout << "Endgame Database generated. Total states: " << endgame_db.size() << std::endl;
    }

    inline uint64_t get_key(uint16_t x1, uint16_t x2, uint16_t x3) const {
        return (static_cast<uint64_t>(x1) << 32) | (static_cast<uint64_t>(x2) << 16) | x3;
    }

    template<uint8_t PHASE>
    constexpr const auto& get_endgame_db() const {
        if constexpr (PHASE == 1) {
            return p1_endgame_db;
        } else if constexpr (PHASE == 2) {
            return p2_endgame_db;
        } else {
            static_assert(PHASE == 1 || PHASE == 2, "Only phases 1 and 2 are supported");
        }
    }

    template<uint8_t PHASE>
    constexpr auto& get_endgame_db() {
        if constexpr (PHASE == 1) {
            return p1_endgame_db;
        } else if constexpr (PHASE == 2) {
            return p2_endgame_db;
        } else {
            static_assert(PHASE == 1 || PHASE == 2, "Only phases 1 and 2 are supported");
        }
    }

    // 移动表
    MoveTable<2187> co_move_table;
    MoveTable<2048> eo_move_table;
    MoveTable<495> uds_move_table;
    MoveTable<40320> cp_move_table;
    MoveTable<40320> udep_move_table;
    MoveTable<24> sep_move_table;
    
    // 剪枝表
    PruningTable<2187> co_pruning_table;
    PruningTable<2048> eo_pruning_table;
    PruningTable<495> uds_pruning_table;
    PruningTable<40320> cp_pruning_table;
    PruningTable<40320> udep_pruning_table;
    PruningTable<24> sep_pruning_table;
    // 反向索引表
    EndgameDB p1_endgame_db;
    EndgameDB p2_endgame_db;
};

} // namespace RubiksSolver

#endif // TABLE_MANAGER_H