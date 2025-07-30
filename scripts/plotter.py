from math import ceil
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import itertools

from .constants import Constants
from .logger import logger

# Color Changer/Symbols.
MARKERS = itertools.cycle(['o', 's', '^', 'D', '*', 'X', 'v', '<', '>', 'P', 'H'])
coloring = [
    "#e6194b", "#3cb44b", "#ffe119", "#4363d8", "#f58231",
    "#911eb4", "#46f0f0", "#f032e6", "#bcf60c", "#fabebe",
    "#008080", "#e6beff", "#9a6324", "#fffac8", "#800000",
    "#aaffc3", "#808000", "#ffd8b1", "#000075", "#808080",
    "#ffffff", "#000000", "#ff4500", "#00ced1", "#ff69b4",
    "#1e90ff", "#7cfc00", "#ff1493", "#00ff7f", "#dc143c",
    "#8a2be2", "#00bfff", "#ff6347", "#7fffd4", "#d2691e",
    "#6495ed", "#dda0dd", "#f0e68c", "#ffb6c1", "#a52a2a"
]
COLORS = itertools.cycle(coloring)
MUTEX_STYLES = {}

def get_style(mutex_name):
    if mutex_name not in MUTEX_STYLES:
        MUTEX_STYLES[mutex_name] = {
            "marker": next(MARKERS),
            "color": next(COLORS)
        }
    return MUTEX_STYLES[mutex_name]


def finish_plotting_cdf(thread_time_or_lock_time):
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

    legend = plt.legend(loc='center left', bbox_to_anchor=(1, 0.5), fontsize='small')
    for handle in legend.legend_handles:
        handle._sizes = [30]

    plt.tight_layout()
    plt.show()


def finish_plotting_graph(axis, rusage=False):
    print("Finishing plotting...")

    for i, ax in enumerate(axis):
        if rusage:
            if i == 0:
                ax.set_title(f"User time vs threads ({Constants.bench_n_seconds}s, {Constants.n_program_iterations}x)")
            else:
                ax.set_title("System time vs threads")
        else:
            if i == 0:
                ax.set_title(f"# Iterations vs threads ({Constants.bench_n_seconds}s, {Constants.n_program_iterations}x)")
            else:
                ax.set_title("Std. dev of # Iterations vs threads")

        if Constants.log_scale:
            ax.set_yscale("log")
        else:
            ax.set_yscale("linear")

        ax.set_xlabel("Threads")
        ax.set_ylabel("Value")
        ax.grid(True, linestyle='--', linewidth=0.5)

        legend = ax.legend(loc='center left', bbox_to_anchor=(1, 0.5), fontsize='small')
        for handle in legend.legend_handles:
            handle._sizes = [30]

    plt.tight_layout()
    plt.show()


def plot_one_cdf(series, mutex_name, error_bars=None, xlabel="", ylabel="", title="", skip=-1, worst_case=-1, average_lock_time=None):
    logger.info(f"Plotting {mutex_name=}")
    
    x_values = series.sort_values().reset_index(drop=True)
    if x_values.size == 0:
        logger.error(f"Failed to plot {mutex_name}: No data.")
        return

    y_values = np.linspace(0, 1, x_values.size)

    if average_lock_time:
        title += f" (avg={average_lock_time:.2e})"
    title += f" ({x_values.size:,} datapoints)"

    # Subsample if too many points
    skip = max(1, int(ceil(x_values.size / Constants.max_n_points)))
    x = x_values[::skip]
    y = y_values[::skip]

    style = get_style(mutex_name)

    if Constants.scatter:
        plt.scatter(
            x, y,
            label=title,
            s=10,
            color=style["color"],
            marker=style["marker"],
            alpha=0.8
        )
    elif error_bars is not None:
        plt.errorbar(
            x, y, error_bars[::skip],
            label=title,
            color=style["color"],
            marker=style["marker"],
            markersize=4,
            linewidth=0.8,
            alpha=0.8
        )
    else:
        # Smooth line + markers
        plt.plot(
            x, y,
            label=title,
            color=style["color"],
            marker=style["marker"],
            markersize=4,
            linewidth=0.8,
            alpha=0.9,
            markevery=max(1, len(x)//20)
        )

    if Constants.log_scale:
        plt.xscale("log")

    plt.xlabel(xlabel)
    plt.ylabel(ylabel)


def plot_one_graph(ax, x, y, mutex_name, error_bars=None, xlabel="", ylabel="", title="", skip=-1, worst_case=-1, data=None, iter_variable_name=None, colname=None):
    logger.info(f"Plotting {mutex_name=}")

    style = get_style(mutex_name)

    if error_bars is not None and data is not None and iter_variable_name and colname:
        sns.lineplot(
            data=data,
            x=iter_variable_name,
            y=colname,
            errorbar=("sd", 0.1),
            label=title,
            marker=style["marker"],
            color=style["color"]
        )
    elif Constants.scatter:
        ax.scatter(
            x, y,
            label=title,
            s=10,
            color=style["color"],
            marker=style["marker"],
            alpha=0.8
        )
    else:
        ax.plot(
            x, y,
            label=title,
            color=style["color"],
            marker=style["marker"],
            markersize=3,
            linewidth=1.0,
            alpha=0.8
        )

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)

    if Constants.log_scale:
        ax.set_yscale("log")
    else:
        ax.set_yscale("linear")
