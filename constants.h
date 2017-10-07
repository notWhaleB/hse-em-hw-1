#ifndef EM_HW_1_CONSTANTS_H
#define EM_HW_1_CONSTANTS_H

#include <unistd.h>

typedef unsigned char byte_t;

const size_t KiB = 1024;
const size_t MiB = 1024 * KiB;
const size_t BUFFER_SZ = 4 * MiB;
const char *const TEST_FILE_PATH = "./tmp";

#ifdef __linux
const char *const DISK_MOUNT = "/dev/sda1";
#elif __APPLE__
const char *const DISK_MOUNT = "/dev/disk0";
#else
const char *const DISK_MOUNT = "";
#endif

const size_t N_TEST_BLOCKS = 0x100000;
const size_t N_RND_ITERS = 10000;
const size_t N_TESTS = 3;

// Set false to see only Avg and StdDev
const bool VERBOSE = false;

#endif //EM_HW_1_CONSTANTS_H
