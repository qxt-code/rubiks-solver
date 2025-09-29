# Rubik's Cube Solver

This project is a 3x3x3 Rubik's Cube solver written in C++. It utilizes a two-phase IDA* (Iterative Deepening A*) search algorithm, based on Kociemba's algorithm, heavily optimized with pre-computed move tables, pruning tables, and endgame databases to find near-optimal solutions in milliseconds.

## 📊 Performance

Benchmark results from solving 1000 random scrambles:

- **Success Rate:** 99.6%
- **Average Solve Time:** 4.68 ms
- **99th Percentile Time:** 26.61 ms
- **Average Solution Length:** 25.2 moves
- **Peak Memory Usage:** ~170.2 MB (as measured by Valgrind Massif)

| Metric (Time)     | Value      |
| ----------------- | ---------- |
| Average           | 4.68 ms    |
| Median            | 2.66 ms    |
| 90th Percentile   | 10.11 ms   |
| 95th Percentile   | 15.24 ms   |
| 99th Percentile   | 26.61 ms   |
| Max               | 127.42 ms  |

| Metric (Moves)    | Value      |
| ----------------- | ---------- |
| Average           | 25.2 moves |
| Median            | 25.0 moves |
| 99th Percentile   | 29.0 moves |

### Memory Usage Profile (Valgrind Massif)

```text
    MB
170.2^                                                                       :
     | #:@@::::::::@::::::::::::::::::::@:::::::@:::::::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
     | #:@ ::::::::@::::::: :::: :::::::@:::::::@::: :::@:::::@::::::@::::::@:
   0 +----------------------------------------------------------------------->Gi
     0                                                                   88.34
```

## 🛠️ Building the Project

The project uses CMake for building.

### Prerequisites

- A C++ compiler that supports C++20
- CMake (version 3.15 or higher)

### Build Steps

1. **Create a build directory:**

    ```bash
    mkdir build
    ```

2. **Run CMake and build the project:**

    ```bash
    cmake -B build
    cmake --build build
    ```

    This will generate two executables in the `build` directory: `rubiks_solver` and `benchmark`.

### Enhanced Heuristic Option

For shorter solutions at the cost of increased solving time, you can enable the enhanced heuristic:

```bash
cmake -B build -DUSE_ENHANCED_HEURISTIC=ON
cmake --build build
```

This option provides more accurate heuristic estimates, resulting in shorter solution paths but requiring more computation time per solve.

## 🚀 Usage

### Solving a Single Scramble

The `rubiks_solver` executable runs in a loop, allowing you to enter scramble strings repeatedly.

**How to Run:**

```bash
./build/rubiks_solver
```

After launching, the program will take a moment to generate or load the pre-computed tables, and then it will prompt you to enter a scramble sequence.

**Example Interaction:**

```text
Initializing tables...
Tables initialized successfully.
Enter scramble sequence (or 'exit' to quit): R U R' U' R' F R2 U' R' U' R U R' F'
Solving...
Total solution (16 moves): D' L2 U' F2 D' B2 U L2 D' F2 U' S' F2 S U' F2 
Enter scramble sequence (or 'exit' to quit): exit
```

### Running Benchmarks

The `benchmark` executable runs a series of tests on scrambles provided in a text file.

1. **Create a scramble file:**

    Create a file named `sc.txt` in the project root directory. Each line should contain one scramble sequence. A `sc.txt` file with 1000 scrambles is already included in the repository.

    **Example `sc.txt`:**

    ```text
    R U R' U'
    F B L R U' D'
    R2 U2 F2 B2 L2 D2
    ...
    ```

2. **Run the benchmark:**

    Execute the `benchmark` program from the `build` directory.

    ```bash
    ./build/benchmark
    ```

    The program will process each scramble in `sc.txt` and print detailed statistics upon completion.
