from .constants import *
from .logger import logger

import matplotlib.pyplot as plt

def finish_plotting_cdf(thread_time_or_lock_time):
    print("Finishing plotting...")
    title = f"{thread_time_or_lock_time} CDF for {Constants.bench_n_threads} threads and {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)"
    if Constants.scatter and Constants.skip != 1:
        title += f"\nEach dot represents the average of {Constants.skip} operations"
    plt.title(title)
    plt.xscale('log')
    legend = plt.legend()
    for handle in legend.legend_handles:
        handle._sizes = [30]
    plt.show()

def finish_plotting_graph(axis):
    print("Finishing plotting...")
    axis[0].set_title(f"# Iterations v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")
    axis[0].set_yscale('log')

    axis[1].set_title(f"Std. dev of # Iterations v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")
    axis[1].set_yscale('log')
    # plt.xscale('log')
    legend = axis[0].legend()
    for handle in legend.legend_handles:
        handle._sizes = [30]

    legend = axis[1].legend()
    for handle in legend.legend_handles:
        handle._sizes = [30]
    plt.show()

def plot_one_cdf(series, mutex_name, xlabel="", ylabel="", title="", skip=-1, worst_case=-1):
    if skip == -1:
        skip = Constants.skip
    logger.info(f"Plotting {mutex_name=}")
    # The y-values should go up from 0 to 1, while the X-values vary along the series
    x_values = series.sort_values().reset_index(drop=True)
    y_values = [a/x_values.size for a in range(x_values.size)]
    # Skip some values to save time
    x = [x_values[i] for i in range(0, x_values.size, skip)]
    y = [y_values[i] for i in range(0, x_values.size, skip)]

    if Constants.scatter:
        plt.scatter(x, y, label=title, s=0.2)
    else:
        plt.plot(x, y, label=title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

def plot_one_graph(ax, x, y, mutex_name, xlabel="", ylabel="", title="", skip=-1, worst_case=-1):
    if skip == -1:
        skip = Constants.skip
    logger.info(f"Plotting {mutex_name=}")
    if Constants.scatter:
        ax.scatter(x, y, label=title, s=0.2)
    else:
        ax.plot(x, y, label=title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)