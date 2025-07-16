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
    for (int i = 0; i < NUM_THREADS; i++) {
        bakery_static_mutex_lock(mutex, i);
        bakery_static_mutex_unlock(mutex, i);
    }
    emucxl_free(region, region_size);
}

int main(int argc, char **argv)
{
    emucxl_init();

    printf("Testing bakery... ");
    test_bakery();
    printf("done\n");

    emucxl_exit();

    printf("Done testing.\n");
    return 0;
}