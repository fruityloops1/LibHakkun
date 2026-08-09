#!/usr/bin/env python3

import os
import subprocess
import tarfile
import sys

from libsetup_libcxx_arg import *

prepackaged_source_tar_name = make_tar_name(clang_version)
prepackaged_source_uri = f"https://codeberg.org/fruityloops1/LibHakkun/releases/download/stdlib-{llvm_version}/" + prepackaged_source_tar_name
print(prepackaged_source_uri)

root_dir = os.getcwd()

def downloadAndExtractPrepackaged():
    print(f"Downloading pre-packaged stdlib")

    subprocess.run(['curl', '-O', '-L', prepackaged_source_uri])

    print(f"Extracting")

    src_tar = tarfile.open(prepackaged_source_tar_name)
    src_tar.extractall('.')
    src_tar.close()

    os.remove(prepackaged_source_tar_name)

downloadAndExtractPrepackaged()
