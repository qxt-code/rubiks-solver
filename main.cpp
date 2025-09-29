#include "cube.h"
#include "table_manager.h"
#include "solver.h"
#include <coordinate.h>
#include <iostream>

int main() {
    try {
        // 初始化所有表格
        // 第一次运行时需要生成，会比较慢
        const auto& tables = RubiksSolver::TableManager::get_instance();

        RubiksSolver::Solver solver(tables);

        std::string scramble;
        
        while (std::cin.good()) {
            std::cout << "Enter scramble sequence (or 'exit' to quit): ";
            std::getline(std::cin, scramble);
            if (scramble == "exit") {
                break;
            }
            if (scramble.empty()) {
                std::cout << "No scramble entered, please try again." << std::endl;
                continue;
            }
            try
            {
                auto cube = RubiksSolver::Cube::from_scramble(scramble);
                std::cout << "Initial Cube State:\n" << cube << std::endl;
                
                std::cout << "Solving..." << std::endl;
                std::vector<RubiksSolver::Move> solution = solver.solve(cube);
                
                std::cout << "Solution found (" << solution.size() << " moves):" << std::endl;
                for (const auto& move : solution) {
                    std::cout << move << " ";
                }
                std::cout << std::endl;
            } catch(const std::invalid_argument& e) {
                std::cerr << e.what() << '\n';
                std::cout << "Please enter a valid scramble sequence." << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "An error occurred: " << e.what() << std::endl;
            }
            
        }

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
        return 1;
    }

    return 0;
}