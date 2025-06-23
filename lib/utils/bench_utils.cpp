#include "bench_utils.hpp"

#include <stdio.h>

void record_rusage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        print_rusage(&usage);
    } else {
        perror("getrusage failed");
    }
}

void print_rusage(struct rusage *usage) {
    printf("User CPU time used: %ld.%06d seconds\n", 
           usage->ru_utime.tv_sec, usage->ru_utime.tv_usec);
    printf("System CPU time used: %ld.%06d seconds\n", 
           usage->ru_stime.tv_sec, usage->ru_stime.tv_usec);
    printf("Maximum resident set size: %ld KB\n", usage->ru_maxrss);
    printf("Integral shared memory size: %ld KB\n", usage->ru_ixrss);
    printf("Integral unshared data size: %ld KB\n", usage->ru_idrss);
    printf("Integral unshared stack size: %ld KB\n", usage->ru_isrss);
    printf("Page reclaims (soft page faults): %ld\n", usage->ru_minflt);
    printf("Page faults (hard page faults): %ld\n", usage->ru_majflt);
    printf("Swaps: %ld\n", usage->ru_nswap);
    printf("Block input operations: %ld\n", usage->ru_inblock);
    printf("Block output operations: %ld\n", usage->ru_oublock);
    printf("IPC messages sent: %ld\n", usage->ru_msgsnd);
    printf("IPC messages received: %ld\n", usage->ru_msgrcv);
    printf("Signals received: %ld\n", usage->ru_nsignals);
    printf("Voluntary context switches: %ld\n", usage->ru_nvcsw);
    printf("Involuntary context switches: %ld\n", usage->ru_nivcsw);
}

void start_timer(struct per_thread_stats *stats) {
    clock_gettime(CLOCK_MONOTONIC, &stats->start_time);
}

void end_timer(struct per_thread_stats *stats) {
    clock_gettime(CLOCK_MONOTONIC, &stats->end_time);
}

void report_thread_latency(struct per_thread_stats *stats, bool csv) {

    long seconds = stats->end_time.tv_sec - stats->start_time.tv_sec;
    long nanoseconds = stats->end_time.tv_nsec - stats->start_time.tv_nsec;

    if (nanoseconds < 0) {
        seconds--;
        nanoseconds += 1e9;
    }

    double elapsed = seconds + nanoseconds / 1e9;

    if (csv) {
        printf("%d,%d,%.6f\n", stats->thread_id, stats->num_iterations, elapsed);
        return;
    }
    else {
        printf("Thread %d: %d iterations completed in %.6f seconds\n",
               stats->thread_id, stats->num_iterations, elapsed);
    }

}

void report_run_latency(struct run_args *stats){
    printf("Run statistics:\n");
    
}