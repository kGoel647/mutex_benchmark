from .constants import *

import subprocess

# TODO: save data files instead of deleting them every time (in their own folder)

def get_data_file_name(mutex_name, i):
    return f"{DATA_FOLDER}/{mutex_name}-{i}.csv"

def get_command(mutex_name, *, csv=True, thread_level=False):
    args = [EXECUTABLE, mutex_name, BENCH_N_THREADS, BENCH_N_ITERATIONS]
    if csv:
        args.append("--csv")
    if thread_level:
        args.append("--thread-level")
    return args

def run_experiment_multithreaded():
    # Run experiment
    # print("Running programs...")
    for mutex_name in MUTEX_NAMES:
        # Create program threads
        threads = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = get_data_file_name(mutex_name, i)
            subprocess.run(["rm", "-f", data_file_name])
            command = get_command(mutex_name, csv=True, thread_level=True)
            thread = subprocess.Popen(command, stdout=subprocess.PIPE)
            threads.append([data_file_name, thread])
        # Collect data from threads
        for data_file_name, thread in threads:
            # print(f"Waiting on {data_file_name}")
            thread.wait()
            csv_data = thread.stdout.read()
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)

def run_experiment_single_threaded():
    # Run experiment
    for mutex_name in MUTEX_NAMES:
        # Create program threads
        threads = []
        for i in range(N_PROGRAM_ITERATIONS):
            print(f"{mutex_name=} | {i=}")
            data_file_name = get_data_file_name(mutex_name, i)
            subprocess.run(["rm", "-f", data_file_name])
            command = get_command(mutex_name, csv=True, thread_level=True)
            thread = subprocess.run(command, stdout=subprocess.PIPE)
            csv_data = thread.stdout
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)

def run_experiment_lock_level_single_threaded():
    for i in range(N_PROGRAM_ITERATIONS):
        for mutex_name in MUTEX_NAMES:
            print(f"{mutex_name=} | {i=}")
            data_file_name = get_data_file_name(mutex_name, i)
            subprocess.run(["rm", "-f", data_file_name])
            command = get_command(mutex_name, csv=True, thread_level=False)
            thread = subprocess.run(command, stdout=subprocess.PIPE)
            csv_data = thread.stdout
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)

# TODO: run_experiment_lock_level_multithreaded