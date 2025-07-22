#include "lock.hpp"
#include "../utils/cxl_utils.hpp"
#include <stdexcept>

class DijkstraMutex : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        size_t k_size = sizeof(std::atomic_size_t);
        size_t unlocking_size = sizeof(std::atomic_bool) * num_threads;
        size_t c_size = sizeof(std::atomic_bool) * num_threads;
        _cxl_region_size = sizeof(std::atomic_size_t) + unlocking_size + c_size;
        _cxl_region = (volatile char*)ALLOCATE(_cxl_region_size);

        this->k = (std::atomic_size_t*)&_cxl_region[0];
        this->unlocking = (std::atomic_bool*)&_cxl_region[k_size];
        this->c         = (std::atomic_bool*)&_cxl_region[k_size + unlocking_size];
        for (size_t i = 0; i < num_threads; i++) {
            unlocking[i] = true;
            c[i] = true;
        }
        *this->k = 0;
        this->num_threads = num_threads;
    }

    void lock(size_t thread_id) override {
        // TODO refactor and remove goto
        unlocking[thread_id] = false;
    try_again:
        c[thread_id] = true;
        if (*k != thread_id) {
            while (!unlocking[*k]) {}
            *k = thread_id;
            
            goto try_again;
        } 
        c[thread_id] = false;
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
        FREE((void*)_cxl_region, _cxl_region_size);
    }

    std::string name(){return "djikstra";};

private:
    volatile char *_cxl_region;
    size_t _cxl_region_size; // this could just be re-calculated
    std::atomic_bool *unlocking;
    std::atomic_bool *c;
    std::atomic_size_t *k;
    size_t num_threads;
};
