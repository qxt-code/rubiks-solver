.PHONY: all run

all:
	cmake -B build
	cmake --build build -j$(nproc)

run: all
	./build/rubiks_solver

bench: all
	./build/benchmark