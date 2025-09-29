#include "cube.h"
#include "coordinate.h" // 如果需要
#include <stdexcept>
#include <algorithm>
#include <ranges>

namespace RubiksSolver {

// 角块位置
enum CornerPos : uint8_t { UFL, UBL, UBR, UFR, DFL, DBL, DBR, DFR };

// 棱块位置
enum EdgePos : uint8_t { UF, UL, UB, UR, DF, DL, DB, DR, FL, BL, BR, FR };


// 例如 cycle(arr, {0, 1, 2, 3}) 会将 arr[0]<-arr[1]<-arr[1]<-arr[2]<-arr[3]<-arr[0]
template<typename T, size_t ArraySize>
void cycle_pieces(
    std::array<T, ArraySize>& arr,
    const std::array<uint8_t, 4>& affected_indices, // 受影响的槽位索引
    const std::array<uint8_t, 4>& target_map   // 置换映射表
) {
    std::array<T, 4> temp_pieces;
    for (size_t i = 0; i < 4; ++i) {
        temp_pieces[i] = arr[affected_indices[i]];
    }

    for (size_t i = 0; i < 4; ++i) {
        arr[target_map[i]] = temp_pieces[i];
    }
}


struct MoveMap {
    // 受影响的槽位索引
    std::array<uint8_t, 4> affected_indices;

