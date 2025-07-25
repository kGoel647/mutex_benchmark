from .constants import *

import subprocess
import os

#TODO: Make this more robust. Add option to use existing build directory.
def build():
    subprocess.run(f"mkdir build data {Constants.Defaults.DATA_FOLDER} {Constants.Defaults.LOGS_FOLDER}".split()) 

    # Compile
    result = subprocess.run("meson setup build".split())#, stdout=subprocess.DEVNULL)
    assert result.returncode == 0, "Meson build failed."
    subprocess.run('meson configure build --buildtype debugoptimized'.split())#, stdout=subprocess.DEVNULL)
    result = subprocess.run("meson compile -C build".split())#, stdout=subprocess.DEVNULL)
    assert result.returncode == 0, "Compilation failed."