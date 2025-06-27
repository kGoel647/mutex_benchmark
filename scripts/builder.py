from .constants import *

import subprocess

def build():
    subprocess.run(f"mkdir build data {DATA_FOLDER} {LOGS_FOLDER} -p".split())

    # Compile
    subprocess.run("meson setup build".split(), stdout=subprocess.DEVNULL)
    subprocess.run("meson compile -C build".split(), stdout=subprocess.DEVNULL)