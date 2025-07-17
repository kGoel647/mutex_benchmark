#include "lock.hpp"
#include "trylock.hpp"
#include <atomic>
#include <time.h>
#include <stdexcept>
#include <string.h>

class BurnsLamportMutex : public virtual TryLock {
public:
    void init(size_t num_threads) override {
        size_t _cxl_region_size = num_threads * sizeof(volatile bool) + sizeof(volatile bool);
        this->_cxl_region = (volatile char*)malloc(_cxl_region_size);
        this->in_contention = (volatile bool*)&this->_cxl_region[0];
        size_t in_contention_size = sizeof(volatile bool) * num_threads;
        memset((void*)in_contention, 0, in_contention_size);
        this->fast = (volatile bool*)&this->_cxl_region[in_contention_size];
        *this->fast = false;
        this->num_threads = num_threads;
    }

    bool trylock(size_t thread_id) override {
        in_contention[thread_id] = true;
        std::atomic_thread_fence(std::memory_order_seq_cst);
        for (size_t higher_priority_thread = 0; higher_priority_thread < thread_id; higher_priority_thread++) {
            if (in_contention[higher_priority_thread]) {
                in_contention[thread_id] = false;
                return false;
            }
        }
        for (size_t lower_priority_thread = thread_id + 1; lower_priority_thread < num_threads; lower_priority_thread++) {
            while (in_contention[lower_priority_thread]) {
                // Busy wait for lower-priority thread to give up.
            }
        }
        bool leader = !*this->fast;
        if (leader) { 
            *fast = true; 
        }
        in_contention[thread_id] = false;
        return leader;
    }

    void lock(size_t thread_id) override {
        while (!trylock(thread_id)) {
            // Busy wait
        }
    }

    void unlock(size_t thread_id) override {
        (void)thread_id; // This parameter is not used
        *fast = false;
    }

    void destroy() override {
        free((void*)in_contention);
    }

    std::string name() override {
        return "burns_lamport";
    }
    
private:
    volatile char *_cxl_region;
    volatile bool *fast;
    volatile bool *in_contention;
    size_t num_threads;
};