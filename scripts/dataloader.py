from .constants import *

import pandas as pd

# Load data from CSV into dictionary of Pandas dataframes

def load_data():
    data = {}
    for mutex_name in MUTEX_NAMES:
        dataframes = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
            dataframe = pd.read_csv(data_file_name, names=["Thread #", "# Iterations", "Time Spent"])
            dataframes.append(dataframe)
        data[mutex_name] = pd.concat(dataframes)
    return data

def load_data_lock_level():
    data = {}
    for mutex_name in MUTEX_NAMES:
        dataframes = []
        for i in range(N_PROGRAM_ITERATIONS):
            data_file_name = f"{DATA_FOLDER}/{mutex_name}-{i}.csv"
            dataframe = pd.read_csv(data_file_name, names=["Thread ID", "Iteration #", "Time Spent"])
            dataframes.append(dataframe)
        data[mutex_name] = pd.concat(dataframes)
    return data
