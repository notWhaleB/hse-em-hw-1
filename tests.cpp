#include "tests.h"

void Test::seq_read() {
    const std::string path = allocate_sample_file(_blockSz * _nBlocks, true);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _RD_POL);

    std::cout << "Info: Running sequential read test..." << std::endl;
    long double readSpeed = _sequential_read(fd, 4 * MiB);
    close(fd);
    unlink(path.c_str());

    std::cout << "Read speed: " << readSpeed << " MiB/s" << std::endl;
}

void Test::seq_write() {
    const std::string path = allocate_sample_file(0, false);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _WR_POL, _CREAT_ACCS_POL);

    std::cout << "Info: Running sequential write test..." << std::endl;
    long double writeSpeed = _sequential_write(fd, 4 * MiB, (_nBlocks * _blockSz) / (4 * MiB));
    close(fd);
    unlink(path.c_str());

    std::cout << "Write speed: " << writeSpeed << " MiB/s" << std::endl;
}

void Test::rnd_read() {
    const std::string path = allocate_sample_file(_blockSz * _nBlocks, true);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _RD_POL);

    std::cout << "Info: Running random read test..." << std::endl;
    long double readLatency = _random_read(fd, _blockSz, _nRndIter);
    close(fd);
    unlink(path.c_str());

    std::cout << "Read latency: " << readLatency << " µs" << std::endl;
}

void Test::rnd_write() {
    const std::string path = allocate_sample_file(0, false);
    if (_dropCache) drop_cache();

    int fd = open(TEST_FILE_PATH, _WR_POL, _CREAT_ACCS_POL);

    std::cout << "Info: Running random write test..." << std::endl;
    long double writeLatency = _random_write(fd, _blockSz, _blockSz * _nBlocks, _nRndIter);
    close(fd);
    unlink(path.c_str());

    std::cout << "Write latency: " << writeLatency << " µs" << std::endl;
}

void Test::rnd_read_parallel() {
    size_t nThreads = std::thread::hardware_concurrency();
    std::vector<std::pair<int, const std::string>> files;
    size_t nBlocks = std::max(_nBlocks / nThreads, (unsigned long)(0x40000));


    for (size_t i = 0; i != nThreads; ++i) {
        const std::string path = allocate_sample_file(_blockSz * nBlocks, true);
        files.emplace_back(std::make_pair(open(path.c_str(), _RD_POL), path));
    }
    if (_dropCache) drop_cache();

    std::cout << "Info: Running parallel random read test..." << std::endl;

    std::vector<std::future<long double>> futures;
    for (auto &i : files) {
        futures.emplace_back(std::async(_random_read, i.first, _blockSz, _nRndIter));
    }

    long double totalLatency = 0.0;
    for (auto &i : futures) {
        totalLatency += i.get();
    }

    for (auto &i : files) {
        close(i.first);
        unlink(i.second.c_str());
    }

    std::cout << "Read latency: " << totalLatency / futures.size() << " µs" << std::endl;
}

long double Test::_sequential_read(int fd, size_t bufferSz) {
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

long double Test::_sequential_write(int fd, size_t blockSz, size_t nBlocks) {
    std::unique_ptr<byte_t[]> buffer(new byte_t[blockSz]);
    long double totalSeconds = 0.0;

    auto func = [fd, &buffer, blockSz] () {
        write(fd, buffer.get(), blockSz);
        fsync(fd);
    };

    for (size_t i = 0; i != nBlocks; ++i) {
        sample_buffer_block(buffer.get(), blockSz);
        totalSeconds += measure_time(func);
    }

    return static_cast<long double>(blockSz * nBlocks) / (MiB * totalSeconds);
}

long double Test::_random_read(int fd, size_t bufferSz, size_t nIter) {
    std::unique_ptr<byte_t[]> buffer(new byte_t[bufferSz]);
    long double totalSeconds = 0.0;
    std::chrono::time_point<std::chrono::system_clock> start;
    off_t nextPos = 0, offset = 0;
    auto fileSz = static_cast<size_t>(lseek(fd, 0, SEEK_END));

    auto func = [fd, &buffer, &offset, bufferSz] () {
        lseek(fd, offset, SEEK_CUR);
        read(fd, buffer.get(), bufferSz);
    };

    for (size_t i = 0; i != nIter; ++i) {
        nextPos = rand_int(0, static_cast<int>(fileSz - bufferSz));
        offset = nextPos - lseek(fd, 0, SEEK_CUR);
        totalSeconds += measure_time(func);
    }

    return 1e6 * totalSeconds / nIter;
}

long double Test::_random_write(int fd, size_t blockSz, size_t maxFileSz, size_t nIter) {
    if (blockSz > maxFileSz) {
        maxFileSz = blockSz;
    }
    std::unique_ptr<byte_t[]> buffer(new byte_t[blockSz]);
    sample_buffer_block(buffer.get(), blockSz);

    long double totalSeconds = 0.0;
    std::chrono::time_point<std::chrono::system_clock> start;
    off_t nextPos = 0, offset = 0;

    auto func = [fd, &buffer, &offset, blockSz] () {
        lseek(fd, offset, SEEK_CUR);
        write(fd, buffer.get(), blockSz);
        fsync(fd);
    };

    for (size_t i = 0; i != nIter; ++i) {
        nextPos = rand_int(0, static_cast<int>(maxFileSz - blockSz));
        offset = nextPos - lseek(fd, 0, SEEK_CUR);
        totalSeconds += measure_time(func);
    }

    return 1e6 * totalSeconds / nIter;
}