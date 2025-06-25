#include "lock.hpp"
#include <stdatomic.h>

class SpinLock : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        (void)num_threads; // This parameter is not used in this implementation
    }

    void lock(size_t thread_id) override {
        (void)thread_id; // This parameter is not used in this implementation
        bool false_ = false;
        while (!std::atomic_compare_exchange_weak(&locked, &false_, true));
    }
    void unlock(size_t thread_id) override {
        (void)thread_id; // This parameter is not used in this implementation
        locked = false;
    }
    void destroy() override {}
    
private:
    volatile std::atomic<bool> locked;
};
