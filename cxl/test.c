
#include "emucxl/src/emucxl_lib.h"
#include <linux/time.h>
#include <time.h>
#include <stdio.h>

#ifndef REGION_SIZE
    #define REGION_SIZE 0x10000
#endif

#ifndef ITERATIONS
    #define ITERATIONS 1000
#endif

#define STR(x) #x

#ifdef emucxl_alloc
    #define TEST test_allocation
#endif

#ifdef emucxl_write
    #define TEST test_write
#endif

#ifdef emucxl_read
    #define TEST test_read
#endif

double get_elapsed_time(struct timespec start_time, struct timespec end_time);
void test_allocation();
void test_write();
void test_read();
volatile static char c;

int main(int argc, char **argv)
{
    struct timespec start_time;
    struct timespec end_time;

    printf("Running " STR(TEST) "...\n");
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    TEST();
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_spent = get_elapsed_time(start_time, end_time);

    #ifdef emucxl_alloc
        printf(STR(ITERATIONS) " iterations of allocating and deallocating " STR(REGION_SIZE) " bytes took %f seconds to run.\n", time_spent);
    #endif

    #ifdef emucxl_write
        printf("Writing " STR(REGION_SIZE) " bytes took %f seconds to run.\n", time_spent);
    #endif

    #ifdef emucxl_read
        printf("Reading " STR(REGION_SIZE) " bytes took %f seconds to run.\n", time_spent);
    #endif
}

void test_allocation()
{
    for (int i = 0; i < ITERATIONS; i++) {
        void *region = emucxl_alloc(REGION_SIZE, 1);
        emucxl_free(region, REGION_SIZE);
    }
}

void test_write()
{
    volatile char *region = (volatile char*)emucxl_alloc(REGION_SIZE, 1);
    for (int i = 0; i < REGION_SIZE; i++) {
        region[i] = 1;
    }
    emucxl_free((void*)region, REGION_SIZE);
}


void test_read()
{
    volatile char *region = (volatile char*)emucxl_alloc(REGION_SIZE, 1);
    for (int i = 0; i < REGION_SIZE; i++) {
        c = region[i];
    }
    emucxl_free((void*)region, REGION_SIZE);
}

void test_read()
{
    volatile char *region = (volatile char*)emucxl_alloc(REGION_SIZE, 1);
    for (int i = 0; i < REGION_SIZE; i++) {
        c = region[i];
    }
    emucxl_free((void*)region, REGION_SIZE);
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