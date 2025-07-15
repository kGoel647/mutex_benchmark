
#include <time.h>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

#include "max_contention_bench.hpp"
#include "bench_utils.hpp"
#include "lock.hpp"

#include "pthread_lock.cpp"
#include "cpp_std_mutex.cpp"
#include "boost_lock.cpp"
#include "dijkstra_lock.cpp"
#include "dijkstra_nonatomic_lock.cpp"
#include "spin_lock.hpp"
#include "nsync_lock.cpp"
#include "exp_spin_lock.cpp"
#include "nsync_lock.cpp"
#include "bakery_mutex.cpp"
#include "bakery_nonatomic_mutex.cpp"
#include "lamport_lock.cpp"
#include "mcs_lock.cpp"
#include "mcs_volatile_lock.cpp"
#include "mcs_malloc_lock.cpp"
#include "knuth_lock.cpp"
#include "peterson_lock.cpp"
#include "boulangerie.cpp"
#include "ticket_lock.cpp"
#include "threadlocal_ticket_lock.cpp"
#include "ring_ticket_lock.cpp"
#include "null_mutex.cpp"
#include "halfnode_lock.cpp"
#include "hopscotch_lock.cpp"
#include "clh_lock.cpp"
#include "linear_cas_elevator.cpp"
#include "tree_cas_elevator.cpp"
#include "linear_bl_elevator.cpp"
#include "tree_bl_elevator.cpp"
#include "burns_lamport_lock.hpp"
#include "futex_mutex.cpp"
#include "elevator_mutex.hpp"
#include "szymanski.cpp"

