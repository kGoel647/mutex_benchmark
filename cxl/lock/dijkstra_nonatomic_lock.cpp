#include "../harness.cpp"

#include "lock.hpp"
#include <stdexcept>

class DijkstraNonatomicMutex : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        size_t size = num_threads * sizeof(bool) * 2;
        this->_cxl_region = (volatile char*)CXL_ALLOCATE(size);

        this->unlocking = (volatile bool*)&this->_cxl_region[0];

        size_t c_offset = sizeof(bool) * num_threads;
        this->c = (volatile bool*)&this->_cxl_region[c_offset];

        for (size_t i = 0; i < num_threads; i++) {
            unlocking[i] = true;
            c[i] = true;
        }
        this->k = 0;
        this->num_threads = num_threads;
    }

    void lock(size_t thread_id) override {
        // TODO refactor and remove goto
        unlocking[thread_id] = false;
    try_again:
        c[thread_id] = true;
        std::atomic_thread_fence(std::memory_order_seq_cst);
        if (k != thread_id) {
            while (!unlocking[k]) {}
            k = thread_id;
            std::atomic_thread_fence(std::memory_order_seq_cst);
            
            goto try_again;
        } 
        c[thread_id] = false;
        std::atomic_thread_fence(std::memory_order_seq_cst);
        for (size_t j = 0; j < num_threads; j++) {
            if (j != thread_id && !c[j]) {
                goto try_again;
            }
        }

    }
    void unlock(size_t thread_id) override {
        unlocking[thread_id] = true;
        c[thread_id] = true;
    }
    void destroy() override {
        CXL_FREE((void*)this->_cxl_region, num_threads * sizeof(bool) * 2);
    }

    std::string name() override {
        return "djikstra";
    }

private:
    volatile char *_cxl_region;
    volatile bool *unlocking;
    volatile bool *c;
    volatile size_t k;
    size_t num_threads;
};