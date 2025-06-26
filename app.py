import pandas as pd
import subprocess
import os
import sys
import matplotlib.pyplot as plt

MUTEX_NAMES = ["pthread", "cpp_std", "boost", "nsync", "dijkstra", "spin", "exp_spin"]
EXECUTABLE_NAME = "max_contention_bench"
BENCH_N_THREADS = "100"
BENCH_N_ITERATIONS = "100"
N_PROGRAM_ITERATIONS = 10
DATA_FOLDER = "./data/generated"
LOGS_FOLDER = "./data/logs"
EXECUTABLE = f"./build/apps/{EXECUTABLE_NAME}/{EXECUTABLE_NAME}"
MULTITHREADED = False

# def set_constants():
#     for arg in sys.argv:


def setup():
    # Make sure the script is being run from the right location (in mutex_benchmark directory)
    absolute_path = os.path.abspath(__file__)
    parent_directory = os.path.dirname(absolute_path)
    os.chdir(parent_directory)

def build():
    subprocess.run(f"mkdir build data {DATA_FOLDER} {LOGS_FOLDER} -p".split())

    # Compile
    subprocess.run("meson setup build".split(), stdout=subprocess.DEVNULL)
    subprocess.run("meson compile -C build".split(), stdout=subprocess.DEVNULL)

def run_experiment_multithreaded():
    # Run experiment
    # print("Running programs...")
    for mutex_name in MUTEX_NAMES:
        # Create program threads
        threads = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
            subprocess.run(["rm", "-f", data_file_name])
            thread = subprocess.Popen([EXECUTABLE, mutex_name, BENCH_N_THREADS, BENCH_N_ITERATIONS, "--csv"], stdout=subprocess.PIPE)
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
    # print("Running programs...")
    for mutex_name in MUTEX_NAMES:
        # Create program threads
        threads = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
            subprocess.run(["rm", "-f", data_file_name])
            thread = subprocess.run([EXECUTABLE, mutex_name, BENCH_N_THREADS, BENCH_N_ITERATIONS, "--csv"], stdout=subprocess.PIPE)
            csv_data = thread.stdout
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)

def load_data():
    # Load data from CSV into dictionary of Pandas dataframes
    data = {}
    for mutex_name in MUTEX_NAMES:
        dataframes = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
            dataframe = pd.read_csv(data_file_name, names=["Thread #", "# Iterations", "Time Spent"])
            dataframes.append(dataframe)
        data[mutex_name] = pd.concat(dataframes)
    return data

def analyze(data):
    # Analysis
    # TODO add more stuff here
    output = ""
    output += f"{BENCH_N_THREADS=} | {BENCH_N_ITERATIONS=} | {N_PROGRAM_ITERATIONS=}\n"
    output += "\n"
    # worst_case = max([max(data[mutex_name]["Time Spent"]) for mutex_name in MUTEX_NAMES])
    # output += f"{worst_case=}\n";

    for mutex_name in MUTEX_NAMES:
        output += f"Mutex {mutex_name:>8} average time: {data[mutex_name]['Time Spent'].mean():.7f} standard deviation: {data[mutex_name]['Time Spent'].std():.7f}\n"
        plot_cdf(
            data[mutex_name]["Time Spent"], 
            mutex_name,
            xlabel="Thread time (seconds)",
            ylabel="% of threads under",
            title=f"{mutex_name}",
            skip=0,
            # worst_case=worst_case
        )

    plt.xscale('log')
    plt.legend()
    plt.show()
    return output

def log(output):
    # Saving data
    log_file_index = 0
    log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"
    while os.path.isfile(log_file_name):
        log_file_index += 1
        log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"

    with open(log_file_name, "w") as log_file:
        log_file.write(output)

def plot_cdf(series, mutex_name, xlabel="", ylabel="", title="", skip=0, worst_case=-1):
    # The y-values should go up from 0 to 1, while the X-values vary along the series
    x_values = series.sort_values().reset_index(drop=True)
    y_values = [a/x_values.size for a in range(x_values.size)]
    # Skip some values to save time
    x = [x_values[i] for i in range(0, x_values.size, 1 + skip)]
    y = [y_values[i] for i in range(0, x_values.size, 1 + skip)]

    plt.plot(x, y, label=title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)


def main():
    setup()
    build()
    if MULTITHREADED:
        run_experiment_multi_threaded()
    else:
        run_experiment_single_threaded()
    data = load_data()
    output = analyze(data)
    print(output)
    log(output)

if __name__ == "__main__":
    main()