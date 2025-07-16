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
    for (size_t i = 0; i < num_threads; i++) {
        mutex->in_contention[i] = false;
    }
    return mutex;
}


bool bl_static_mutex_trylock(struct bl_static_mutex *mutex, size_t thread_id) 
{
    size_t num_threads = mutex->num_threads;

    mutex->in_contention[thread_id] = true;
    // Does this fence happen?
    atomic_thread_fence(memory_order_seq_cst);
    for (size_t i = 0; i < thread_id; i++) {
        if (mutex->in_contention[i]) {
            // Give up if a higher-priority thread is in contention.
            mutex->in_contention[thread_id] = false;
            return false;
        }
    }

    for (size_t i = thread_id + 1; i < num_threads; i++) {
        while (mutex->in_contention[i]) {
            // Wait for it to stop being in contention.
        }
    }

    bool leader = !mutex->is_locked;
    if (leader) {
        mutex->is_locked = true;
    }
    mutex->in_contention[thread_id] = false;
    return leader;
}

void bl_static_mutex_lock(struct bl_static_mutex *mutex, size_t thread_id)
{
    while (!bl_static_mutex_trylock(mutex, thread_id)) {
        // Busy wait
    }
}

void bl_static_mutex_unlock(struct bl_static_mutex *mutex, size_t thread_id)
{
    mutex->is_locked = false;
}