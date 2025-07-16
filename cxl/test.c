#include "bakery_static_mutex.c"
#include "emucxl/src/emucxl_lib.h"

#define NUM_THREADS 10

#define LOCAL_MEMORY 0
#define REMOTE_CXL_MEMORY 1

void test_bakery()
{
    size_t region_size = bakery_static_mutex_get_size(NUM_THREADS);
    void *region = emucxl_alloc(region_size, REMOTE_CXL_MEMORY);
    struct bakery_static_mutex *mutex = bakery_static_mutex_init(region, NUM_THREADS);
    bakery_static_mutex_lock(mutex, 0);
    bakery_static_mutex_unlock(mutex, 0);
    emucxl_free(region, region_size);
}

int main(int argc, char **argv)
{
    emucxl_init();

    test_bakery();

    emucxl_exit();

    return 0;
}