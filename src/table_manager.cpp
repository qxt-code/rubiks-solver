#include "table_manager.h"
#include <iostream>

namespace RubiksSolver {

const TableManager& TableManager::get_instance() {
    static TableManager instance;
    return instance;
}

// 构造函数，初始化所有表格
TableManager::TableManager() {
    initialize();
}

void TableManager::initialize() {
    std::cout << "Initializing tables..." << std::endl;
    std::cout << "Loading or generating move tables..." << std::endl;
    if (load_array_binary(co_move_table, "data/co_move_table.bin") &&
        load_array_binary(eo_move_table, "data/eo_move_table.bin") &&
        load_array_binary(uds_move_table, "data/uds_move_table.bin") &&
        load_array_binary(cp_move_table, "data/cp_move_table.bin") &&
        load_array_binary(udep_move_table, "data/udep_move_table.bin") &&
        load_array_binary(sep_move_table, "data/sep_move_table.bin")) {
        std::cout << "All move tables loaded successfully." << std::endl;
    } else {
        create_directory("data");
        std::cout << "Generating move tables..." << std::endl;
        generate_co_move_table();
        generate_eo_move_table();
        generate_uds_move_table();
        generate_cp_move_table();
        generate_udep_move_table();
        generate_sep_move_table();
        
        std::cout << "Saving move tables..." << std::endl;
        
        save_array_binary(co_move_table, "data/co_move_table.bin");
        save_array_binary(eo_move_table, "data/eo_move_table.bin");
        save_array_binary(uds_move_table, "data/uds_move_table.bin");
        save_array_binary(cp_move_table, "data/cp_move_table.bin");
        save_array_binary(udep_move_table, "data/udep_move_table.bin");
        save_array_binary(sep_move_table, "data/sep_move_table.bin");
        std::cout << "Move tables generated and saved." << std::endl;
    }

    std::cout << "Loading or generating pruning tables..." << std::endl;
    if (load_array_binary(co_pruning_table, "data/co_pruning_table.bin") &&
        load_array_binary(eo_pruning_table, "data/eo_pruning_table.bin") &&
        load_array_binary(uds_pruning_table, "data/uds_pruning_table.bin") &&
        load_array_binary(cp_pruning_table, "data/cp_pruning_table.bin") &&
        load_array_binary(udep_pruning_table, "data/udep_pruning_table.bin") &&
        load_array_binary(sep_pruning_table, "data/sep_pruning_table.bin")) {
        std::cout << "All pruning tables loaded successfully." << std::endl;
    } else {
        std::cout << "Generating pruning tables..." << std::endl;
        generate_pruning_table<Phase1Coord>("Corner Orientation Pruning", co_pruning_table,
            [&](uint16_t coord, Move m) { return get_co_move(coord, m); });
        generate_pruning_table<Phase1Coord>("Edge Orientation Pruning", eo_pruning_table,
            [&](uint16_t coord, Move m) { return get_eo_move(coord, m); });
        generate_pruning_table<Phase1Coord>("UDSlice Edge Position Pruning", uds_pruning_table,
            [&](uint16_t coord, Move m) { return get_uds_move(coord, m); });
        generate_pruning_table<Phase2Coord>("Corner Permutation Pruning", cp_pruning_table,
            [&](uint16_t coord, Move m) { return get_cp_move(coord, m); });
        generate_pruning_table<Phase2Coord>("UD Edge Permutation Pruning", udep_pruning_table,
            [&](uint16_t coord, Move m) { return get_udep_move(coord, m); });
        generate_pruning_table<Phase2Coord>("Slice Edge Permutation Pruning", sep_pruning_table,
            [&](uint16_t coord, Move m) { return get_sep_move(coord, m); });

        std::cout << "Saving pruning tables..." << std::endl;

        save_array_binary(co_pruning_table, "data/co_pruning_table.bin");
        save_array_binary(eo_pruning_table, "data/eo_pruning_table.bin");
        save_array_binary(uds_pruning_table, "data/uds_pruning_table.bin");
        save_array_binary(cp_pruning_table, "data/cp_pruning_table.bin");
        save_array_binary(udep_pruning_table, "data/udep_pruning_table.bin");
        save_array_binary(sep_pruning_table, "data/sep_pruning_table.bin");
        std::cout << "Pruning tables generated and saved." << std::endl;
    }
    
    std::cout << "Loading or generating endgame databases..." << std::endl;
    if (load_map_binary(p1_endgame_db, "data/p1_endgame_db.bin") &&
        load_map_binary(p2_endgame_db, "data/p2_endgame_db.bin")) {
        std::cout << "Endgame databases loaded successfully." << std::endl;
    } else {
        std::cout << "Generating endgame databases..." << std::endl;
        generate_endgame_db<1, Phase1Coord>();
        generate_endgame_db<2, Phase2Coord>();
        
        std::cout << "Saving endgame databases..." << std::endl;
        save_map_binary(p1_endgame_db, "data/p1_endgame_db.bin");
        save_map_binary(p2_endgame_db, "data/p2_endgame_db.bin");
        std::cout << "Endgame databases generated and saved." << std::endl;
    }
    std::cout << "All tables initialized." << std::endl;
    std::cout << "Initialization complete." << std::endl;
    
}

void TableManager::generate_co_move_table() {
    generate_move_table<Phase1Coord>("Corner Orientation", co_move_table,
        [&](Phase1Coord& coord, uint16_t i) { coord.set_corner_orientation(i); },
        [&](Phase1Coord& coord) -> uint16_t { return coord.get_corner_orientation(); });
}

void TableManager::generate_eo_move_table() {
    generate_move_table<Phase1Coord>("Edge Orientation", eo_move_table,
        [&](Phase1Coord& coord, uint16_t i) { coord.set_edge_orientation(i); },
        [&](Phase1Coord& coord) -> uint16_t { return coord.get_edge_orientation(); });
}

void TableManager::generate_uds_move_table() {
    generate_move_table<Phase1Coord>("UDSlice Edge Position", uds_move_table,
        [&](Phase1Coord& coord, uint16_t i) { coord.set_ud_slice_edges(i); },
        [&](Phase1Coord& coord) -> uint16_t { return coord.get_ud_slice_position(); });
}

void TableManager::generate_cp_move_table() {
    generate_move_table<Phase2Coord>("Corner Permutation", cp_move_table,
        [&](Phase2Coord& coord, uint16_t i) { coord.set_corner_permutation(i); },
        [&](Phase2Coord& coord) -> uint16_t { return coord.get_corner_permutation(); });
}

void TableManager::generate_udep_move_table() {
    generate_move_table<Phase2Coord>("UD Edge Permutation", udep_move_table,
        [&](Phase2Coord& coord, uint16_t i) { coord.set_ud_edge_permutation(i); },
        [&](Phase2Coord& coord) -> uint16_t { return coord.get_ud_edge_permutation(); });
}

void TableManager::generate_sep_move_table() {
    generate_move_table<Phase2Coord>("Slice Edge Permutation", sep_move_table,
        [&](Phase2Coord& coord, uint16_t i) { coord.set_slice_edge_permutation(i); },
        [&](Phase2Coord& coord) -> uint16_t { return coord.get_slice_edge_permutation(); });
}



uint8_t TableManager::get_phase1_pruning(const Phase1Coord& coord) const {
    return std::max({
        get_co_pruning(coord.get_corner_orientation()),
        get_eo_pruning(coord.get_edge_orientation()),
        get_uds_pruning(coord.get_ud_slice_position())
    });
}

uint8_t TableManager::get_phase2_pruning(const Phase2Coord& coord) const {
    return std::max({
        get_cp_pruning(coord.get_corner_permutation()),
        get_udep_pruning(coord.get_ud_edge_permutation()),
        get_sep_pruning(coord.get_slice_edge_permutation())
    });
}


} // namespace RubiksSolver