    // 移动后的位置映射
    std::array<uint8_t, 4> target_map;
};

// 描述一次转动效果的数据结构
struct MoveDefinition {
    // 角块置换
    MoveMap corner_permutation;
    // 棱块置换
    MoveMap edge_permutation;
    // 对应角块的朝向变化 (模3)
    std::array<uint8_t, 4> corner_orientation_changes;
    // 对应棱块的朝向变化 (模2)
    std::array<uint8_t, 4> edge_orientation_changes;
};

const std::array<MoveDefinition, 18> ALL_MOVES_DATA = {{
    // Move::U1
    { {{UFL, UBL, UBR, UFR}, {UBL, UBR, UFR, UFL}}, {{UF, UL, UB, UR}, {UL, UB, UR, UF}}, {0,0,0,0}, {0,0,0,0} },
    // Move::U2
    { {{UFL, UFR, UBR, UBL}, {UFR, UBR, UBL, UFL}}, {{UF, UR, UB, UL}, {UR, UB, UL, UF}}, {0,0,0,0}, {0,0,0,0} },
    // Move::U3
    { {{UFL, UBR, UFR, UBL}, {UBR, UFL, UBL, UFR}}, {{UF, UB, UL, UR}, {UB, UF, UR, UL}}, {0,0,0,0}, {0,0,0,0} },
    // Move::D1
    { {{DFL, DFR, DBR, DBL}, {DFR, DBR, DBL, DFL}}, {{DF, DR, DB, DL}, {DR, DB, DL, DF}}, {0,0,0,0}, {0,0,0,0} },
    // Move::D2
    { {{DFL, DBL, DBR, DFR}, {DBL, DBR, DFR, DFL}}, {{DF, DL, DB, DR}, {DL, DB, DR, DF}}, {0,0,0,0}, {0,0,0,0} },
    // Move::D3
    { {{DFL, DBR, DFR, DBL}, {DBR, DFL, DBL, DFR}}, {{DF, DB, DL, DR}, {DB, DF, DR, DL}}, {0,0,0,0}, {0,0,0,0} },
    // Move::F1
    { {{UFL, UFR, DFR, DFL}, {UFR, DFR, DFL, UFL}}, {{UF, FR, DF, FL}, {FR, DF, FL, UF}}, {2,1,2,1}, {1,1,1,1} },
    // Move::F2
    { {{UFL, DFL, DFR, UFR}, {DFL, DFR, UFR, UFL}}, {{UF, FL, DF, FR}, {FL, DF, FR, UF}}, {2,1,2,1}, {1,1,1,1} },
    // Move::F3
    { {{UFL, DFR, UFR, DFL}, {DFR, UFL, DFL, UFR}}, {{UF, DF, FL, FR}, {DF, UF, FR, FL}}, {0,0,0,0}, {0,0,0,0} },
    // Move::B1
    { {{UBL, DBL, DBR, UBR}, {DBL, DBR, UBR, UBL}}, {{UB, BL, DB, BR}, {BL, DB, BR, UB}}, {1,2,1,2}, {1,1,1,1} },
    // Move::B2
    { {{UBL, UBR, DBR, DBL}, {UBR, DBR, DBL, UBL}}, {{UB, BR, DB, BL}, {BR, DB, BL, UB}}, {1,2,1,2}, {1,1,1,1} },
    // Move::B3
    { {{UBL, DBR, UBR, DBL}, {DBR, UBL, DBL, UBR}}, {{UB, DB, BL, BR}, {DB, UB, BR, BL}}, {0,0,0,0}, {0,0,0,0} },
    // Move::L1
    { {{UFL, DFL, DBL, UBL}, {DFL, DBL, UBL, UFL}}, {{UL, FL, DL, BL}, {FL, DL, BL, UL}}, {1,2,1,2}, {0,0,0,0} },
    // Move::L2
    { {{UFL, UBL, DBL, DFL}, {UBL, DBL, DFL, UFL}}, {{UL, BL, DL, FL}, {BL, DL, FL, UL}}, {1,2,1,2}, {0,0,0,0} },
    // Move::L3
    { {{UFL, DBL, UBL, DFL}, {DBL, UFL, DFL, UBL}}, {{UL, DL, FL, BL}, {DL, UL, BL, FL}}, {0,0,0,0}, {0,0,0,0} },
    // Move::R1
    { {{UFR, UBR, DBR, DFR}, {UBR, DBR, DFR, UFR}}, {{UR, BR, DR, FR}, {BR, DR, FR, UR}}, {2,1,2,1}, {0,0,0,0} },
    // Move::R2
    { {{UFR, DFR, DBR, UBR}, {DFR, DBR, UBR, UFR}}, {{UR, FR, DR, BR}, {FR, DR, BR, UR}}, {2,1,2,1}, {0,0,0,0} },
    // Move::R3
    { {{UFR, DBR, UBR, DFR}, {DBR, UFR, DFR, UBR}}, {{UR, DR, FR, BR}, {DR, UR, BR, FR}}, {0,0,0,0}, {0,0,0,0} },

}};

Cube::Cube() {
    // 初始化角块
    for (uint8_t i = 0; i < 8; ++i) {
        corners[i].piece = i;
        corners[i].orientation = 0;
    }
    // 初始化棱块
    for (uint8_t i = 0; i < 12; ++i) {
        edges[i].piece = i;
        edges[i].orientation = 0;
    }
}

void Cube::apply_move(Move m) {
    if (m >= Move::COUNT)   
        throw std::out_of_range("Invalid move");
    // 1. 从数据表中获取当前转动的定义
    const auto& move_def = ALL_MOVES_DATA[static_cast<int>(m)];

    // 2. 应用置换
    cycle_pieces(corners, move_def.corner_permutation.affected_indices, move_def.corner_permutation.target_map);
    cycle_pieces(edges, move_def.edge_permutation.affected_indices, move_def.edge_permutation.target_map);

    // 3. 应用朝向变化
    for (int i = 0; i < 4; ++i) {
        uint8_t c_idx = move_def.corner_permutation.affected_indices[i];
        corners[c_idx].orientation = (corners[c_idx].orientation + move_def.corner_orientation_changes[i]) % 3;

        uint8_t e_idx = move_def.edge_permutation.affected_indices[i];
        edges[e_idx].orientation = (edges[e_idx].orientation + move_def.edge_orientation_changes[i]) % 2;
    }
}

bool Cube::is_solved() const {
    for (int i = 0; i < 8; ++i) {
        if (corners[i].piece != i || corners[i].orientation != 0) {
            return false;
        }
    }
    for (int i = 0; i < 12; ++i) {
        if (edges[i].piece != i || edges[i].orientation != 0) {
            return false;
        }
    }
    return true;
}

Color Cube::get_corner_sticker_color(uint8_t corner_pos, uint8_t sticker_pos) const {
    const auto& corner = corners[corner_pos];
    // 根据角块的朝向调整sticker位置
    uint8_t actual_sticker = (sticker_pos - corner.orientation + 3) % 3;
    return CORNER_COLORS[corner.piece][actual_sticker];
}

Color Cube::get_edge_sticker_color(uint8_t edge_pos, uint8_t sticker_pos) const {
    const auto& edge = edges[edge_pos];
    // 根据棱块的朝向调整sticker位置
    uint8_t actual_sticker = (sticker_pos + edge.orientation) % 2;
    return EDGE_COLORS[edge.piece][actual_sticker];
}

std::array<Color, 9> Cube::get_face_colors(Face face) const {
    std::array<Color, 9> colors;
    
    switch (face) {
        case Face::U: // 上面 (白面)
            colors[0] = get_corner_sticker_color(UBL, 0); // 左上角
            colors[1] = get_edge_sticker_color(UB, 0);    // 上边
            colors[2] = get_corner_sticker_color(UBR, 0); // 右上角
            colors[3] = get_edge_sticker_color(UL, 0);    // 左边
            colors[4] = Color::White;                     // 中心
            colors[5] = get_edge_sticker_color(UR, 0);    // 右边
            colors[6] = get_corner_sticker_color(UFL, 0); // 左下角
            colors[7] = get_edge_sticker_color(UF, 0);    // 下边
            colors[8] = get_corner_sticker_color(UFR, 0); // 右下角
            break;
            
        case Face::D: // 下面 (黄面)
            colors[0] = get_corner_sticker_color(DFL, 0); // 左上角
            colors[1] = get_edge_sticker_color(DF, 0);    // 上边
            colors[2] = get_corner_sticker_color(DFR, 0); // 右上角
            colors[3] = get_edge_sticker_color(DL, 0);    // 左边
            colors[4] = Color::Yellow;                    // 中心
            colors[5] = get_edge_sticker_color(DR, 0);    // 右边
            colors[6] = get_corner_sticker_color(DBL, 0); // 左下角
            colors[7] = get_edge_sticker_color(DB, 0);    // 下边
            colors[8] = get_corner_sticker_color(DBR, 0); // 右下角
            break;
            
        case Face::F: // 前面 (红面)
            colors[0] = get_corner_sticker_color(UFL, 1); // 左上角
            colors[1] = get_edge_sticker_color(UF, 1);    // 上边
            colors[2] = get_corner_sticker_color(UFR, 2); // 右上角
            colors[3] = get_edge_sticker_color(FL, 0);    // 左边
            colors[4] = Color::Red;                       // 中心
            colors[5] = get_edge_sticker_color(FR, 0);    // 右边
            colors[6] = get_corner_sticker_color(DFL, 2); // 左下角
            colors[7] = get_edge_sticker_color(DF, 1);    // 下边
            colors[8] = get_corner_sticker_color(DFR, 1); // 右下角
            break;
            
        case Face::B: // 后面 (橙面)
            colors[0] = get_corner_sticker_color(UBR, 1); // 左上角
            colors[1] = get_edge_sticker_color(UB, 1);    // 上边
            colors[2] = get_corner_sticker_color(UBL, 2); // 右上角
            colors[3] = get_edge_sticker_color(BR, 0);    // 左边
            colors[4] = Color::Orange;                    // 中心
            colors[5] = get_edge_sticker_color(BL, 0);    // 右边
            colors[6] = get_corner_sticker_color(DBR, 2); // 左下角
            colors[7] = get_edge_sticker_color(DB, 1);    // 下边
            colors[8] = get_corner_sticker_color(DBL, 1); // 右下角
            break;
            
        case Face::L: // 左面 (绿面)
            colors[0] = get_corner_sticker_color(UBL, 1); // 左上角
            colors[1] = get_edge_sticker_color(UL, 1);    // 上边
            colors[2] = get_corner_sticker_color(UFL, 2); // 右上角
            colors[3] = get_edge_sticker_color(BL, 1);    // 左边
            colors[4] = Color::Green;                      // 中心
            colors[5] = get_edge_sticker_color(FL, 1);    // 右边
            colors[6] = get_corner_sticker_color(DBL, 2); // 左下角
            colors[7] = get_edge_sticker_color(DL, 1);    // 下边
            colors[8] = get_corner_sticker_color(DFL, 1); // 右下角
            break;

        case Face::R: // 右面 (蓝面)
            colors[0] = get_corner_sticker_color(UFR, 1); // 左上角
            colors[1] = get_edge_sticker_color(UR, 1);    // 上边
            colors[2] = get_corner_sticker_color(UBR, 2); // 右上角
            colors[3] = get_edge_sticker_color(FR, 1);    // 左边
            colors[4] = Color::Blue;                       // 中心
            colors[5] = get_edge_sticker_color(BR, 1);    // 右边
            colors[6] = get_corner_sticker_color(DFR, 2); // 左下角
            colors[7] = get_edge_sticker_color(DR, 1);    // 下边
            colors[8] = get_corner_sticker_color(DBR, 1); // 右下角
            break;
            
    }
    
    return colors;
}

std::string Cube::to_string() const {
    // 定义颜色 (使用256色获得更好的显示效果)
    const std::string W = "\x1b[48;5;255;30m"; // White background, black text
    const std::string R = "\x1b[48;5;196;30m"; // Red background, black text
    const std::string G = "\x1b[48;5;46;30m";  // Green background, black text
    const std::string B = "\x1b[48;5;21;97m";  // Blue background, white text
    const std::string Y = "\x1b[48;5;226;30m"; // Yellow background, black text
    const std::string O = "\x1b[48;5;208;30m"; // Orange background, black text
    const std::string Z = "\x1b[0m";           // Reset
    const std::string S = "  ";               // Sticker size
    
    // 获取颜色字符串的辅助函数
    auto get_color_string = [&](Color c) -> std::string {
        switch (c) {
            case Color::White:  return W + S;
            case Color::Red:    return R + S;
            case Color::Green:  return G + S;
            case Color::Blue:   return B + S;
            case Color::Yellow: return Y + S;
            case Color::Orange: return O + S;
            default: return W + S;
        }
    };
    
    std::string result;
    
    // 获取各个面的颜色
    auto u_colors = get_face_colors(Face::U);
    auto f_colors = get_face_colors(Face::F);
    auto r_colors = get_face_colors(Face::R);
    auto b_colors = get_face_colors(Face::B);
    auto l_colors = get_face_colors(Face::L);
    auto d_colors = get_face_colors(Face::D);
    
    // 魔方的平铺展示格式：
    //       U U U
    //       U U U  
    //       U U U
    // L L L F F F R R R B B B
    // L L L F F F R R R B B B
    // L L L F F F R R R B B B
    //       D D D
    //       D D D
    //       D D D
    
    result += "      " + get_color_string(u_colors[0]) + get_color_string(u_colors[1]) + get_color_string(u_colors[2]) + Z + "\n";
    result += "      " + get_color_string(u_colors[3]) + get_color_string(u_colors[4]) + get_color_string(u_colors[5]) + Z + "\n";
    result += "      " + get_color_string(u_colors[6]) + get_color_string(u_colors[7]) + get_color_string(u_colors[8]) + Z + "\n";
    result += "\n";
    
    // 中间三行: L F R B
    for (int row = 0; row < 3; ++row) {
        result += get_color_string(l_colors[row * 3 + 0]) + get_color_string(l_colors[row * 3 + 1]) + get_color_string(l_colors[row * 3 + 2]);
        result += get_color_string(f_colors[row * 3 + 0]) + get_color_string(f_colors[row * 3 + 1]) + get_color_string(f_colors[row * 3 + 2]);
        result += get_color_string(r_colors[row * 3 + 0]) + get_color_string(r_colors[row * 3 + 1]) + get_color_string(r_colors[row * 3 + 2]);
        result += get_color_string(b_colors[row * 3 + 0]) + get_color_string(b_colors[row * 3 + 1]) + get_color_string(b_colors[row * 3 + 2]);
        result += Z + "\n";
    }
    result += "\n";
    
    result += "      " + get_color_string(d_colors[0]) + get_color_string(d_colors[1]) + get_color_string(d_colors[2]) + Z + "\n";
    result += "      " + get_color_string(d_colors[3]) + get_color_string(d_colors[4]) + get_color_string(d_colors[5]) + Z + "\n";
    result += "      " + get_color_string(d_colors[6]) + get_color_string(d_colors[7]) + get_color_string(d_colors[8]) + Z + "\n";
    
    return result;
}

Cube Cube::from_scramble(const std::string& scramble) {
    Cube cube;
    auto split_view = scramble | std::views::split(' ');
    for (const auto& split : split_view) {
        std::string move_str(split.begin(), split.end());
        if (move_str.empty()) continue;
        Move move = string_to_move(move_str);
        cube.apply_move(move);
    }
    return cube;
}

void Cube::apply_sequence(const std::vector<Move>& sequence) {
    for (const Move& move : sequence) {
        apply_move(move);
    }
}

} // namespace RubiksSolver