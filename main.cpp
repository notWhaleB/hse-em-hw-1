#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <random>
#include <time.h>
#include <sstream>
#include <chrono>
#include <memory>
#include <unistd.h>
#include <sys/stat.h>

typedef unsigned char byte_t;

const size_t KiB = 1024;
const size_t MiB = 1024 * KiB;
const size_t BUFFER_SZ = 256;

void drop_cache() {
#ifdef __linux
    const char *cmd = "sync && echo 3 > /proc/sys/vm/drop_caches";
#elif __APPLE__
    const char *cmd = "sync && purge";
#else
    #warning "Cache clearing not supported for target OS."
    const char *cmd = "";
    return;
#endif

    FILE *file;
    char buffer[BUFFER_SZ];

    file = popen(cmd, "r");

    while (fread(buffer, sizeof(char), sizeof(buffer), file)) {
#ifdef DEBUG
        std::cout << buffer;
#endif
    }

    pclose(file);
}

inline int rand_int(int min, int max) {
    return static_cast<int>(random() % (max - min) + min);
}

template <typename Func>
long double measure_time(Func func) {
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    func();
    std::chrono::duration<long double> elapsedTime = std::chrono::system_clock::now() - start;
    return elapsedTime.count();
}

void sample_buffer_block(byte_t *buffer, size_t blockSz) {
    static const byte_t sample[] = {0x8b, 0xad, 0xf0, 0x0d};
    static const size_t sampleSz = sizeof(sample);

    for (size_t i = 0; i != blockSz; ++i) {
        buffer[i] = sample[i % sampleSz];
    }
}

namespace Test {
    long double seq_read(int fd, size_t bufferSz) {
        std::unique_ptr<byte_t[]> buffer(new byte_t[bufferSz]);
        size_t totalSz = 0;
        ssize_t readSz = 0;
        long double totalSeconds = 0.0;

        auto func = [&readSz, fd, &buffer, bufferSz] () {
            readSz = read(fd, buffer.get(), bufferSz);
        };

        while (true) {
            totalSeconds += measure_time(func);

            if (readSz <= 0) break;
            totalSz += readSz;
        }

        return static_cast<long double>(totalSz) / (MiB * totalSeconds);
    }

    long double seq_write(int fd, size_t blockSz, size_t nBlocks) {
        std::unique_ptr<byte_t[]> buffer(new byte_t[blockSz]);
        long double totalSeconds = 0.0;

        auto func = [fd, &buffer, blockSz] () {
            write(fd, buffer.get(), blockSz);
        };

        for (size_t i = 0; i != nBlocks; ++i) {
            sample_buffer_block(buffer.get(), blockSz);
            totalSeconds += measure_time(func);
        }

        return static_cast<long double>(blockSz * nBlocks) / (MiB * totalSeconds);
    }

//    double rnd_write(int fd, size_t blockSz, size_t maxFileSz, size_t nIter) {
//        if (blockSz > maxFileSz) {
//            maxFileSz = blockSz;
//        }
//        std::unique_ptr<byte_t[]> buffer(new byte_t[blockSz]);
//        sample_buffer_block(buffer.get(), blockSz);
//
//        double totalSeconds = 0.0;
//        std::chrono::time_point<std::chrono::system_clock> start;
//
//        for (size_t i = 0; i != nIter; ++i) {
//            off_t nextPos = rand_int(0, static_cast<int>(maxFileSz - blockSz));
//            off_t offset = nextPos - lseek(fd, 0, SEEK_CUR);
//
//            start = std::chrono::system_clock::now();
//            {
//                lseek(fd, offset, SEEK_CUR);
//                write(fd, buffer.get(), blockSz);
//            }
//            totalSeconds += (std::chrono::system_clock::now() - start).count();
//        }
//
//        return static_cast<double>(blockSz * nIter) / (MiB * totalSeconds);
//    }


}

int main(int argc, char** argv) {
    std::string mode;
    if (argc != 2) {
        std::cout << "Warning: invalid args, run with additional argument for mode." << std::endl;
        std::cout << "Please choose (seq-read, seq-write): ";
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
    } else {
        drop_cache();
        std::cout << "Info: Cache has been dropped." << std::endl;
    }

    srandom(static_cast<unsigned int>(time(nullptr)));

    if (mode == "seq-read") {
        int fd = open("./tmp.bin", O_RDONLY | O_SYNC);

        std::cout << "Info: Running sequential read test..." << std::endl;
        long double readSpeed = Test::seq_read(fd, 1 * MiB);
        std::cout << "Read speed: " << readSpeed << " MiB/s" << std::endl;

        close(fd);
    } else if (mode == "seq-write") {
        int fd = open("./tmp.bin", O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);

        std::cout << "Info: Running sequential write test..." << std::endl;
        long double writeSpeed = Test::seq_write(fd, 1 * MiB, 512);
        std::cout << writeSpeed << " MiB/s" << std::endl;

        close(fd);
    } else {
        std::cout << "Incorrect mode " << mode << "." << std::endl;
    }

    return 0;
}
