#include "util.h"

const size_t disk_block_size() {
#if defined(__linux) || defined(__APPLE__)
    #ifdef __linux
    const unsigned long _IOCTL = BLKPBSZGET;
    #else
    const unsigned long _IOCTL = DKIOCGETPHYSICALBLOCKSIZE;
    #endif

    uint32_t physBlockSz;
    int dev = open(DISK_MOUNT, O_RDONLY);

    if (ioctl(dev, _IOCTL, &physBlockSz) == -1) {
        printf("Warning: Can't get physical block size on device %s, set to 512.\n", DISK_MOUNT);
        physBlockSz = 512;
    }
    size_t blockSz = physBlockSz;

    close(dev);
#else
    #warning "Unsupported OS, disk block size set to 512 bytes."
    size_t blockSz = 512;
#endif

    return blockSz;
}

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
    char buffer[256];

    file = popen(cmd, "r");

    while (fread(buffer, sizeof(char), sizeof(buffer), file)) {
#ifdef DEBUG
        std::cout << buffer;
#endif
    }

    if (VERBOSE) printf("Info: Cache has been dropped.\n");

    pclose(file);
}

int rand_int(int min, int max) {
    return static_cast<int>(random() % (max - min) + min);
}
