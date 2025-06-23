#include "max_contention_bench.hpp"
#include "bench_utils.hpp"

#include "lock.hpp"
#include "pthread_lock.cpp"

void max_contention_bench(int num_threads, int num_iterations, SoftwareMutex* lock) {

    // Create run args structure to hold thread arguments
    // struct run_args args;
    // args.num_threads = num_threads;
    // args.thread_args = new per_thread_args*[num_threads];

    // Create shared memory for the lock
    // This could be a simple pointer or a more complex shared memory structure
    // void* shared_memory = nullptr; // Replace with actual shared memory allocation if needed

    // Initialize the lock
    lock->init(num_threads);

    // Create a flag to signal the threads to start
    std::shared_ptr<std::atomic<bool>> start_flag = std::make_shared<std::atomic<bool>>(false);

    // Create an array of thread arguments
    std::vector<per_thread_args> thread_args(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        thread_args[i].thread_id = i;
        thread_args[i].stats.num_iterations = num_iterations;
        thread_args[i].lock = lock; // Pass the lock to each thread
    }

    // Create an array of threads
    std::vector<std::thread> threads(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        thread_args[i].start_flag = start_flag; // Share the start flag with each thread
        threads[i] = std::thread([&thread_args, i]() {

            // Record the thread ID
            thread_args[i].stats.thread_id = thread_args[i].thread_id;
        
            // Each thread will run this function
            while (!*thread_args[i].start_flag) {
                // Wait until the start flag is set
            }

            // Start the timer for this thread
            start_timer(&thread_args[i].stats);

            // Perform the locking operations
            for (int j = 0; j < thread_args[i].stats.num_iterations; ++j) {
                thread_args[i].lock->lock(thread_args[i].thread_id);
                // Critical section code goes here
                thread_args[i].lock->unlock(thread_args[i].thread_id);
            }

            // End the timer for this thread
            end_timer(&thread_args[i].stats);
        });
    }

    // Start the threads
    *start_flag = true; // Set the start flag to signal the threads to begin

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Cleanup resources
    lock->destroy(); // Cleanup the lock resources

    // Destroy the lock 
    delete lock; // Assuming lock was dynamically allocated

    // Output benchmark results

    for (auto& args : thread_args) {
        report_thread_latency(&args.stats, false); // Report latency for each thread
    }

    // record_rusage(); // Record resource usage
    // report_latency(&args); // Report latency if needed
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <num_iterations>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_iterations = atoi(argv[2]);

    // Create a lock instance (using Pthread lock as an example)
    SoftwareMutex* lock = new Pthread();
    
    // Run the max contention benchmark
    max_contention_bench(num_threads, num_iterations, lock);

    return 0;
}