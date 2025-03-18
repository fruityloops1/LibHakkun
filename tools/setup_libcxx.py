#!/usr/bin/env python3

import os
import subprocess
import tarfile
import zipfile
import multiprocessing
import importlib.util
import shutil
import urllib.request
import sys

is_aarch32 = len(sys.argv) > 1 and sys.argv[1] == 'aarch32'

target = 'armv7-none-eabi' if is_aarch32 else 'aarch64-none-elf'

musl_ver = 'musl-1.2.4'
musl_source_tar_name = musl_ver + '.tar.gz'
musl_source = "https://musl.libc.org/releases/" + musl_source_tar_name

llvm_source = "https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-19.1.0.zip"

root_dir = os.getcwd()

def downloadAndCompileMusl():
    print(f"Downloading musl")

    os.chdir(f'{root_dir}/lib/std')
    subprocess.run(['curl', '-O', musl_source])

    print(f"Extracting musl")

    src_tar = tarfile.open(musl_source_tar_name)
    src_tar.extractall('.')
    src_tar.close()

    os.remove(musl_source_tar_name)

    print(f"Building musl")

    os.rename(musl_ver, 'musl')
    os.chdir('musl')

    env = os.environ.copy()
    env["CC"] = "clang"
    env["AR"] = "llvm-ar"
    env["RANLIB"] = "llvm-ranlib"
    env["LIBCC"] = " "
    env["CFLAGS"] = f"-ffunction-sections -fdata-sections -flto -target {target} -march={"armv7a" if is_aarch32 else "armv8-a"} -mtune=cortex-a57 {"-mfloat-abi=hard" if is_aarch32 else ""}"

    subprocess.run(["./configure", "--disable-shared", "--target", target], env=env)

    empty_files = [
        'src/malloc/mallocng/malloc.c',
        'src/malloc/mallocng/aligned_alloc.c',
        'src/malloc/mallocng/free.c',
        'src/malloc/mallocng/realloc.c',
        'src/malloc/oldmalloc/aligned_alloc.c',
        'src/malloc/oldmalloc/malloc.c',
        'src/malloc/free.c',
        'src/malloc/realloc.c',
        'src/malloc/calloc.c',
        'src/malloc/replaced.c',
        'src/malloc/reallocarray.c',
        'src/malloc/posix_memalign.c',
        'src/malloc/memalign.c',
        'src/malloc/lite_malloc.c',
        'src/malloc/lite_calloc.c',
    ]

    for file in empty_files:
        with open(file, 'w') as f:
            f.write('')

    subprocess.run(["make", "-j", f"{multiprocessing.cpu_count()}"])

    os.chdir(root_dir)

