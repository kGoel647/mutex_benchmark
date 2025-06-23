#ifndef __BENCH_UTILS_HPP_
#define __BENCH_UTILS_HPP_

#pragma once

#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <memory>
#include <atomic>


struct per_thread_stats {
    int thread_id;
    int num_iterations;

    struct timespec start_time;
    struct timespec end_time;
};


struct run_stats {
    int num_threads;
    struct per_thread_stats **thread_stats;
    struct rusage usage;
};


// Use getrusage  to record resource usage

void record_rusage();
void print_rusage(struct rusage *usage);

void start_timer(struct per_thread_stats *stats);
void end_timer(struct per_thread_stats *stats);

void report_thread_latency(struct per_thread_stats *stats);
void report_run_latency(struct run_stats *stats);

#endif // __BENCH_UTILS_HPP_