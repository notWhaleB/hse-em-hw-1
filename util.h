#ifndef EM_HW_1_UTIL_H
#define EM_HW_1_UTIL_H

#include "constants.h"

#include <cstdio>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <chrono>
#include <random>

#ifdef __APPLE__
#include <sys/disk.h>
#endif

const size_t disk_block_size();

void drop_cache();

int rand_int(int, int);

template <typename Func>
long double measure_time(Func func) {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    func();
    std::chrono::duration<long double> elapsedTime = std::chrono::system_clock::now() - start;
    return elapsedTime.count();
}

#endif //EM_HW_1_UTIL_H
