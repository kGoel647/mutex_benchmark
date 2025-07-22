
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef REGION_SIZE
    #define REGION_SIZE 0x10000
#endif

#ifndef ITERATIONS
    #define ITERATIONS 1000
#endif

#define XSTR(x) #x
#define STR(x) XSTR(x)
#define CLOCK_MONOTONIC 1 // Might be platform-dependent

#if defined(cxl)
    #include "emucxl/src/emucxl_lib.h"
    #define ALLOC(size, node) emucxl_alloc(size, node)
    #define FREE(ptr, size), emucxl_free(ptr, size)
    #define TEST_ENVIRONMENT "'cxl'"
#else
    #define ALLOC(size, node) malloc(size); (void)(node)
    #define FREE(ptr, size) free(ptr); (void)(size)
    #define TEST_ENVIRONMENT "'local'"
#endif

#if defined(alloc_test)
    #define TEST test_alloc
#elif defined(write_test)
    #define TEST test_write
#elif defined(read_test)
    #define TEST test_read
#else
    #define TEST()
    #error "Choose -Dalloc_test, -Dwrite_test, or -Dread_test"
#endif

double get_elapsed_time(struct timespec start_time, struct timespec end_time);
void test_alloc();
void test_write();
void test_read();
volatile static char c;

int main(int argc, char **argv)
{
    #ifdef cxl
        emucxl_init();
    #endif
        struct timespec start_time, end_time;
        printf("Running " STR(TEST) " in environment " TEST_ENVIRONMENT "...\n");
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        TEST();

        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double time_spent = get_elapsed_time(start_time, end_time);
        #if defined(alloc_test)
            printf(STR(ITERATIONS) " iterations of allocating and deallocating " STR(REGION_SIZE) " bytes took %.8f seconds to run.\n", time_spent);
        #elif defined(write_test)
            printf(STR(ITERATIONS) " iterations of writing " STR(REGION_SIZE) " bytes took %.8f seconds to run.\n", time_spent);
        #elif defined(read_test)
            printf(STR(ITERATIONS) " iterations of reading " STR(REGION_SIZE) " bytes took %.8f seconds to run.\n", time_spent);
        #endif
    #ifdef cxl
        emucxl_exit();
    #endif
}

void test_alloc()
{
    for (int i = 0; i < ITERATIONS; i++) {
        void *region = ALLOC(REGION_SIZE, 1);
        FREE(region, REGION_SIZE);
    }
}

void test_write()
{
    for (int i = 0; i < ITERATIONS; i++) {
        volatile char *region = (volatile char*)ALLOC(REGION_SIZE, 1);
        for (int i = 0; i < REGION_SIZE; i++) {
            region[i] = 1;
        }
        FREE((void*)region, REGION_SIZE);
    }
}


void test_read()
{
    for (int i = 0; i < ITERATIONS; i++) {
        volatile char *region = (volatile char*)ALLOC(REGION_SIZE, 1);
        for (int i = 0; i < REGION_SIZE; i++) {
            c = region[i];
        }
        FREE((void*)region, REGION_SIZE);
    }
}

double get_elapsed_time(struct timespec start_time, struct timespec end_time) {
    long seconds = end_time.tv_sec - start_time.tv_sec;
    long nanoseconds = end_time.tv_nsec - start_time.tv_nsec;

    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1e9;
    }
    double elapsed = seconds + nanoseconds / 1e9;
    if (elapsed < 0.0) {
        return 0.0;
    }
    return elapsed;
}