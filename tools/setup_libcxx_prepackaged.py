#!/usr/bin/env python3

import os
import subprocess
import tarfile
import sys

is_aarch32 = len(sys.argv) > 1 and sys.argv[1] == 'aarch32'

prepackaged_source_tar_name = "stdlib-aarch32-19.1.0_clang_19.1.7.tar.xz" if is_aarch32 else "stdlib-19.1.0_clang_19.1.7.tar.xz"
prepackaged_source = "https://github.com/fruityloops1/LibHakkun/releases/download/stdlib-19.1.0-2/" + prepackaged_source_tar_name

root_dir = os.getcwd()

def downloadAndCompilePrepackaged():
    print(f"Downloading pre-packaged stdlib")

    subprocess.run(['curl', '-O', '-L', prepackaged_source])

    print(f"Extracting")

    src_tar = tarfile.open(prepackaged_source_tar_name)
    src_tar.extractall('.')
    src_tar.close()

    os.remove(prepackaged_source_tar_name)

downloadAndCompilePrepackaged()
