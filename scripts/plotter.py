from math import ceil

import matplotlib.pyplot as plt

from .constants import Constants
from .logger import logger


def finish_plotting_cdf(thread_time_or_lock_time):
    """
    Finalize and show a cumulative distribution (CDF) plot.
    """
    print("Finishing plotting CDF...")
    title = (
        f"{thread_time_or_lock_time} CDF for "
        f"{Constants.bench_n_threads} threads, "
        f"{Constants.bench_n_seconds}s "
        f"({Constants.n_program_iterations}×)"
    )
    if Constants.noncritical_delay != 1:
        title += (
            f"\nNoncritical delay: {Constants.noncritical_delay:,} ns "
            f"({Constants.noncritical_delay:.2e} ns)"
        )
    if Constants.low_contention:
        title += f"\nLow-contention mode: stagger {Constants.stagger_ms} ms/start"
    plt.title(title)
    plt.xscale('log')
    legend = plt.legend()
    for handle in legend.legend_handles:
        handle._sizes = [30]
    plt.show()


def finish_plotting_graph(axis):
    """
    Finalize and show a 2-panel graph: iterations vs threads and std dev vs threads.
    Expects `axis` to be a tuple/list of two Axes objects.
    """
    print("Finishing plotting graph...")
    # Upper plot: total iterations
    axis[0].set_title(
        f"# Iterations vs threads for {Constants.bench_n_seconds}s ({Constants.n_program_iterations}×)"
    )
    axis[0].set_yscale('log')

    # Lower plot: standard deviation
    axis[1].set_title(
        f"Std. dev of # Iterations vs threads for {Constants.bench_n_seconds}s ({Constants.n_program_iterations}×)"
    )
    axis[1].set_yscale('log')

    # Legend sizing
    for ax in axis:
        legend = ax.legend()
        for handle in legend.legend_handles:
            handle._sizes = [30]

    plt.show()


def plot_one_cdf(series, mutex_name, xlabel="", ylabel="", title="",
                 skip=-1, worst_case=-1, average_lock_time=None):
    
    logger.info(f"Plotting CDF for {mutex_name}")
    x_vals = series.sort_values().reset_index(drop=True)
    y_vals = [i / x_vals.size for i in range(x_vals.size)]
    title_label = title + f" ({x_vals.size:,} points)"
    if average_lock_time is not None:
        title_label += f" (avg lock time={average_lock_time:.2e})"

    step = int(ceil(x_vals.size / Constants.max_n_points))
    x = [x_vals[i] for i in range(0, x_vals.size, step)]
    y = [y_vals[i] for i in range(0, x_vals.size, step)]

    if Constants.scatter:
        plt.scatter(x, y, label=title_label, s=0.2)
    else:
        plt.plot(x, y, label=title_label)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)


def plot_one_graph(ax, x, y, mutex_name, xlabel="", ylabel="", title="",
                   skip=-1, worst_case=-1):
    logger.info(f"Plotting graph for {mutex_name}")
    if Constants.scatter:
        ax.scatter(x, y, label=title, s=0.2)
    else:
        ax.plot(x, y, label=title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
