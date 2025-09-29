#ifndef MOVES_H
#define MOVES_H

#include <array>
#include <stdexcept>
#include <string>
#include <cstdint>

#include <unordered_map>

namespace RubiksSolver {

enum class Move : uint8_t {
    U1, U2, U3, // U, U', U2
    D1, D2, D3,
    F1, F2, F3,
    B1, B2, B3,
    L1, L2, L3,
    R1, R2, R3,
    COUNT = 18

};

// 移动对应的字符列表
inline const std::array<std::string, 18> MOVE_STRINGS = {
    "U", "U'", "U2",
    "D", "D'", "D2",
    "F", "F'", "F2",
    "B", "B'", "B2",
    "L", "L'", "L2",
    "R", "R'", "R2",
};

// 字符对应的移动枚举
inline const std::unordered_map<std::string, Move> MOVE_MAP = {
    {"U", Move::U1}, {"U'", Move::U2}, {"U2", Move::U3},
    {"D", Move::D1}, {"D'", Move::D2}, {"D2", Move::D3},
    {"F", Move::F1}, {"F'", Move::F2}, {"F2", Move::F3},
    {"B", Move::B1}, {"B'", Move::B2}, {"B2", Move::B3},
    {"L", Move::L1}, {"L'", Move::L2}, {"L2", Move::L3},
    {"R", Move::R1}, {"R'", Move::R2}, {"R2", Move::R3},
};

inline std::string move_to_string(const Move& m) {
    return MOVE_STRINGS[static_cast<int>(m)];
}

inline Move string_to_move(const std::string& str) {
    auto it = MOVE_MAP.find(str);
    if (it != MOVE_MAP.end()) {
        return it->second;
    }
    throw std::invalid_argument("Invalid move string: " + str);
}


inline std::ostream& operator<<(std::ostream& os, Move m) {
    return os << MOVE_STRINGS[static_cast<int>(m)];
};


// 获取逆转动
inline Move invert_move(Move m) { 
    int base = static_cast<int>(m) / 3 * 3;
    int amount = static_cast<int>(m) % 3;
    return static_cast<Move>(base + (amount == 2 ? 2 : 1 - amount));
}



} // namespace RubiksSolver

#endif // MOVES_H