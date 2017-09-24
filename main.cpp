#include "main.h"

int main(int argc, char** argv) {
    std::string mode;
    if (argc != 2) {
        std::cout << "Warning: invalid args, run with additional argument for mode." << std::endl;
        std::cout << "Please choose (seq-read, seq-write, rnd-read, rnd-write): ";
        std::cin >> mode;
    } else {
        mode = argv[1];
    }

    bool isRoot = (getuid() == 0);

    if (!isRoot) {
        std::cout << "Warning: must be run as root for using drop_cache." << std::endl;
        std::cout << "drop_cache feature will be disabled, continue anyway? [y/n]" << std::endl;
        std::string ans;
        std::cin >> ans;

        if (ans != "y") return 0;
    }

    srandom(static_cast<unsigned int>(time(nullptr)));

    Test tests(disk_block_size(), N_TEST_BLOCKS, N_RND_ITERS, BUFFER_SZ, isRoot);

    if (mode == "seq-read") {
        tests.seq_read();
    } else if (mode == "seq-write") {
        tests.seq_write();
    } else if (mode == "rnd-read") {
        tests.rnd_read();
    } else if (mode == "rnd-write") {
        tests.rnd_write();
    } else if (mode == "rnd-read-parallel") {
        tests.rnd_read_parallel();
    } else if (mode == "rnd-write-parallel") {
        tests.rnd_write_parallel();
    } else if (mode == "rnd-mixed-parallel") {
        tests.rnd_mixed_parallel();
    } else {
        std::cout << "Incorrect mode " << mode << "." << std::endl;
    }

    return 0;
}
