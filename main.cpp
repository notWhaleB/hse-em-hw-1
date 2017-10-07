#include "main.h"

int main(int argc, char** argv) {
    std::string mode;
    if (argc != 2) {
        std::cout << "Warning: invalid args, run with additional argument for mode." << std::endl;
        std::cout << "Please choose (seq-read, seq-write, rnd-read, rnd-write, ";
        std::cout << "rnd-read-parallel, rnd-write-parallel, rnd-mixed-parallel): ";
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

    std::cout << "Running tests..." << std::endl;
    if (mode == "seq-read") {
        tests.several_run([&tests] () -> long double {
            return tests.seq_read(!VERBOSE);
        }, N_TESTS, "Avg read speed: %.2Lf MiB/s, StdDev: %.2Lf MiB/s.\n");
    } else if (mode == "seq-write") {
        tests.several_run([&tests] () -> long double {
            return tests.seq_write(!VERBOSE);
        }, N_TESTS, "Avg write speed: %.2Lf MiB/s, StdDev: %.2Lf MiB/s.\n");
    } else if (mode == "rnd-read") {
        tests.several_run([&tests] () -> long double {
            return tests.rnd_read(!VERBOSE);
        }, N_TESTS, "Avg read latency: %.2Lf µs, StdDev: %.2Lf µs.\n");
    } else if (mode == "rnd-write") {
        tests.several_run([&tests] () -> long double {
            return tests.rnd_write(!VERBOSE);
        }, N_TESTS, "Avg write latency: %.2Lf µs, StdDev: %.2Lf µs.\n");
    } else if (mode == "rnd-read-parallel") {
        tests.several_run([&tests] () -> long double {
            return tests.rnd_read_parallel(!VERBOSE);
        }, N_TESTS, "Avg read latency: %.2Lf µs, StdDev: %.2Lf µs.\n");
    } else if (mode == "rnd-write-parallel") {
        tests.several_run([&tests] () -> long double {
            return tests.rnd_write_parallel(!VERBOSE);
        }, N_TESTS, "Avg write latency: %.2Lf µs, StdDev: %.2Lf µs.\n");
    } else if (mode == "rnd-mixed-parallel") {
        tests.several_run([&tests] () -> long double {
            return tests.rnd_mixed_parallel(!VERBOSE);
        }, N_TESTS, "Avg read+write latency: %.2Lf µs, StdDev: %.2Lf µs.\n");
    } else {
        std::cout << "Incorrect mode " << mode << "." << std::endl;
    }

    return 0;
}
