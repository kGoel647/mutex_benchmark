from .constants import *
from .runner import *
from .dataloader import *
from .analyzer import *
from .builder import *
from .logger import log

import os

# TODO: add argument parsing to replace constants

def setup():
    # Make sure the script is being run from the right location (in mutex_benchmark directory)
    absolute_path = os.path.abspath(__file__)
    parent_directory = os.path.dirname(absolute_path)
    os.chdir(parent_directory + "/..")

def run_experiment_thread_level():
    if MULTITHREADED:
        run_experiment_multi_threaded()
    else:
        run_experiment_single_threaded()
    data = load_data()
    output = analyze(data)
    print(output)
    log(output)

def run_experiment_lock_level():
    run_experiment_lock_level_single_threaded()
    data = load_data_lock_level()
    output = analyze_lock_level(data)
    print(output)
    log(output)


def main():
    setup()
    build()
    if THREAD_LEVEL:
        run_experiment_thread_level()
    else:
        run_experiment_lock_level()

if __name__ == "__main__":
    main()