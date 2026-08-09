import sys

is_aarch32 = len(sys.argv) > 1 and sys.argv[1] == 'aarch32'
is_ounce = len(sys.argv) > 2 and sys.argv[2] == 'ounce'

target = 'armv7-none-eabi' if is_aarch32 else 'aarch64-none-elf'
arch = "arm" if is_aarch32 else "aarch64"
platform = "Ounce" if is_ounce else "NX"

musl_ver = 'musl-1.2.5'
llvm_version = '19.1.7'
clang_version = '19.1.7'

def make_tar_name(clang_version: str) -> str:
    return f'stdlib_{musl_ver}_llvm-{llvm_version}_clang-{clang_version}_{arch}_{platform}.tar.xz'