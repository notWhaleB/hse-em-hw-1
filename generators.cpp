#include "generators.h"

void sample_buffer_block(byte_t *buffer, size_t blockSz) {
    static const byte_t sample[] = {0x8b, 0xad, 0xf0, 0x0d};
    static const size_t sampleSz = sizeof(sample);

    for (size_t i = 0; i != blockSz; ++i) {
        buffer[i] = sample[i % sampleSz];
    }
}

const std::string allocate_sample_file(size_t sz, bool create) {
    std::string path = "./tmp_" + std::to_string(random());

    if (!create) return path;

    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR);
    lseek(fd, static_cast<off_t>(sz), SEEK_SET);
    write(fd, "1", 1);
    fsync(fd);
    close(fd);

    return path;
}
