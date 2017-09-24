all: run

build:
	g++ main.cpp util.cpp generators.cpp tests.cpp -O2 -std=c++11 -o main -lpthread

run: build
	timeout 2m ./main seq-read
	timeout 2m ./main seq-write
	timeout 30s ./main rnd-read
	timeout 30s ./main rnd-write
	timeout 2m ./main rnd-read-parallel
	timeout 2m ./main rnd-write-parallel
	timeout 2m ./main rnd-mixed-parallel