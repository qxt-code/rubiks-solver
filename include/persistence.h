#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <concepts>

template<typename T>
concept OneDimensionalArray = requires(T t) {
    typename T::value_type;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.data() } -> std::convertible_to<typename T::value_type*>;
} && !requires(T t) {
    typename T::value_type::value_type;
};

template<typename T>
concept TwoDimensionalArray = requires(T t) {
    typename T::value_type;
    typename T::value_type::value_type;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t[0].data() } -> std::convertible_to<typename T::value_type::value_type*>;
};

template<typename ArrayType>
void save_array_binary(const ArrayType& arr, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    if constexpr (OneDimensionalArray<ArrayType>) {
        // 一维数组处理
        using ValueType = typename ArrayType::value_type;
        constexpr size_t size = std::tuple_size_v<ArrayType>;
        file.write(reinterpret_cast<const char*>(arr.data()), sizeof(ValueType) * size);
        std::cout << "1D Array saved to " << filename << " (size: " << size << ")" << std::endl;
    }
    else if constexpr (TwoDimensionalArray<ArrayType>) {
        // 二维数组处理
        using ValueType = typename ArrayType::value_type::value_type;
        constexpr size_t rows = std::tuple_size_v<ArrayType>;
        constexpr size_t cols = std::tuple_size_v<typename ArrayType::value_type>;
        
        for (const auto& row : arr) {
            file.write(reinterpret_cast<const char*>(row.data()), sizeof(ValueType) * cols);
        }
        std::cout << "2D Array saved to " << filename << " (size: " << rows << "x" << cols << ")" << std::endl;
    }
    else {
        std::cerr << "Unsupported array type for " << filename << std::endl;
    }
    file.close();
}

template<typename ArrayType>
bool load_array_binary(ArrayType& arr, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }

    if constexpr (OneDimensionalArray<ArrayType>) {
        // 一维数组处理
        using ValueType = typename ArrayType::value_type;
        constexpr size_t size = std::tuple_size_v<ArrayType>;
        file.read(reinterpret_cast<char*>(arr.data()), sizeof(ValueType) * size);
        std::cout << "1D Array loaded from " << filename << " (size: " << size << ")" << std::endl;
    }
    else if constexpr (TwoDimensionalArray<ArrayType>) {
        // 二维数组处理
        using ValueType = typename ArrayType::value_type::value_type;
        constexpr size_t rows = std::tuple_size_v<ArrayType>;
        constexpr size_t cols = std::tuple_size_v<typename ArrayType::value_type>;
        
        for (auto& row : arr) {
            file.read(reinterpret_cast<char*>(row.data()), sizeof(ValueType) * cols);
        }
        std::cout << "2D Array loaded from " << filename << " (size: " << rows << "x" << cols << ")" << std::endl;
    }
    else {
        std::cerr << "Unsupported array type for " << filename << std::endl;
        return false;
    }
    
    file.close();
    return true;
}

template<typename T>
void save_map_binary(const std::unordered_map<uint64_t, std::vector<T>>& map, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    uint64_t map_size = map.size();
    out.write(reinterpret_cast<const char*>(&map_size), sizeof(map_size));

    for (const auto& pair : map) {
        // 写入 key
        out.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
        
        // 写入 vector 的大小和数据
        uint64_t vec_size = pair.second.size();
        out.write(reinterpret_cast<const char*>(&vec_size), sizeof(vec_size));
        out.write(reinterpret_cast<const char*>(pair.second.data()), vec_size * sizeof(T));
    }
    std::cout << "Map saved to " << filename << " (size: " << map_size << ")" << std::endl;
}

template<typename T>
bool load_map_binary(std::unordered_map<uint64_t, std::vector<T>>& map, const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
     if (!in.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }

    uint64_t map_size;
    in.read(reinterpret_cast<char*>(&map_size), sizeof(map_size));
    
    map.reserve(map_size); // 预分配空间，提高性能

    for (uint64_t i = 0; i < map_size; ++i) {
        uint64_t key;
        in.read(reinterpret_cast<char*>(&key), sizeof(key));

        uint64_t vec_size;
        in.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));

        std::vector<T> vec(vec_size);
        in.read(reinterpret_cast<char*>(vec.data()), vec_size * sizeof(T));

        map[key] = std::move(vec);
    }
    std::cout << "Map loaded from " << filename << " (size: " << map_size << ")" << std::endl;
    return true;
}

inline bool create_directory(const std::string& path) {
    try {
        if (std::filesystem::create_directory(path)) {
            std::cout << "成功创建目录: " << path << std::endl;
            return true;
        } else if (std::filesystem::exists(path)) {
            std::cout << "目录已存在: " << path << std::endl;
            return true;
        } else {
            std::cout << "创建目录失败: " << path << std::endl;
            return false;
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "文件系统错误: " << ex.what() << std::endl;
        return false;
    }
}

#endif // PERSISTENCE_H