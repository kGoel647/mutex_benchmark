from .constants import *

import subprocess
import os

def build():
    subprocess.run(f"mkdir build data {Constants.Defaults.DATA_FOLDER} {Constants.Defaults.LOGS_FOLDER} -p".split())

    # Compile
    result = subprocess.run("meson setup build".split())#, stdout=subprocess.DEVNULL)
    assert result.returncode == 0, "Meson build failed."
    configure_command = "meson configure build --buildtype debugoptimized"
    cpp_args = []
    if Constants.cxl:
        cpp_args.append("-Dcxl")
    cpp_args.append('-mwaitpkg')
    configure_command += ' -Dcpp_args="' + ' '.join(cpp_args) + '"'
    result = subprocess.run(configure_command.split())
    assert result.returncode == 0, "Configuration failed." #, stdout=subprocess.DEVNULL)
    result = subprocess.run("meson compile -C build".split())#, stdout=subprocess.DEVNULL)
    assert result.returncode == 0, "Compilation failed."