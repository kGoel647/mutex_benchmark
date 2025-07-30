from .constants import *
from .runner import get_data_file_name
from .logger import logger

import pandas as pd

# Load data from CSV into dictionary of Pandas dataframes

def load_data():
    data = {}
    for mutex_name in Constants.mutex_names:
        dataframes = []
        for i in range(Constants.n_program_iterations):
            data_file_name = get_data_file_name(mutex_name, i)
            dataframe = pd.read_csv(data_file_name, names=["Thread #", "Seconds", "# Iterations"])
            dataframes.append(dataframe)
        data[mutex_name] = pd.concat(dataframes)
    return data

def print_averages_lock_level():
    CHUNKSIZE = 1_000_000
    logger.debug("print_averages_lock_level running...")
    info = []
    for mutex_name in Constants.mutex_names:
        total_average = 0
        total_count = 0
        for i in range(Constants.n_program_iterations):
            data_file_name = get_data_file_name(mutex_name, i)
            for i, df in enumerate(pd.read_csv(data_file_name, names=["Thread ID", "Iteration #", "Time Spent"], chunksize=CHUNKSIZE)):
                current_average = df["Time Spent"].mean()
                current_count = df["Time Spent"].size
                logger.debug(f"\tMutex {mutex_name:<20}: loaded chunk #{i:0>3}... {current_average=:.12f} | {current_count=:>13}")
                total_average = (total_average * total_count + current_average * current_count) / (total_count + current_count)
                total_count += current_count
        info.append(f"Mutex {mutex_name:>20}: mean time spent: {total_average:.12f} | datapoint count: {total_count:>13}")
        logger.info(info[-1])
    print('\n'.join(info))
    logger.debug("print_averages_lock_level done.")

def load_data_lock_level():
    logger.debug("load_data_lock_level running...")
    data = {}
    for mutex_name in Constants.mutex_names:
        dataframes = []
        for i in range(Constants.n_program_iterations):
            data_file_name = get_data_file_name(mutex_name, i)
            dataframe = pd.read_csv(data_file_name, names=["Thread ID", "Iteration #", "Time Spent"])
            dataframes.append(dataframe)
        data[mutex_name] = pd.concat(dataframes)
    logger.debug("load_data_lock_level done.")
    return data

# def load_data_iter_v_threads():
#     if (Constants.iter_threads)
#     data = {}
#     for mutex_name in Constants.mutex_names:
#         data[mutex_name]=[]
#         for threads in range(*Constants.iter_threads):
#             dataframes=[]
#             for i in range(Constants.n_program_iterations):
#                 data_file_name = get_data_file_name(mutex_name, i, threads=threads)
#                 dataframe = pd.read_csv(data_file_name, names=["Thread ID", "Seconds", "# Iterations"])
#                 dataframes.append(dataframe)
#             dataframes=pd.concat(dataframes)
#             data[mutex_name].append(dataframes["# Iterations"])
#     return data

def get_column_names(rusage):
    if rusage:
        return ["utime", "stime", "maxrss", "ru_minflt", "ru_majflt"]
    else:
        return ["Thread ID", "Seconds", "# Iterations"]

def load_data_iter(iter_variable_name, iter_range, rusage=False):
    data = {}
    for mutex_name in Constants.mutex_names:
        data[mutex_name]=[]
        for iter_variable_value in range(*iter_range):
            extra_command_args = {iter_variable_name:iter_variable_value, "rusage":rusage}
            dataframes=[]
            for i in range(Constants.n_program_iterations):
                data_file_name = get_data_file_name(mutex_name, i, **extra_command_args)
                dataframe = pd.read_csv(data_file_name, names=get_column_names(rusage))
                dataframes.append(dataframe)
            dataframes=pd.concat(dataframes)
            dataframes[iter_variable_name] = iter_variable_value
            data[mutex_name].append(dataframes)
        data[mutex_name] = pd.concat(data[mutex_name])
    return data
