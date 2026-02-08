#!/usr/bin/env python3

import os
import shutil
import multiprocessing
import subprocess

root_dir = os.getcwd()
project_dir = f'{root_dir}/sys/hakkun/sail'
build_dir = f'{project_dir}/build'

def compileSail():
    subprocess.run(['cmake', '-S', project_dir, '-B', build_dir])
    subprocess.run(["cmake", "--build", build_dir, "-j", f"{multiprocessing.cpu_count()}"])

try:
    shutil.rmtree(build_dir)
except FileNotFoundError:
    pass
os.makedirs(build_dir)
compileSail()
