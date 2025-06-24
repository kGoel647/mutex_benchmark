import pandas as pd
import subprocess
import os

MUTEX_NAMES = ["pthread", "cpp_std", "boost"]
EXECUTABLE_NAME = "max_contention_bench"
BENCH_N_THREADS = "10"
BENCH_N_ITERATIONS = "1000"
N_PROGRAM_ITERATIONS = 1000
DATA_FOLDER = "./data/generated"
LOGS_FOLDER = "./data/logs"
EXECUTABLE = f"./build/apps/{EXECUTABLE_NAME}/{EXECUTABLE_NAME}"

# Make sure the script is being run from the right location (in mutex_benchmark directory)
absolute_path = os.path.abspath(__file__)
parent_directory = os.path.dirname(absolute_path)
os.chdir(parent_directory)

subprocess.run("mkdir build data -p".split())

# Compile
subprocess.run("meson setup build".split(), stdout=subprocess.DEVNULL)
subprocess.run("meson compile -C build".split(), stdout=subprocess.DEVNULL)

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

# Load data from CSV into dictionary of Pandas dataframes
data = {}
for mutex_name in MUTEX_NAMES:
    dataframes = []
    for i in range(N_PROGRAM_ITERATIONS):
        data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
        dataframe = pd.read_csv(data_file_name, names=["Thread #", "# Iterations", "Time Spent"])
        dataframes.append(dataframe)
    data[mutex_name] = pd.concat(dataframes)

# Analysis
# TODO add more stuff here
output = ""
for mutex_name in MUTEX_NAMES:
    output += f"Mutex {mutex_name} average time: {data[mutex_name]['Time Spent'].mean()}\n"
print(output)

# Saving data
log_file_index = 0
log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"
while os.path.isfile(log_file_name):
    log_file_index += 1
    log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"

with open(log_file_name, "w") as log_file:
    log_file.write(output)
