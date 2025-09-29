cmake_minimum_required(VERSION 3.15)

project(RubiksSolver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

if(NOT USE_ENHANCED_HEURISTIC)
    set(USE_ENHANCED_HEURISTIC OFF CACHE BOOL "Enable the enhanced search heuristic" FORCE)
endif()

file(GLOB SRC_FILES "src/*.cpp")

add_executable(rubiks_solver main.cpp ${SRC_FILES})
add_executable(benchmark benchmark.cpp ${SRC_FILES})

target_include_directories(rubiks_solver PUBLIC include)
target_include_directories(benchmark PUBLIC include)

target_compile_definitions(rubiks_solver PRIVATE
    "$<$<BOOL:${USE_ENHANCED_HEURISTIC}>:USE_ENHANCED_HEURISTIC>"
)

target_compile_definitions(benchmark PRIVATE
    "$<$<BOOL:${USE_ENHANCED_HEURISTIC}>:USE_ENHANCED_HEURISTIC>"
)

target_compile_options(rubiks_solver PRIVATE
    $<$<CONFIG:Debug>:-O0 -g -Wall -Wextra>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

target_compile_options(benchmark PRIVATE
    $<$<CONFIG:Debug>:-O0 -g -Wall -Wextra>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
