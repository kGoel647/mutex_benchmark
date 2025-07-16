#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <threads.h>
#include <string.h>
#include <stdatomic.h>


struct bl_static_mutex {
    size_t num_threads;
    volatile bool is_locked;
    volatile bool in_contention[];
};

size_t bl_static_mutex_get_size(size_t num_threads)
{
    return sizeof(struct bl_static_mutex) + sizeof(bool) * num_threads;
}

struct bl_static_mutex *bl_static_mutex_init(void *region, size_t num_threads)
{
    struct bl_static_mutex *mutex = (struct bl_static_mutex*)region;
    mutex->num_threads = num_threads;
    memset((void*)&mutex->in_contention, 0, num_threads);
}


void bl_static_mutex_lock(struct bl_static_mutex *mutex, size_t thread_id) 
{
    size_t num_threads = mutex->num_threads;
restart:
    mutex->in_contention[thread_id] = true;
    // Does this fence happen?
    atomic_thread_fence(memory_order_seq_cst);
    for (int i = 0; i < thread_id; i++) {
        if (mutex->in_contention[i]) {
            goto restart;
        }
    }

    for (int i = thread_id + 1; i < num_threads; i++) {
        while (mutex->in_contention[i]) {
            // Wait for it to stop being in contention.
        }
    }

    if (!mutex->is_locked) {
        mutex->is_locked = true;
        mutex->in_contention[thread_id] = false;
    } else {
        goto restart;
    }
}

void bl_static_mutex_unlock(struct bl_static_mutex *mutex, size_t thread_id)
{
    mutex->in_contention[thread_id] = false;
}