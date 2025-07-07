#include "lock.hpp"
#include <atomic>
#include <time.h>

class SpinLock : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        (void)num_threads; // This parameter is not used
    }

    void lock(size_t thread_id) override {
        (void)thread_id; // This parameter is not used

        while (lock_.test_and_set(std::memory_order_acquire)) {
            // Busy wait
        }
    }

    void unlock(size_t thread_id) override {
        (void)thread_id; // This parameter is not used

        lock_.clear();
    }

    void destroy() override {}

    std::string name(){
        return "spin";
    }
    
private:
    volatile std::atomic_flag lock_ = ATOMIC_FLAG_INIT;
};
