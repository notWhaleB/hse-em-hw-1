#include "tests.h"

long double Test::seq_read(bool quiet) {
    const std::string path = allocate_sample_file(_blockSz * _nBlocks, true);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _RD_POL);

    if (!quiet) std::cout << "Info: Running sequential read test..." << std::endl;
    long double readSpeed = _sequential_read(fd, 4 * MiB);
    close(fd);
    unlink(path.c_str());

    if (!quiet) std::cout << "Read speed: " << readSpeed << " MiB/s" << std::endl;

    return readSpeed;
}

long double Test::seq_write(bool quiet) {
    const std::string path = allocate_sample_file(0, false);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _WR_POL, _CREAT_ACCS_POL);

    if (!quiet) std::cout << "Info: Running sequential write test..." << std::endl;
    long double writeSpeed = _sequential_write(fd, 4 * MiB, (_nBlocks * _blockSz) / (4 * MiB));
    close(fd);
    unlink(path.c_str());

    if (!quiet) std::cout << "Write speed: " << writeSpeed << " MiB/s" << std::endl;

    return writeSpeed;
}

long double Test::rnd_read(bool quiet) {
    const std::string path = allocate_sample_file(_blockSz * _nBlocks, true);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _RD_POL);

    if (!quiet) std::cout << "Info: Running random read test..." << std::endl;
    long double readLatency = _random_read(fd, _blockSz, _nRndIter);
    close(fd);
    unlink(path.c_str());

    if (!quiet) std::cout << "Read latency: " << readLatency << " µs" << std::endl;

    return readLatency;
}

long double Test::rnd_write(bool quiet) {
    const std::string path = allocate_sample_file(0, false);
    if (_dropCache) drop_cache();

    int fd = open(path.c_str(), _WR_POL, _CREAT_ACCS_POL);

    if (!quiet) std::cout << "Info: Running random write test..." << std::endl;
    long double writeLatency = _random_write(fd, _blockSz, _blockSz * _nBlocks, _nRndIter);
    close(fd);
    unlink(path.c_str());

    if (!quiet) std::cout << "Write latency: " << writeLatency << " µs" << std::endl;

    return writeLatency;
}

long double Test::rnd_read_parallel(bool quiet) {
    return _rnd_read_parallel(this, quiet);
}

long double Test::_rnd_read_parallel(const Test *cls, bool quiet) {
    size_t nThreads = N_THREADS;
    std::vector<std::pair<int, const std::string>> files;
    size_t nBlocks = std::max(cls->_nBlocks / nThreads, (unsigned long)(0x40000));


    for (size_t i = 0; i != nThreads; ++i) {
        const std::string path = allocate_sample_file(cls->_blockSz * nBlocks, true);
        files.emplace_back(std::make_pair(open(path.c_str(), cls->_RD_POL), path));
    }
    if (cls->_dropCache) drop_cache();

    if (!quiet) std::cout << "Info: Running parallel random read test..." << std::endl;

    std::vector<std::future<long double>> futures;
    for (auto &i : files) {
        futures.emplace_back(std::async(std::launch::async, _random_read,
                                        i.first, cls->_blockSz, cls->_nRndIter));
    }

    long double totalLatency = 0.0;
    for (auto &i : futures) {
        totalLatency += i.get();
    }

    for (auto &i : files) {
        close(i.first);
        unlink(i.second.c_str());
    }

    long double readLatency = totalLatency / futures.size();
    if (!quiet) std::cout << "Read latency: " << readLatency << " µs" << std::endl;

    return readLatency;
}

long double Test::rnd_write_parallel(bool quiet) {
    return _rnd_write_parallel(this, quiet);
}

long double Test::_rnd_write_parallel(const Test *cls, bool quiet) {
    size_t nThreads = N_THREADS;
    std::vector<std::pair<int, const std::string>> files;
    size_t nBlocks = std::max(cls->_nBlocks / nThreads, (unsigned long)(0x40000));

    for (size_t i = 0; i != nThreads; ++i) {
        const std::string path = allocate_sample_file(0, false);
        files.emplace_back(std::make_pair(open(path.c_str(), cls->_WR_POL, cls->_CREAT_ACCS_POL), path));
    }
    if (cls->_dropCache) drop_cache();

    if (!quiet) std::cout << "Info: Running parallel random write test..." << std::endl;

    std::vector<std::future<long double>> futures;
    for (auto &i : files) {
        futures.emplace_back(std::async(std::launch::async, _random_write,
                                        i.first, cls->_blockSz, cls->_blockSz * nBlocks, cls->_nRndIter));
    }

    long double totalLatency = 0.0;
    for (auto &i : futures) {
        totalLatency += i.get();
    }

    for (auto &i : files) {
        close(i.first);
        unlink(i.second.c_str());
    }

    long double writeLatency = totalLatency / futures.size();
    if (!quiet) std::cout << "Write latency: " << writeLatency << " µs" << std::endl;

    return writeLatency;
}

long double Test::rnd_mixed_parallel(bool quiet) {
    if (_dropCache) drop_cache();
    bool tmp = _dropCache;
    _dropCache = false;

    std::future<long double> readFuture = std::async(std::launch::async, _rnd_read_parallel, this, true);
    std::future<long double> writeFuture = std::async(std::launch::async, _rnd_write_parallel, this, true);

    long double latency = (readFuture.get() + writeFuture.get()) / 2;

    _dropCache = tmp;

    if (!quiet) std::cout << "Mixed read+write latency: " << latency << " µs" << std::endl;

    return latency;
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
        assert(write(fd, buffer.get(), blockSz) >= 0);
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
        assert(read(fd, buffer.get(), bufferSz) >= 0);
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
        assert(write(fd, buffer.get(), blockSz) >= 0);
        fsync(fd);
    };

    for (size_t i = 0; i != nIter; ++i) {
        nextPos = rand_int(0, static_cast<int>(maxFileSz - blockSz));
        offset = nextPos - lseek(fd, 0, SEEK_CUR);
        totalSeconds += measure_time(func);
    }

    return 1e6 * totalSeconds / nIter;
}