def downloadAndCompileLibcxxLibcxxabiLibunwindCompilerRt():
    print(f"Downloading LLVM")

    os.chdir(f'{root_dir}/lib/std')
    llvm_zip_name = llvm_source.split('/')[-1]
    subprocess.run(['curl', '-O', '-L', llvm_source])

    print(f"Extracting LLVM")
    
    with zipfile.ZipFile(llvm_zip_name, 'r') as zip_ref:
        zip_ref.extractall('.')

    os.rename(f'llvm-project-{llvm_zip_name.removesuffix('.zip')}', 'llvm-project')

    os.remove(llvm_zip_name)

    os.chdir('llvm-project')

    build_dir = 'build'
    try:
        shutil.rmtree(build_dir)
    except FileNotFoundError:
        pass
    os.makedirs(build_dir)

    musl_path = f"{os.getcwd()}/../musl"

    c_flags = f"""
        {"-mfloat-abi=hard" if is_aarch32 else ""}
        -flto
        -ffunction-sections
        -fdata-sections
        -target {target}
        -D_POSIX_C_SOURCE=200809L
        -D_LIBUNWIND_IS_BAREMETAL
        -D_LIBCPP_HAS_THREAD_API_PTHREAD
        -D_GNU_SOURCE
        -isystem {musl_path}/include
        -isystem {musl_path}/obj/include
        -isystem {musl_path}/arch/generic
        -isystem {musl_path}/arch/{"arm" if is_aarch32 else "aarch64"}
        -DNDEBUG
        -D_LIBCXXABI_NO_EXCEPTIONS
        -D_LIBCPP_HAS_NO_EXCEPTIONS
        """.replace('\n', '')

    cmake_setup_args = [
        'cmake', '-G', 'Ninja', '-S', 'runtimes', '-B', build_dir,
        f'-DCMAKE_INSTALL_PREFIX={build_dir}',
        '-DLLVM_ENABLE_RUNTIMES="libc;libcxx;libcxxabi;libunwind;compiler-rt"',
        f'-DCMAKE_C_COMPILER_TARGET={target}',
        f'-DCMAKE_CXX_COMPILER_TARGET={target}',
        '-DCMAKE_C_COMPILER=clang',
        '-DCMAKE_CXX_COMPILER=clang++',
        '-DCMAKE_C_COMPILER_WORKS=TRUE',
        '-DCMAKE_CXX_COMPILER_WORKS=TRUE',
        '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
        '-DLIBCXX_ENABLE_SHARED=OFF',
        '-DLIBCXXABI_ENABLE_SHARED=OFF',
        '-DLIBUNWIND_ENABLE_SHARED=OFF',
        '-DLIBCXX_INSTALL_LIBRARY=OFF',
        '-DLIBCXX_INSTALL_HEADERS=OFF',
        '-DLIBCXX_HAS_MUSL_LIBC=ON',
        '-DLIBCXXABI_BAREMETAL=ON',
        '-DCAN_TARGET_aarch64=TRUE',
        '-DCAN_TARGET_armv7=TRUE',
        '-DCOMPILER_RT_BUILD_BUILTINS=ON',
        '-DCOMPILER_RT_BUILD_LIBFUZZER=OFF',
        '-DCOMPILER_RT_BUILD_MEMPROF=OFF',
        '-DCOMPILER_RT_BUILD_PROFILE=OFF',
        '-DCOMPILER_RT_BUILD_SANITIZERS=OFF',
        '-DCOMPILER_RT_BUILD_XRAY=OFF',
        '-DCOMPILER_RT_BUILD_ORC=OFF',
        '-DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON',
        '-DLLVM_LIBRARY_OUTPUT_INTDIR=OFF',
        '-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF',
        f'-DCMAKE_C_FLAGS="{c_flags}"',
        f'-DCMAKE_CXX_FLAGS="{c_flags}"',
        f'-DCMAKE_ASM_FLAGS="-target {target}"',
    ]

    cmake_build_args = [
        'ninja', '-C', build_dir,
        'compiler-rt', 'unwind', 'cxx', 'cxxabi'
    ]

    print(' '.join(cmake_setup_args))
    os.system(' '.join(cmake_setup_args)) # idk
    print(' '.join(cmake_build_args))
    subprocess.run(cmake_build_args)

    shutil.copyfile(f'{build_dir}/lib/libc++.a', f'{root_dir}/lib/std/libc++.a')
    shutil.copyfile(f'{build_dir}/lib/libc++abi.a', f'{root_dir}/lib/std/libc++abi.a')
    shutil.copyfile(f'{build_dir}/lib/libunwind.a', f'{root_dir}/lib/std/libunwind.a')
    if is_aarch32:
        shutil.copyfile(f'{build_dir}/compiler-rt/lib/linux/libclang_rt.builtins-arm.a', f'{root_dir}/lib/std/libclang_rt.builtins-arm.a')
    else:
        shutil.copyfile(f'{build_dir}/compiler-rt/lib/linux/libclang_rt.builtins-aarch64.a', f'{root_dir}/lib/std/libclang_rt.builtins-aarch64.a')
    shutil.copyfile(f'{musl_path}/lib/libc.a', f'{root_dir}/lib/std/libc.a')
    shutil.copyfile(f'{musl_path}/lib/libm.a', f'{root_dir}/lib/std/libm.a')

    os.chdir(root_dir)

try:
    shutil.rmtree(f'{root_dir}/lib/std')
except FileNotFoundError:
    pass
os.makedirs(f'{root_dir}/lib/std')
downloadAndCompileMusl()
downloadAndCompileLibcxxLibcxxabiLibunwindCompilerRt()
