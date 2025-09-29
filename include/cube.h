#ifndef CUBE_H
#define CUBE_H

#include "moves.h"
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <concepts>

namespace RubiksSolver {

// 魔方颜色枚举 (基于白顶红前的标准定向)
enum class Color : uint8_t {
    White = 0,   // U面 (上面)
    Yellow = 1,  // D面 (下面)
    Red = 2,     // F面 (前面)
    Orange = 3,  // B面 (后面)
    Green = 4,   // L面 (左面)
    Blue = 5     // R面 (右面)
};

// 魔方面枚举
enum class Face : uint8_t {
    U = 0, D = 1, F = 2, B = 3, L = 4, R = 5
};

inline Face get_face(Move m) { return static_cast<Face>(static_cast<int>(m) / 3); }

// 角块和棱块在已复原状态下的颜色定义
constexpr std::array<std::array<Color, 3>, 8> CORNER_COLORS = {{
    {Color::White, Color::Red, Color::Green},    // UFL (0)
    {Color::White, Color::Green, Color::Orange}, // UBL (1) 
    {Color::White, Color::Orange, Color::Blue}, // UBR (2)
    {Color::White, Color::Blue, Color::Red},   // UFR (3)
    {Color::Yellow, Color::Green, Color::Red},   // DFL (4)
    {Color::Yellow, Color::Orange, Color::Green}, // DBL (5)
    {Color::Yellow, Color::Blue, Color::Orange}, // DBR (6)
    {Color::Yellow, Color::Red, Color::Blue}   // DFR (7)
}};

constexpr std::array<std::array<Color, 2>, 12> EDGE_COLORS = {{
    {Color::White, Color::Red},    // UF (0)
    {Color::White, Color::Green},   // UL (1)
    {Color::White, Color::Orange}, // UB (2)
    {Color::White, Color::Blue},  // UR (3)
    {Color::Yellow, Color::Red},   // DF (4)
    {Color::Yellow, Color::Green},  // DL (5)
    {Color::Yellow, Color::Orange}, // DB (6)
    {Color::Yellow, Color::Blue}, // DR (7)
    {Color::Red, Color::Green},     // FL (8)
    {Color::Orange, Color::Green},  // BL (9)
    {Color::Orange, Color::Blue}, // BR (10)
    {Color::Red, Color::Blue}     // FR (11)
}};

using Piece = uint8_t;
using Orientation = uint8_t;

template<typename T>
concept HasPiece = requires(T t) {
    { t.piece } -> std::convertible_to<uint8_t>;  // T必须包含piece，且可转换为uint8_t
};
// 角块
struct Corner { 
    Piece piece : 3;       // 0-7，只需要3位
    Orientation orientation : 2; // 0-2，只需要2位
    uint8_t _reserved : 3;   // 保留位
};
// 棱块
struct Edge { 
    Piece piece : 4;       // 0-11，需要4位
    Orientation orientation : 1; // 0-1，只需要1位
    uint8_t _reserved : 3;   // 保留位
};

class Cube {
public:
    Cube();

    // 从打乱序列构造
    static Cube from_scramble(const std::string& scramble);
    
    // 应用一次转动
    void apply_move(Move m);
    
    // 应用一个转动序列
    void apply_sequence(const std::vector<Move>& sequence);
    
    // 检查是否已复原
    bool is_solved() const;
    
    // 以平铺的方式打印魔方状态
    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const Cube& cube) {
        return os << cube.to_string();
    }
    
    // 获取指定位置的颜色
    Color get_corner_sticker_color(uint8_t corner_pos, uint8_t sticker_pos) const;
    Color get_edge_sticker_color(uint8_t edge_pos, uint8_t sticker_pos) const;
    
    // 获取魔方面的所有9个sticker颜色
    std::array<Color, 9> get_face_colors(Face face) const;

    // 允许Coordinate类访问内部来计算坐标
    friend class Coordinate;
    friend class Phase1Coord;
    friend class Phase2Coord;

private:
    std::array<Corner, 8> corners;
    std::array<Edge, 12> edges;
};

} // namespace RubiksSolver

#endif // CUBE_H