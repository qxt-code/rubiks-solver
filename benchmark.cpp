#include "cube.h"
#include "table_manager.h"
#include "solver.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>

struct BenchmarkResult {
    double solve_time_ms;
    int solution_length;
    std::string scramble;
    bool success;
};

double get_percentile(std::vector<double>& data, double percentile) {
    if (data.empty()) return 0.0;
    
    std::sort(data.begin(), data.end());
    size_t index = static_cast<size_t>((percentile / 100.0) * (data.size() - 1));
    
    if (index >= data.size()) {
        return data.back();
    }
    
    return data[index];
}

void print_statistics(const std::vector<BenchmarkResult>& results) {
    if (results.empty()) {
        std::cout << "No results to analyze." << std::endl;
        return;
    }
    
    std::vector<BenchmarkResult> successful_results;
    for (const auto& result : results) {
        if (result.success) {
            successful_results.push_back(result);
        }
    }
    
    if (successful_results.empty()) {
        std::cout << "No successful solves." << std::endl;
        return;
    }
    
    std::vector<double> solve_times;
    std::vector<double> solution_lengths;
    
    for (const auto& result : successful_results) {
        solve_times.push_back(result.solve_time_ms);
        solution_lengths.push_back(static_cast<double>(result.solution_length));
    }
    
    std::cout << "\n========== BENCHMARK RESULTS ==========" << std::endl;
    std::cout << "Total scrambles: " << results.size() << std::endl;
    std::cout << "Successful solves: " << successful_results.size() << std::endl;
    std::cout << "Success rate: " << std::fixed << std::setprecision(2) 
              << (100.0 * successful_results.size() / results.size()) << "%" << std::endl;
    
    std::cout << "\n--- SOLVE TIME STATISTICS (ms) ---" << std::endl;
    std::cout << "90th percentile: " << std::fixed << std::setprecision(2) 
              << get_percentile(solve_times, 90.0) << " ms" << std::endl;
    std::cout << "95th percentile: " << std::fixed << std::setprecision(2) 
              << get_percentile(solve_times, 95.0) << " ms" << std::endl;
    std::cout << "99th percentile: " << std::fixed << std::setprecision(2) 
              << get_percentile(solve_times, 99.0) << " ms" << std::endl;
    
    double sum_time = 0.0;
    for (double time : solve_times) {
        sum_time += time;
    }
    double avg_time = sum_time / solve_times.size();
    
    std::sort(solve_times.begin(), solve_times.end());
    double min_time = solve_times.front();
    double max_time = solve_times.back();
    double median_time = solve_times[solve_times.size() / 2];
    
    std::cout << "Average: " << std::fixed << std::setprecision(2) << avg_time << " ms" << std::endl;
    std::cout << "Median: " << std::fixed << std::setprecision(2) << median_time << " ms" << std::endl;
    std::cout << "Min: " << std::fixed << std::setprecision(2) << min_time << " ms" << std::endl;
    std::cout << "Max: " << std::fixed << std::setprecision(2) << max_time << " ms" << std::endl;
    
    std::cout << "\n--- SOLUTION LENGTH STATISTICS (moves) ---" << std::endl;
    std::cout << "90th percentile: " << std::fixed << std::setprecision(1) 
              << get_percentile(solution_lengths, 90.0) << " moves" << std::endl;
    std::cout << "95th percentile: " << std::fixed << std::setprecision(1) 
              << get_percentile(solution_lengths, 95.0) << " moves" << std::endl;
    std::cout << "99th percentile: " << std::fixed << std::setprecision(1) 
              << get_percentile(solution_lengths, 99.0) << " moves" << std::endl;
    
    double sum_length = 0.0;
    for (double length : solution_lengths) {
        sum_length += length;
    }
    double avg_length = sum_length / solution_lengths.size();
    
    std::sort(solution_lengths.begin(), solution_lengths.end());
    double min_length = solution_lengths.front();
    double max_length = solution_lengths.back();
    double median_length = solution_lengths[solution_lengths.size() / 2];
    
    std::cout << "Average: " << std::fixed << std::setprecision(1) << avg_length << " moves" << std::endl;
    std::cout << "Median: " << std::fixed << std::setprecision(1) << median_length << " moves" << std::endl;
    std::cout << "Min: " << std::fixed << std::setprecision(1) << min_length << " moves" << std::endl;
    std::cout << "Max: " << std::fixed << std::setprecision(1) << max_length << " moves" << std::endl;
    
    std::cout << "\n=======================================" << std::endl;
}

int main() {
    try {
        std::cout << "Initializing tables..." << std::endl;
        const auto& tables = RubiksSolver::TableManager::get_instance();
        std::cout << "Tables initialized successfully." << std::endl;
        
        RubiksSolver::Solver solver(tables);
        
        std::ifstream file("sc.txt");
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open sc.txt file" << std::endl;
            return 1;
        }
        
        std::vector<std::string> scrambles;
        std::string line;
        
        while (std::getline(file, line)) {
            if (!line.empty()) {
                scrambles.push_back(line);
            }
        }
        file.close();
        
        std::cout << "Loaded " << scrambles.size() << " scrambles from sc.txt" << std::endl;
        std::cout << "Starting benchmark...\n" << std::endl;
        
        std::vector<BenchmarkResult> results;
        results.reserve(scrambles.size());
        
        for (size_t i = 0; i < scrambles.size(); ++i) {
            const std::string& scramble = scrambles[i];
            std::cout << "Processing scramble " << (i + 1) << "/" << scrambles.size() << ": " << scramble << std::endl;
            
            BenchmarkResult result;
            result.scramble = scramble;
            result.success = false;
            
            try {
                auto cube = RubiksSolver::Cube::from_scramble(scramble);
                
                auto start_time = std::chrono::high_resolution_clock::now();
                std::vector<RubiksSolver::Move> solution = solver.solve(cube);
                auto end_time = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                result.solve_time_ms = duration.count();
                result.solution_length = static_cast<int>(solution.size());
                result.success = true;
                
                std::cout << "  ✓ Solved in " << std::fixed << std::setprecision(2) 
                          << result.solve_time_ms << " ms, " 
                          << result.solution_length << " moves" << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "  ✗ Failed: " << e.what() << std::endl;
                result.solve_time_ms = 0.0;
                result.solution_length = 0;
            }
            
            results.push_back(result);
        }
        
        print_statistics(results);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
