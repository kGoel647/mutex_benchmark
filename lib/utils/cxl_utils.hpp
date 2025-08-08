// Use -Dcxl to compile for CXL.


#ifndef __CXL_UTILS_HPP
#define __CXL_UTILS_HPP

#pragma once

#include <stddef.h>

// #ifndef __cpp_lib_hardware_interference_size
// namespace std {
//     const size_t hardware_destructive_interference_size = 64;
// }
// #endif


// defined via macro so they can be changed for the actual hardware
#ifdef cxl
    #include "stddef.h"
    extern "C" { 
        void *emucxl_alloc(size_t size, int node);
        void  emucxl_free(void *ptr, size_t size);
        void emucxl_init();
        void emucxl_exit();
    }
    #define ALLOCATE(size) emucxl_alloc(size, 1)
    #define FREE(ptr, size) emucxl_free(ptr, size)

    #define cxl_mutex_benchmark_init() emucxl_init()
    #define cxl_mutex_benchmark_exit() emucxl_exit()
#elif defined(hardware_cxl)
    #include <iostream>
    #include <thread>
    #include <chrono>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <atomic>
    #include <cstring>
    #include <numaif.h>
    inline void *_cxl_region_init(size_t region_size) {
        void* mapped = mmap(nullptr, region_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (mapped == MAP_FAILED) {
            perror("mmap");
            return nullptr;
        }

        unsigned long nodemask = 1UL; /* indicates physical memory node, might change */
        int mode = MPOL_BIND;
        unsigned long maxnode = sizeof(nodemask) * 8;
        // printf("mbind(%p, %ld, 0x%X, &0x%X, %ld, 0)\n", mapped, region_size, mode, nodemask, maxnode);
        if (mbind(mapped, region_size, mode, &nodemask, maxnode, 0) != 0) {
            perror("mbind");
        }

        return mapped;
    }

    inline void _cxl_region_free(void *region, size_t region_size) {
        munmap(region, region_size);
    }

    #define ALLOCATE(size) _cxl_region_init(size)
    #define FREE(ptr, size) _cxl_region_free(ptr, size)

    #define cxl_mutex_benchmark_init()
    #define cxl_mutex_benchmark_exit()
#else
    #define ALLOCATE(size) malloc(size)
    #define FREE(ptr, size) free(ptr); (void)(size) //
    
    #define cxl_mutex_benchmark_init()
    #define cxl_mutex_benchmark_exit()
#endif // CXL

#if defined(__x86_64)
    //#define Fence() __asm__ __volatile__ ( "mfence" )
    #define FENCE() __asm__ __volatile__ ( "lock; addq $0,128(%%rsp);" ::: "cc" )
#elif defined(__i386)
    #define FENCE() __asm__ __volatile__ ( "lock; addl $0,128(%%esp);" ::: "cc" )
#elif defined(__ARM_ARCH)
    #define FENCE() __asm__ __volatile__ ( "DMB ISH" ::: )
#else
    #error unsupported architecture
#endif

#endif // __CXL_UTILS_HPP