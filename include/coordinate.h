#ifndef COORDINATE_H
#define COORDINATE_H

#include "cube.h"
#include <cstdint>
#include <span>
#include <ranges>
#include <algorithm>
#include <iostream>

#include <cassert>


namespace RubiksSolver {

using Coord = uint16_t;

// C(n, k) 查询表
constexpr std::array<std::array<int, 13>, 13> precompute_combinations() {
    std::array<std::array<int, 13>, 13> table{};
    for (int n = 0; n < 13; ++n) {
        table[n][0] = 1;
        for (int k = 1; k <= n; ++k) {
            table[n][k] = table[n - 1][k - 1] + table[n - 1][k];
        }
    }
    return table;
}
constexpr std::array<std::array<int, 13>, 13> C_nk_table = precompute_combinations();

constexpr std::array<int, 9> factorials = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};

struct Phase1Coord {
    
public:
    Phase1Coord() 
    : corner_orientation(0), edge_orientation(0), uds_edge_position(0) {}
    
    Phase1Coord(const Cube& cube) {
        this->cube = cube;
        encode_from_cube(cube);
    }
    
    Phase1Coord(Coord co, Coord eo, Coord uds) 
    : cube(Cube()), corner_orientation(co), edge_orientation(eo), uds_edge_position(uds) { decode_to_cube(cube); }
    // 应用一次转动,修改内部魔方状态，并更新坐标
    void apply_move(Move m) {
        cube.apply_move(m);
        encode_from_cube(cube);
    }
    // 设置角块朝向坐标，并更新魔方状态
    void set_corner_orientation(Coord co) {
        corner_orientation = co;
        decode_corner_orientation(cube);
    }
    
    void set_edge_orientation(Coord eo) {
        edge_orientation = eo;
        decode_edge_orientation(cube);
    }
    
    void set_ud_slice_edges(Coord uds) {
        uds_edge_position = uds;
        decode_ud_slice_position(cube);
    }
    
    inline Coord get_corner_orientation() const { return corner_orientation; }
    inline Coord get_edge_orientation() const { return edge_orientation; }
    inline Coord get_ud_slice_position() const { return uds_edge_position; }

    inline bool is_solved() const {
        return corner_orientation == 0 && edge_orientation == 0 && uds_edge_position == 0;
    }

    // 第一阶段允许的移动 (所有18个)
    static constexpr std::array<Move, 18> AVAILABLE_MOVES = {
        Move::U1, Move::U2, Move::U3, Move::D1, Move::D2, Move::D3,
        Move::L1, Move::L2, Move::L3, Move::R1, Move::R2, Move::R3,
        Move::F1, Move::F2, Move::F3, Move::B1, Move::B2, Move::B3
    };

private:
    // 设置角块朝向坐标编码
    void encode_corner_orientation(const Cube& cube);
    // 设置棱块朝向坐标编码
    void encode_edge_orientation(const Cube& cube);
    // 设置UDSlice棱块位置坐标编码
    void encode_ud_slice_position(const Cube& cube);
    void encode_from_cube(const Cube& cube) {
        encode_corner_orientation(cube);
        encode_edge_orientation(cube);
        encode_ud_slice_position(cube);
    }

    // 解码角块朝向坐标
    void decode_corner_orientation(Cube& cube);
    // 解码棱块朝向坐标
    void decode_edge_orientation(Cube& cube);
    // 解码UDSlice棱块位置坐标
    void decode_ud_slice_position(Cube& cube);
    void decode_to_cube(Cube& cube) {
        decode_corner_orientation(cube);
        decode_edge_orientation(cube);
        decode_ud_slice_position(cube);
    }

    Cube cube;
    // 角块朝向坐标 (0-2186)
    Coord corner_orientation = 0;
    // 棱块朝向坐标 (0-2047)
    Coord edge_orientation = 0;
    // UDSlice棱块位置坐标 (0-494)
    Coord uds_edge_position = 0;
};

struct Phase2Coord {

public:
    Phase2Coord() 
    : cube(Cube()), corner_permutation(0), ud_edge_permutation(0), slice_edge_permutation(0) {}
    
    Phase2Coord(const Cube& cube) {
        this->cube = cube;
        encode_from_cube(cube);
    }

    Phase2Coord(Coord cp, Coord udep, Coord sep) 
    : cube(Cube()), corner_permutation(cp), ud_edge_permutation(udep), slice_edge_permutation(sep) { decode_to_cube(cube); }

    void apply_move(Move m) {
        cube.apply_move(m);
        encode_from_cube(cube);
    }

