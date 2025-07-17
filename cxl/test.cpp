// TODO: restructure cxl folder

#define debug(...)

#if defined(__x86_64)
    //#define Fence() __asm__ __volatile__ ( "mfence" )
    #define Fence() __asm__ __volatile__ ( "lock; addq $0,128(%%rsp);" ::: "cc" )
#elif defined(__i386)
    #define Fence() __asm__ __volatile__ ( "lock; addl $0,128(%%esp);" ::: "cc" )
#elif defined(__ARM_ARCH)
    #define Fence() __asm__ __volatile__ ( "DMB ISH" ::: )
#else
    #error unsupported architecture
#endif

#include "bakery_static_mutex.cpp"
#include "bl_static_mutex.cpp"
#include "emucxl_lib.h"

#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 10
#define NUM_ITERATIONS 1000
#define MUTEX_NAME_ROOT bl

#define concat_nx(a, b) a##b
#define concat(a, b) concat_nx(a, b)

#define str_nx(a) #a
#define str(a) str_nx(a)

#define MUTEX concat(MUTEX_NAME_ROOT, _static_mutex)
#define mutex_t struct MUTEX
#define m_lock(mutex, thread_id) concat(MUTEX, _lock)(mutex, thread_id)
#define m_unlock(mutex, thread_id) concat(MUTEX, _unlock)(mutex, thread_id)
#define m_init(region, num_threads) concat(MUTEX, _init)(region, num_threads)
#define m_get_size(num_threads) concat(MUTEX, _get_size)(num_threads)

#define LOCAL_MEMORY 0
#define REMOTE_CXL_MEMORY 1

struct thread_info {
    size_t thread_id;
    mutex_t *mutex;
    size_t *counter;
    bool *start_flag;
};

static void *thread_start(void *arg)
{
    struct thread_info thread_info = *(struct thread_info*)arg;
    printf("%ld: thread_start running...\n", thread_info.thread_id);

    while (!*thread_info.start_flag);

    for (size_t i = 0; i < NUM_ITERATIONS; i++) {
        debug("%ld: Locking...\n", thread_info.thread_id);
        m_lock(thread_info.mutex, thread_info.thread_id);
        debug("%ld: Locked.\n", thread_info.thread_id);
        (*thread_info.counter)++;
        debug("%ld: Unlocking...\n", thread_info.thread_id);
        m_unlock(thread_info.mutex, thread_info.thread_id);
        debug("%ld: Unlocked. (Completed one iteration)\n", thread_info.thread_id);
    }

    printf("%ld: thread_start finished.\n", thread_info.thread_id);

    return nullptr;
}

void test_mutex()
{
    size_t region_size = m_get_size(NUM_THREADS);
    void *region = emucxl_alloc(region_size, REMOTE_CXL_MEMORY);
    mutex_t *mutex = m_init(region, NUM_THREADS);
    // Should these be in shared memory?
    size_t counter = 0;
    bool start_flag = false;
    pthread_t threads[NUM_THREADS];
    struct thread_info threads_info[NUM_THREADS];
    
    for (size_t thread_id = 0; thread_id < NUM_THREADS; thread_id++)
    {
        struct thread_info info = { thread_id, mutex, &counter, &start_flag };
        threads_info[thread_id] = info;
        pthread_create(&threads[thread_id], NULL, &thread_start, &threads_info[thread_id]);
    }

    start_flag = true;

    void *_ret;
    for (size_t thread_id = 0; thread_id < NUM_THREADS; thread_id++)
    {
        pthread_join(threads[thread_id], &_ret);
    }

    printf("expected=%d, actual=%ld\n", NUM_ITERATIONS * NUM_THREADS, counter);

    emucxl_free(region, region_size);
}

int main(int argc, char **argv)
{
    emucxl_init();

    printf("Testing " str(MUTEX) " ...\n");
    test_mutex();
    printf("done\n");

    emucxl_exit();

    printf("Done testing.\n");
    return 0;
}