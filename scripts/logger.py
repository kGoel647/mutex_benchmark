import os

from .constants import *

# TODO: logger class used throughout program, probably standard library logger

def log(output):
    # Saving data
    log_file_index = 0
    log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"
    while os.path.isfile(log_file_name):
        log_file_index += 1
        log_file_name = f"{LOGS_FOLDER}/{log_file_index}.log"

    with open(log_file_name, "w") as log_file:
        log_file.write(output)