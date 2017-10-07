#ifndef EM_HW_1_TESTS_H
#define EM_HW_1_TESTS_H

#include "constants.h"
#include "util.h"
#include "generators.h"

#include <unistd.h>
#include <memory>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include <math.h>

class Test {
public:
    Test(size_t blockSz, size_t nBlocks, size_t nRndIter, size_t bufferSz, bool dropCache)
        : _blockSz(blockSz), _nBlocks(nBlocks), _nRndIter(nRndIter),
          _bufferSz(bufferSz), _dropCache(dropCache) {};

    long double seq_read(bool);
    long double seq_write(bool);
    long double rnd_read(bool);
    long double rnd_write(bool);
    long double rnd_read_parallel(bool);
    long double rnd_write_parallel(bool);
    long double rnd_mixed_parallel(bool);

    template <typename Func>
    static void several_run(Func func, size_t num, const char *logTemplate) {
        assert(num > 0);

        std::vector<long double> results, sqDiff;

        for (size_t i = 0; i != num; ++i) {
            long double res = func();
            results.push_back(res);
        }

        long double avg = std::accumulate(results.begin(), results.end(), 0.0) / num;

        for (auto &res : results) {
            sqDiff.push_back(std::pow(res - avg, 2.0));
        }

        long double stdDev = std::sqrt(std::accumulate(sqDiff.begin(), sqDiff.end(), 0.0) / num);

        printf(logTemplate, avg, stdDev);
    }

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

    static long double _rnd_read_parallel(const Test *, bool);
    static long double _rnd_write_parallel(const Test *, bool);
};

#endif //EM_HW_1_TESTS_H
