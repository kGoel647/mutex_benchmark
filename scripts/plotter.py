from math import ceil

import matplotlib.pyplot as plt
import seaborn as sns

from .constants import Constants
from .logger import logger


def finish_plotting_cdf(thread_time_or_lock_time):
    """
    Finalize and show a cumulative distribution (CDF) plot.
    """
    if Constants.skip_plotting:
        return
    print("Finishing plotting CDF...")
    title = (
        f"{thread_time_or_lock_time} CDF for "
        f"{Constants.bench_n_threads} threads, "
        f"{Constants.bench_n_seconds}s "
        f"({Constants.n_program_iterations}×)"
    )
    if Constants.noncritical_delay != -1:
        title += (
            f"\nNoncritical delay: {Constants.noncritical_delay:,} ns "
            f"({Constants.noncritical_delay:.2e} ns)"
        )
    if Constants.low_contention:
        title += f"\nLow-contention mode: stagger {Constants.stagger_ms} ms/start"
    plt.title(title)
    if Constants.log_scale and not Constants.iter:
        plt.xscale('log')
    legend = plt.legend()
    for handle in legend.legend_handles: # type: ignore
        handle._sizes = [30]
    plt.show()

def finish_plotting_graph(axis, rusage=False):
    if Constants.skip_plotting:
        return
    print("Finishing plotting...")
    if not(rusage):
        axis[0].set_title(f"# Iterations v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")
        axis[0].set_yscale('log')
    else:
        axis[0].set_title(f"# User time v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")

    if len(axis)>1:
        if not(rusage):
            axis[1].set_title(f"Std. dev of # Iterations v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")
            axis[1].set_yscale('log')
        else:
            axis[1].set_title(f"System time v threads for {Constants.bench_n_seconds} seconds ({Constants.n_program_iterations}x)")

    for ax in axis:
      legend = ax.legend()
      for handle in legend.legend_handles:
          handle._sizes = [30]

    plt.show()

def plot_one_cdf(series, mutex_name, error_bars=None, xlabel="", ylabel="", title="", skip=-1, worst_case=-1, average_lock_time=None):
    if Constants.skip_plotting:
        return
    logger.info(f"Plotting {mutex_name=}")
    # The y-values should go up from 0 to 1, while the X-values vary along the series
    x_values = series.sort_values().reset_index(drop=True)
    y_values = [a/x_values.size for a in range(x_values.size)]
    title += f" ({x_values.size:,} datapoints)"
    if average_lock_time:
        title += f" ({average_lock_time=:.2e})"
    # Skip some values to save time
    logger.info(x_values.size)
    skip = int(ceil(x_values.size / Constants.max_n_points))

    if (x_values.size == 0):
        logger.error(f"Failed to plot {mutex_name}: No data.")
        return

    x = [x_values[i] for i in range(0, x_values.size, skip)]
    y = [y_values[i] for i in range(0, x_values.size, skip)]

    if Constants.scatter:
        plt.scatter(x, y, label=title, s=0.2)
        plt.xscale("log")
    elif error_bars is not None:
        plt.errorbar(x, y, error_bars, label=title)
    else:
        plt.plot(x, y, label=title)
        plt.xscale("log")
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

def plot_one_graph(ax, x, y, mutex_name, error_bars=None, xlabel="", ylabel="", title="", skip=-1, worst_case=-1, data=None, iter_variable_name=None, colname=None):
    if Constants.skip_plotting:
        return
    logger.info(f"Plotting {mutex_name=}")
    # print(data)
    if error_bars is not None:
        # data = data.sample(1000)
        # color = sns.color_palette()
        print(data)
        logger.debug(dict(data=data, x=iter_variable_name, y=colname, errorbar=("sd", 0.1), label=title))
        grid = sns.lineplot(data=data, x=iter_variable_name, y=colname, errorbar=("sd", 0.1), label=title)
        grid.set(yscale="log")
        # sns.scatterplot(data=data, x="threads", y="Time Spent", palette=color)
        return
        # ax.errorbar(x, y, error_bars, marker='o', capsize=5, capthick=1, label=title)
    elif Constants.scatter:
        ax.scatter(x, y, label=title, s=0.2)
    else:
        ax.plot(x, y, label=title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