int max_contention_bench(
    int num_threads, 
    std::chrono::seconds run_time, 
    bool csv, 
    bool thread_level, 
    bool no_output, 
    int max_critical_delay_iterations, 
    int max_noncritical_delay_iterations, 
    bool low_contention,
    int stagger_ms,
    SoftwareMutex* lock
) {
    // Create run args structure to hold thread arguments
    // struct run_args args;
    // args.num_threads = num_threads;
    // args.thread_args = new per_thread_args*[num_threads];

    // Create shared memory for the lock
    // This could be a simple pointer or a more complex shared memory structure
    // void* shared_memory = nullptr; // Replace with actual shared memory allocation if needed

    // Initialize the lock
    lock->init(num_threads);
    auto start_flag = std::make_shared<std::atomic<bool>>(false);
    auto end_flag   = std::make_shared<std::atomic<bool>>(false);
    volatile int* counter = (volatile int*)malloc(sizeof(int));
    *counter = 0;

    std::vector<per_thread_args> thread_args(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        thread_args[i].thread_id            = i;
        thread_args[i].stats.run_time       = run_time;
        thread_args[i].stats.num_iterations = 0;
        thread_args[i].lock                 = lock;
        thread_args[i].start_flag           = start_flag;
        thread_args[i].end_flag             = end_flag;
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        if (thread_level) {
            threads.emplace_back([&, i]() {
                thread_args[i].stats.thread_id = i;
                if (low_contention && stagger_ms > 0) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(i * stagger_ms)
                    );
                }
                while (!*start_flag) {}
                while (!*end_flag) {
                    lock->lock(i);
                    thread_args[i].stats.num_iterations++;
                    (*counter)++; // Critical section
                    Fence(); //ensure that counter was updated before unlocking; required for any impl.
                    lock->unlock(i);
                }
            });
        } else {
            threads.emplace_back([&, i]() {
                thread_args[i].stats.thread_id = i;
                init_lock_timer(&thread_args[i].stats);
                if (low_contention && stagger_ms > 0) {
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(i * stagger_ms)
                    );
                }
                while (!*start_flag) {}
                struct timespec delay = {0,0}, rem;
                while (!*end_flag) {
                    start_lock_timer(&thread_args[i].stats);
                    lock->lock(i);
                    (*counter)++;
                    lock->unlock(i);
                    end_lock_timer(&thread_args[i].stats);

                    delay.tv_nsec = rand() % max_noncritical_delay_ns;
                    nanosleep(&delay, &rem);
                    thread_args[i].stats.num_iterations++;
                }
            });
        }
    }

    *start_flag = true;
    std::this_thread::sleep_for(run_time);
    *end_flag = true;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    int expected = 0;
    for (auto& targs : thread_args) {
        expected += targs.stats.num_iterations;
    }
    if (*counter != expected) {
        fprintf(stderr,
            "Mutex %s failed; counter = %d, expected = %d\n",
            lock->name().c_str(), *counter, expected
        );
        return 1;
    }

    lock->destroy();
    free((void*)counter);
    delete lock;

    if (!no_output) {
        for (auto& targs : thread_args) {
            report_thread_latency(&targs.stats, csv, thread_level);
            if (!thread_level) destroy_lock_timer(&targs.stats);
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr,
            "Usage: %s <mutex_name> <num_threads> <run_time_s> <max_noncrit_delay_ns> "
            "[--csv] [--thread-level] [--no-output] [--low-contention] [--stagger-ms ms]\n",
            argv[0]
        );
        return 1;
    }

    const char* mutex_name            = argv[1];
    int         num_threads           = atoi(argv[2]);
    int         run_time_sec          = atoi(argv[3]);
    int         max_noncrit_delay_ns  = atoi(argv[4]);

    bool csv             = false;
    bool thread_level    = false;
    bool no_output       = false;
    bool low_contention  = false;
    int  stagger_ms      = 0;

    for (int i = 5; i < argc; ++i) {
        if (strcmp(argv[i], "--csv") == 0) {
            csv = true;
        } else if (strcmp(argv[i], "--thread-level") == 0) {
            thread_level = true;
        } else if (strcmp(argv[i], "--no-output") == 0) {
            no_output = true;
        } else if (strcmp(argv[i], "--low-contention") == 0) {
            low_contention = true;
        } else if (strcmp(argv[i], "--stagger-ms") == 0 && i + 1 < argc) {
            stagger_ms = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unrecognized flag: %s\n", argv[i]);
            return 1;
        }
    }

    if (max_noncrit_delay_ns <= 0) {
        max_noncrit_delay_ns = 1;
    }

    SoftwareMutex* lock = nullptr;
    if      (strcmp(mutex_name, "pthread") == 0)             lock = new Pthread();
    else if (strcmp(mutex_name, "cpp_std") == 0)             lock = new CPPMutex();
    else if (strcmp(mutex_name, "boost") == 0)               lock = new BoostMutex();
    else if (strcmp(mutex_name, "dijkstra") == 0)            lock = new DijkstraMutex();
    else if (strcmp(mutex_name, "dijkstra_nonatomic") == 0)  lock = new DijkstraNonatomicMutex();
    else if (strcmp(mutex_name, "spin") == 0)                lock = new SpinLock();
    else if (strcmp(mutex_name, "exp_spin") == 0)            lock = new ExponentialSpinLock();
    else if (strcmp(mutex_name, "nsync") == 0)               lock = new NSync();
    else if (strcmp(mutex_name, "bakery") == 0)              lock = new BakeryMutex();
    else if (strcmp(mutex_name, "bakery_nonatomic") == 0)    lock = new BakeryNonAtomicMutex();
    else if (strcmp(mutex_name, "lamport") == 0)             lock = new LamportLock();
    else if (strcmp(mutex_name, "mcs") == 0)                 lock = new MCSMutex();
    else if (strcmp(mutex_name, "mcs_volatile") == 0)        lock = new MCSVolatileMutex();
    else if (strcmp(mutex_name, "mcs_malloc") == 0)          lock = new MCSMallocMutex();
    else if (strcmp(mutex_name, "knuth") == 0)               lock = new KnuthMutex();
    else if (strcmp(mutex_name, "peterson") == 0)            lock = new PetersonMutex();
    else if (strcmp(mutex_name, "boulangerie") == 0)         lock = new Boulangerie();
    else if (strcmp(mutex_name, "szymanski") == 0)           lock = new SzymanskiLock();
    else if (strcmp(mutex_name, "ticket") == 0)              lock = new TicketMutex();
    else if (strcmp(mutex_name, "threadlocal_ticket") == 0)  lock = new ThreadlocalTicketMutex();
    else if (strcmp(mutex_name, "ring_ticket") == 0)         lock = new RingTicketMutex();
    else if (strcmp(mutex_name, "null") == 0)                lock = new NullMutex();
    else if (strcmp(mutex_name, "halfnode") == 0)            lock = new HalfnodeMutex();
    else if (strcmp(mutex_name, "hopscotch") == 0)           lock = new HopscotchMutex();
    else if (strcmp(mutex_name, "clh") == 0){                lock = new CLHMutex();
    else if (strcmp(mutex_name, "linear_cas_elevator") == 0) lock = new LinearCASElevatorMutex();
    else if (strcmp(mutex_name, "tree_cas_elevator") == 0)   lock = new TreeCASElevatorMutex();
    else if (strcmp(mutex_name, "linear_bl_elevator") == 0)  lock = new LinearBLElevatorMutex();
    else if (strcmp(mutex_name, "tree_bl_elevator") == 0)    lock = new TreeBLElevatorMutex();
    else if (strcmp(mutex_name, "burns_lamport") == 0)       lock = new BurnsLamportMutex();
    else if (strcmp(mutex_name, "futex") == 0)               lock = new FutexLock();
    else if (strcmp(mutex_name, "elevator") == 0)            lock = new ElevatorMutex();
    else {
        fprintf(stderr,
            "Unrecognized mutex '%s'\n", mutex_name
        );
        return 1;
    }

    return max_contention_bench(
        num_threads,
        std::chrono::seconds(run_time_sec),
        csv,
        thread_level,
        no_output,
        max_noncrit_delay_ns,
        low_contention,
        stagger_ms,
        lock
    );
}
