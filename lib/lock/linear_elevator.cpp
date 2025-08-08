#include "lock.hpp"
#include <stdexcept>
#include <atomic>

// TODO: explicit memory ordering.
// NOTE: Because of the limitations of `thread_local`,
// this class MUST be singleton. TODO: This is not yet explicitly enforced.

template <class WakerLock>
class LinearElevatorMutex : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        size_t waker_lock_size = WakerLock::get_cxl_region_size(num_threads);
        size_t thread_n_given_lock_size = std::hardware_destructive_interference_size * (num_threads + 1);
        size_t thread_n_is_waiting_size = sizeof(volatile bool) * num_threads;
        _cxl_region_size = waker_lock_size + thread_n_given_lock_size + thread_n_is_waiting_size;
        _cxl_region = (volatile char*)ALLOCATE(_cxl_region_size);

        size_t offset = 0;
        offset += thread_n_given_lock_size;
        this->designated_waker_lock.region_init(num_threads, &_cxl_region[offset]);
        offset += waker_lock_size;
        this->thread_n_is_waiting = (volatile bool*)&_cxl_region[offset];
        
        this->num_threads = num_threads;
        for (size_t i = 0; i < num_threads; i++) {
            *get_thread_n_given_lock(i) = false;
        }
        *get_thread_n_given_lock(num_threads) = true;
        for (size_t i = 0; i < num_threads; i++) {
            this->thread_n_is_waiting[i] = false;
        }
    }

    inline volatile bool *get_thread_n_given_lock(size_t thread_id) {
        return (volatile bool*)&this->_cxl_region[thread_id * std::hardware_destructive_interference_size];
    }

    void lock(size_t thread_id) override {
        volatile bool *given_lock = get_thread_n_given_lock(thread_id);
        thread_n_is_waiting[thread_id] = true;
        if (designated_waker_lock.trylock(thread_id)) {
            // This thread will wake if
            // a) the unlocker makes a mistake and doesn't realize anyone is waiting or
            // b) (more likely) if this thread is the first to get to the lock
            volatile bool *designated_waker_given_lock = get_thread_n_given_lock(num_threads);
            while (*given_lock == false && *designated_waker_given_lock == false) {
                // Busy wait
            }
            *designated_waker_given_lock = false;
            designated_waker_lock.unlock(thread_id);
        } else {
            while (*given_lock == false) {
                // Busy wait
            }
        }
        *given_lock = false;
        Fence();
    }

    void unlock(size_t thread_id) override {
        // Cycle around the thread list to find the next successor
        // Start at 1 because we don't loop back to ourself.
        for (size_t offset = 1; offset < num_threads; offset++) {
            size_t next_successor_index = (thread_id + offset) % num_threads;
            if (thread_n_is_waiting[next_successor_index]) {
                thread_n_is_waiting[thread_id] = false;
                Fence();
                *get_thread_n_given_lock(next_successor_index) = true;
                return; // Successfully passed off to successor
            }
        }

        thread_n_is_waiting[thread_id] = false;
        Fence();

        // No successor found.
        *get_thread_n_given_lock(num_threads) = true;
    }

    void destroy() override {
        FREE((void*)_cxl_region, _cxl_region_size);
    }

    std::string name() override {
        return "linear_elevator";
    }
private:
    volatile char *_cxl_region;
    size_t _cxl_region_size;
    size_t num_threads;

    WakerLock designated_waker_lock;
    volatile bool *thread_n_is_waiting;
};