#ifndef EM_HW_1_TESTS_H
#define EM_HW_1_TESTS_H

#include "constants.h"
#include "util.h"
#include "generators.h"

#include <unistd.h>
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <future>

class Test {
public:
    Test(size_t blockSz, size_t nBlocks, size_t nRndIter, size_t bufferSz, bool dropCache)
        : _blockSz(blockSz), _nBlocks(nBlocks), _nRndIter(nRndIter),
          _bufferSz(bufferSz), _dropCache(dropCache) {};

    void seq_read();
    void seq_write();
    void rnd_read();
    void rnd_write();
    void rnd_read_parallel();

    ~Test() = default;

private:
    const int _RD_POL = O_RDONLY | O_SYNC;
    const int _WR_POL = O_WRONLY | O_CREAT | O_TRUNC | O_SYNC;
    const int _CREAT_ACCS_POL = S_IRUSR | S_IWUSR;

    size_t _blockSz;
    size_t _nBlocks;
    size_t _nRndIter;
    size_t _bufferSz;
    bool _dropCache;

    static long double _sequential_read(int, size_t);
    static long double _sequential_write(int, size_t, size_t);
    static long double _random_read(int, size_t, size_t);
    static long double _random_write(int, size_t, size_t, size_t);
};

#endif //EM_HW_1_TESTS_H
