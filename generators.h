#ifndef EM_HW_1_GENERATORS_H
#define EM_HW_1_GENERATORS_H

#include "constants.h"

#include <cstdio>
#include <unistd.h>
#include <cassert>
#include <fcntl.h>
#include <string>
#include <random>

void sample_buffer_block(byte_t *, size_t);

const std::string allocate_sample_file(size_t, bool);

#endif //EM_HW_1_GENERATORS_H
