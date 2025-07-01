#include "lock.hpp"
#include <stdexcept>
#include <atomic>
#include <stdio.h>

// TODO: explicit memory ordering.
// NOTE: Because of the limitations of `thread_local`,
// this class MUST be singleton. TODO: This is not yet explicitly enforced.

struct QNode {
    std::atomic<volatile QNode*> next;
    std::atomic_bool locked;
};

class MCSMutex : public virtual SoftwareMutex {
public:
    void init(size_t num_threads) override {
        (void)num_threads; // Unused

        lock_.store(nullptr);
    }

    void lock(size_t thread_id) override {
        (void)thread_id; // Unused

        my_node.next.store(nullptr);
        // my_node.locked may remain uninitialized. 
        // This is fine because if we have the lock we will never
        // access my_node.locked.
        volatile QNode* predecessor = lock_.exchange(&my_node);
        // lock_.locked.store(true);
        // Once the lock._next is swapped, won't be modified
        if (predecessor != nullptr) {
            my_node.locked.store(true);
            predecessor->next.store(&my_node);
            while (my_node.locked) {
                // printf("Waiting for node unlock...\n");
                // Busy wait
            }
        }
        // printf("Locked\n");
    }

    void unlock(size_t thread_id) override {
        (void)thread_id; // Unused

        if (my_node.next.load() == nullptr) {
            // printf("std::atomic_compare_exchange_strong(\n"
            //         "    %p,\n"
            //         "    %p,\n"
            //         "    %p\n"
            //         ")\n",
            //         &lock_,
            //         (volatile QNode**)&my_node,
            //         (volatile QNode*)nullptr
            // );
            volatile QNode* my_node_p = &my_node;
            if (std::atomic_compare_exchange_strong(
                    &lock_,
                    // We need to get the address of my_node, but the addressing operation
                    // with a temporary variable makes this operation non-atomic and ruins everything.
                    &my_node_p,
                    nullptr
                )) {
                    // printf("Successfully std::atomic_compare_exchanged, unlocked\n");
                    return;
                }
            while (my_node.next.load() == nullptr) {
                // printf("Waiting for successor...\n");
                // Busy wait (should this return to the start?)
            }
        }
        my_node.next.load()->locked.store(false);
        // printf("Unlocked\n");
    }

    void destroy() override {

    }

    std::string name() override {
        return "mcs";
    }
private:
    // Do these need to be volatile?
    static volatile thread_local QNode my_node;
    std::atomic<volatile QNode*> lock_;
};

volatile thread_local QNode MCSMutex::my_node;