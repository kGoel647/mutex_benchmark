from .constants import *

import matplotlib.pyplot as plt

def finish_plotting_cdf(thread_time_or_lock_time):
    print("Finishing plotting...")
    plt.title(f"{thread_time_or_lock_time} CDF for {BENCH_N_THREADS} threads and {BENCH_N_ITERATIONS} iterations ({N_PROGRAM_ITERATIONS}x)")
    plt.xscale('log')
    plt.legend()
    plt.show()

def plot_one_cdf(series, mutex_name, xlabel="", ylabel="", title="", skip=SKIP, worst_case=-1):
    print(f"Plotting {mutex_name=}")
    # The y-values should go up from 0 to 1, while the X-values vary along the series
    x_values = series.sort_values().reset_index(drop=True)
    y_values = [a/x_values.size for a in range(x_values.size)]
    # Skip some values to save time
    x = [x_values[i] for i in range(0, x_values.size, 1 + skip)]
    y = [y_values[i] for i in range(0, x_values.size, 1 + skip)]

    if SCATTER:
        plt.scatter(x, y, label=title, s=0.2)
    else:
        plt.plot(x, y, label=title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)