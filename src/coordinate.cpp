#include <algorithm>
#include "coordinate.h"

namespace RubiksSolver {

void Phase1Coord::encode_corner_orientation(const Cube& cube) {
    Coord co_coord = 0;
    for (int i = 0; i < 7; ++i) { // 只计算前7个，最后一个是推断出来的
        co_coord *= 3;
        co_coord += cube.corners[i].orientation;
    }
    this->corner_orientation = co_coord;
}

void Phase1Coord::encode_edge_orientation(const Cube& cube) {
    Coord eo_coord = 0;
    for (int i = 0; i < 11; ++i) { // 只计算前11个
        eo_coord *= 2;
        eo_coord += cube.edges[i].orientation;
    }
    this->edge_orientation = eo_coord;
}

void Phase1Coord::encode_ud_slice_position(const Cube& cube) {
    Coord uds_coord = 0;
    std::vector<int> slice_edge_indices;

    // 找到4个中层棱块所在的槽位索引
    for (int i = 0; i < 12; ++i) {
        if (cube.edges[i].piece >= 8 && cube.edges[i].piece <= 11) {
            slice_edge_indices.push_back(11 - i);
            if (slice_edge_indices.size() == 4) break; // 找到4个就停止
        }
    }

    // 对找到的索引进行降序排序
    std::sort(slice_edge_indices.begin(), slice_edge_indices.end(), std::greater<int>());

    // 应用组合数求和公式
    int k = 4; 
    for (int index : slice_edge_indices) {
        uds_coord += C_nk_table[index][k];
        k--;
    }
    this->uds_edge_position = uds_coord;
}


void Phase1Coord::decode_corner_orientation(Cube& cube) {
    int parity = 0;
    
    for(int i = 0; i < 8; ++i) cube.corners[i].orientation = 0;

    // 设置前7个角块的朝向
    for (int i = 6; i >= 0; --i) {
        int ori = this->corner_orientation % 3;
        this->corner_orientation /= 3;
        cube.corners[i].orientation = ori;
        parity += ori;
    }
    // 第8个角块的朝向由前7个决定，总和必须能被3整除
    cube.corners[7].orientation = (3 - (parity % 3)) % 3;
}

void Phase1Coord::decode_edge_orientation(Cube& cube) {
    int parity = 0;
    for(int i = 0; i < 12; ++i) cube.edges[i].orientation = 0;

    for (int i = 10; i >= 0; --i) {
        int ori = this->edge_orientation % 2;
        this->edge_orientation /= 2;
        cube.edges[i].orientation = ori;
        parity += ori;
    }
    cube.edges[11].orientation = (2 - (parity % 2)) % 2;
}

void Phase1Coord::decode_ud_slice_position(Cube& cube) {
    // 清除UDSlice棱块的piece值，UD层棱块对第一阶段没有影响
    for(int i = 0; i < 12; ++i) {
        auto piece = cube.edges[i].piece;
        // UDSlice棱块的piece值范围是8-11
        cube.edges[i].piece = (piece < 8) ? piece : (piece - 4); 
    }

    int k = 4; // 共有4个UDSlice棱块
    
    uds_edge_position = this->uds_edge_position;
    // 从最大的槽位11开始，向下遍历
    for (int i = 0; i < 12 && k > 0; ++i) {
        // 如果坐标值大于“从n以下的位置中选k个”的组合数
        if (uds_edge_position >= C_nk_table[11 - i][k]) {
            // 那么n这个位置必须被选中
            uds_edge_position -= C_nk_table[11 - i][k];

            // 将一个UDSlice棱块(piece >= 8)放到这个槽位
            // 依次选择 8, 9, 10, 11 这几个piece，块顺序对第一阶段没有影响
            cube.edges[i].piece = 7 + k;

            k--;
        }
    }
}

} // namespace RubiksSolver