    void set_corner_permutation(Coord cp) {
        corner_permutation = cp;
        decode_corner_permutation(cube);
    }
    void set_ud_edge_permutation(Coord udep) {
        ud_edge_permutation = udep;
        decode_ud_edge_permutation(cube);
    }
    void set_slice_edge_permutation(Coord sep) {
        slice_edge_permutation = sep;
        decode_slice_edge_permutation(cube);
    }

    inline Coord get_corner_permutation() const { return corner_permutation; }
    inline Coord get_ud_edge_permutation() const { return ud_edge_permutation; }
    inline Coord get_slice_edge_permutation() const { return slice_edge_permutation; }

    inline bool is_solved() const {
        return corner_permutation == 0 && ud_edge_permutation == 0 && slice_edge_permutation == 0;
    }

    // 第二阶段允许的移动 (F、B、L、R面只允许180度转动)
    static constexpr std::array<Move, 10> AVAILABLE_MOVES = {
        Move::U1, Move::U2, Move::U3, Move::D1, Move::D2, Move::D3,
        Move::L3, Move::R3, Move::F3, Move::B3
    };

private:
    // 编码角块排列坐标
    void encode_corner_permutation(const Cube& cube) {
        this->corner_permutation = encode_perm<Coord, Corner>(cube.corners);
    }
    // 编码UD层棱块排列坐标
    void encode_ud_edge_permutation(const Cube& cube) {
        this->ud_edge_permutation = encode_perm<Coord, Edge>(std::span(cube.edges).subspan(0, 8));
    }
    // 编码中层棱块排列坐标
    void encode_slice_edge_permutation(const Cube& cube) {
        this->slice_edge_permutation = encode_perm<Coord, Edge>(std::span(cube.edges).subspan(8, 4));
    }
    void encode_from_cube(const Cube& cube) {
        encode_corner_permutation(cube);
        encode_ud_edge_permutation(cube);
        encode_slice_edge_permutation(cube);
    }

    void decode_corner_permutation(Cube& cube) {
        decode_perm<Corner, Coord>(cube.corners, corner_permutation);
    }
    void decode_ud_edge_permutation(Cube& cube) {
        decode_perm<Edge, Coord>(std::span(cube.edges).subspan(0, 8), ud_edge_permutation);
    }
    void decode_slice_edge_permutation(Cube& cube) {
        decode_perm<Edge, Coord>(std::span(cube.edges).subspan(8, 4), slice_edge_permutation);
    }
    void decode_to_cube(Cube& cube) {
        decode_corner_permutation(cube);
        decode_ud_edge_permutation(cube);
        decode_slice_edge_permutation(cube);
    }

    
    // 排列编码为坐标
    template<typename R, HasPiece T>
    R encode_perm(std::span<const T> perm) {

        R rank = 0;

        int n = static_cast<int>(perm.size());
        std::vector<uint8_t> available_pieces;
        if (n == 8) {
            available_pieces = {0, 1, 2, 3, 4, 5, 6, 7};
        } else {
            available_pieces = {8, 9, 10, 11};
        }

        for (int i = 0; i < n; ++i) {
            // 找到当前piece在可用列表中的索引
            auto it = std::find_if(available_pieces.begin(), available_pieces.end(), [&](const uint8_t& piece) { return piece == perm[i].piece; });
            if (it == available_pieces.end()) {
                throw std::invalid_argument("Piece not found in available pieces.");
            }
            int index = std::distance(available_pieces.begin(), it);
            
            // 累加到rank中
            rank += index * factorials[n - 1 - i];
            
            // 从可用列表中移除
            available_pieces.erase(it);
        }
        // rank 小于40320
        if (rank >= 40320) {
            throw std::out_of_range("Permutation rank exceeds maximum value for 8 pieces.");
        }
        return rank;
    }

    // 解码坐标为排列
    template<HasPiece T, typename R>
    void decode_perm(std::span<T> perm, R rank) const {
        int n = static_cast<int>(perm.size());
        std::vector<uint8_t> available_pieces;
        if (n == 8) {
            available_pieces = {0, 1, 2, 3, 4, 5, 6, 7};
        } else {
            available_pieces = {8, 9, 10, 11};
        }

        for (int i = 0; i < n; ++i) {
            int index = rank / factorials[n - 1 - i];
            perm[i].piece = available_pieces[index];
            available_pieces.erase(available_pieces.begin() + index);
            rank %= factorials[n - 1 - i];
        }
    }

    Cube cube;
    // 角块排列坐标 (0-40319)(8!)
    Coord corner_permutation = 0;
    // UD层棱块排列坐标 (0-40319)(8!)
    Coord ud_edge_permutation = 0;
    // 中层棱块排列坐标 (0-23)(4!)
    Coord  slice_edge_permutation = 0;
};

} // namespace RubiksSolver

#endif // COORDINATE_H