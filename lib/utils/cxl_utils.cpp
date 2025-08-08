
#ifdef hardware_cxl
#include <iostream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <atomic>
#include <cstring>
#include <numaif.h>
void *_cxl_region_init(size_t region_size) {
    void* mapped = mmap(nullptr, region_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        return nullptr;
    }

    unsigned long nodemask = 1UL << 1; /* indicates physical memory node, might change */
    int mode = MPOL_BIND;
    unsigned long maxnode = sizeof(nodemask) * 8;
    if (mbind(mapped, region_size, mode, &nodemask, maxnode, 0) != 0) {
        perror("mbind");
    }

    return mapped;
}

void _cxl_region_free(void *region, size_t region_size) {
    munmap(region, region_size);
}
#endif