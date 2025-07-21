// Use -Dcxl to compile for CXL.


#ifndef __CXL_UTILS_HPP
#define __CXL_UTILS_HPP

#pragma once


// defined via macro so they can be changed for the actual hardware
#ifdef cxl
    #include <stddef.h>
    extern "C" { 
        void *emucxl_alloc(size_t size, int node);
        void  emucxl_free(void *ptr, size_t size);
    }
    #define ALLOCATE(size) emucxl_alloc(size, 1)
    #define FREE(ptr, size) emucxl_free(ptr, size)
#else
    #define ALLOCATE(size) malloc(size)
    #define FREE(ptr, size) free(ptr); (void)(size) //
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