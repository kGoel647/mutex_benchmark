#include <stdint.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdbool.h>
#include <threads.h>
#include <string.h>

struct bakery_static_mutex {
    size_t num_threads;
    atomic_size_t *number;
    atomic_bool *choosing;
    char tail[];
};

size_t bakery_static_mutex_get_size(size_t num_threads)
{
    // This would break if sizeof(atomic_bool) were less than number_size and caused it to be misaligned
    size_t number_size = sizeof(atomic_size_t) * num_threads;
    size_t choosing_size = sizeof(atomic_bool) * num_threads;
    return sizeof(struct bakery_static_mutex) + number_size + choosing_size;
}

struct bakery_static_mutex *bakery_static_mutex_init(void *region, size_t num_threads)
{
    struct bakery_static_mutex *mutex = (struct bakery_static_mutex*)region;
    mutex->num_threads = num_threads;
    mutex->number = (atomic_size_t*)&mutex->tail[0];
    mutex->choosing = (atomic_bool*)&mutex->tail[num_threads * sizeof(atomic_size_t)];
    memset((void*)mutex->tail, 0, num_threads * (sizeof(atomic_size_t) + sizeof(atomic_bool)));
}

void bakery_static_mutex_lock(struct bakery_static_mutex *mutex, size_t thread_id) 
{
    // Get "bakery number"
    mutex->choosing[thread_id] = true;
    size_t my_bakery_number = 1;
    for (size_t i = 0; i < mutex->num_threads; i++) {
        if (mutex->number[i] + 1 > my_bakery_number) {
            my_bakery_number = mutex->number[i] + 1;
        }
    }
    mutex->number[thread_id] = my_bakery_number;
    mutex->choosing[thread_id] = false;
    // Lock waiting part
    for (size_t j = 0; j < mutex->num_threads; j++) {
        while (mutex->choosing[j] != 0) {
            // Wait for that thread to be done choosing a number.
        }
        while ((mutex->number[j] != 0 && mutex->number[j] < mutex->number[thread_id]) 
            || (mutex->number[j] == mutex->number[thread_id] && j < thread_id)) {
            // Stall until our bakery number is the lowest..
        }
    }
}

void bakery_static_mutex_unlock(struct bakery_static_mutex *mutex, size_t thread_id)
{
    atomic_thread_fence(memory_order_seq_cst);
    mutex->number[thread_id] = 0;
}