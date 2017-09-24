all: run

build:
	g++ main.cpp util.cpp generators.cpp tests.cpp -O2 -std=c++11 -o main -lpthread

run: build
	./main seq-read
	./main seq-write
	./main rnd-read
	./main rnd-write
	./main rnd-read-parallel
	./main rnd-write-parallel
	./main rnd-mixed-parallel