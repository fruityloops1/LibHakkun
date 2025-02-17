#!/usr/bin/env python3

import os
import shutil
import multiprocessing
import subprocess

root_dir = os.getcwd()
build_dir = f'{root_dir}/sys/hakkun/sail/build'

def compileSail():
    subprocess.run(['cmake', '..'])
    subprocess.run(["make", "-j", f"{multiprocessing.cpu_count()}"])

try:
    shutil.rmtree(build_dir)
except FileNotFoundError:
    pass
os.makedirs(build_dir)
os.chdir(build_dir)
compileSail()
