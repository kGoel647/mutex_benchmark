# scripts/runner.py

from .constants import Constants
from .logger    import logger
import subprocess


def get_data_file_name(mutex_name, i, **kwargs):
    name_root = f"{Constants.data_folder}/{mutex_name}-{i}"
    for name, value in kwargs.items():
        name_root += f"-{name}={value}"
    name = name_root + ".csv"
    return name

def get_command(mutex_name, *, threads=None, csv=True, thread_level=False, critical_delay=None, noncritical_delay=None):
    if threads is None:
        threads = Constants.bench_n_threads
    if critical_delay is None:
        critical_delay = Constants.critical_delay
    if noncritical_delay is None:
        noncritical_delay = Constants.noncritical_delay
    cmd = [
        Constants.Defaults.EXECUTABLE, 
        mutex_name, 
        str(threads), 
        str(Constants.bench_n_seconds), 
        str(critical_delay), 
        str(noncritical_delay),
    ]
    if csv:
        cmd.append("--csv")
    if thread_level:
        cmd.append("--thread-level")

    if Constants.low_contention:
        cmd.append("--low-contention")
        if Constants.stagger_ms and Constants.stagger_ms > 0:
            cmd += ["--stagger-ms", str(Constants.stagger_ms)]

    logger.debug(f"Bench command: {cmd}")
    return cmd

def run_experiment_multi_threaded():
    for mutex_name in Constants.mutex_names:
        procs = []
        for i in range(Constants.n_program_iterations):
            fname = get_data_file_name(mutex_name, i, Constants.bench_n_threads)
            subprocess.run(["rm", "-f", fname])
            p = subprocess.Popen(
                get_command(
                    mutex_name,
                    Constants.bench_n_threads,
                    csv=True,
                    thread_level=True
                ),
                stdout=subprocess.PIPE
            )
            procs.append((fname, p))

        for fname, p in procs:
            p.wait()
            output = p.stdout.read()
            with open(fname, "wb") as f:
                f.write(output)

def run_experiment_single_threaded():
    for i in range(Constants.n_program_iterations):
        for mutex_name in Constants.mutex_names:
        # Create program threads
            threads = []
            logger.info(f"{mutex_name=} | {i=}")
            data_file_name = get_data_file_name(mutex_name, i)
            subprocess.run(["rm", "-f", data_file_name])
            command = get_command(mutex_name, csv=True, thread_level=True)
            thread = subprocess.run(command, stdout=subprocess.PIPE)
            csv_data = thread.stdout
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)

def run_experiment_lock_level_single_threaded():
    for i in range(Constants.n_program_iterations):
        for mutex_name in Constants.mutex_names:
            logger.info(f"{mutex_name=} | {i=}")
            data_file_name = get_data_file_name(mutex_name, i)
            subprocess.run(["rm", "-f", data_file_name])
            command = get_command(mutex_name, csv=True, thread_level=False)
            thread = subprocess.run(command, stdout=subprocess.PIPE)
            csv_data = thread.stdout
            with open(data_file_name, "wb") as data_file:
                data_file.write(csv_data)


def run_experiment_iter_v_threads_single_threaded():
    # TODO replace with generic "run_experiment_iter_single_threaded"
    for threads in range(*Constants.iter_threads):
        for i in range(Constants.n_program_iterations):
            for mutex_name in Constants.mutex_names:
                logger.info(f"{mutex_name=} | {threads=} | {i=}")
                data_file_name = get_data_file_name(mutex_name, i)
                subprocess.run(["rm", "-f", data_file_name])
                command = get_command(mutex_name, threads=threads, csv=True, thread_level=True)
                thread=subprocess.run(command, stdout=subprocess.PIPE)
                csv_data = thread.stdout
                with open(data_file_name, "wb") as data_file:
                    data_file.write(csv_data)