#include "lock.hpp"
#include <stdexcept>

class DijkstraMutex : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        this->unlocking = (volatile std::atomic_bool*)malloc(sizeof(std::atomic_bool) * (num_threads+1));
        this->c         = (volatile std::atomic_bool*)malloc(sizeof(std::atomic_bool) * (num_threads+1));
        for (size_t i = 0; i <= num_threads; i++) {
            unlocking[i] = true;
            c[i] = true;
        }
        this->k = 0;
        this->num_threads = num_threads;
    }

    void lock(size_t thread_id) override {
        // TODO refactor and remove goto
        unlocking[thread_id+1] = false;
    try_again:
        c[thread_id+1] = true;
        if (k != thread_id+1) {
            while (!unlocking[k]) {}
            k = thread_id+1;
            
            goto try_again;
        } 
        c[thread_id+1] = false;
        for (size_t j = 1; j <= num_threads; j++) {
            if (j != thread_id+1 && !c[j]) {
                goto try_again;
            }
        }

    }

    void unlock(size_t thread_id) override {
        k=0;
        unlocking[thread_id+1] = true;
        c[thread_id+1] = true;
    }

    void destroy() override {
        // free((void*)unlocking);
        // free((void*)c);
    }

    std::string name() override {return "djikstra";};

private:
    volatile std::atomic_bool *unlocking;
    volatile std::atomic_bool *c;
    volatile std::atomic<size_t> k;
    size_t num_threads;
};
