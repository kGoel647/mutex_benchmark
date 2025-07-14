import logging

class Constants:
    class Defaults:
        MUTEX_NAMES = [
            "burns_lamport",
            # "dijkstra",
            # "bakery",
            "spin",
            "exp_spin",
            "nsync",
            # "pthread",
            "mcs",
            # "knuth",
            # "peterson",
            # "clh",
            "hopscotch",
            "ticket", 
            # "halfnode", 
            # "lamport", 
            # "boulangerie",
            "tree_cas_elevator",
            "linear_cas_elevator",
            "tree_bl_elevator",
            "linear_bl_elevator",
            "futex",
        ]
        EXECUTABLE_NAME = "max_contention_bench"
        BENCH_N_THREADS = "10"
        BENCH_N_SECONDS = "1"
        N_PROGRAM_ITERATIONS = 10
        DATA_FOLDER = "./data/generated"
        LOGS_FOLDER = "./data/logs"
        EXECUTABLE = f"./build/apps/{EXECUTABLE_NAME}/{EXECUTABLE_NAME}"
        MULTITHREADED = False
        THREAD_LEVEL = True
        SCATTER = False
        LOG = logging.INFO
        SKIP = 1
        MAX_N_POINTS = 1000
    
