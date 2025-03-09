#pragma once

#include "elf.h"

#ifdef __aarch64__
using Elf_Addr = Elf64_Addr;
using Elf_Rel = Elf64_Rel;
using Elf_Rela = Elf64_Rela;
using Elf_Dyn = Elf64_Dyn;
using Elf_Sym = Elf64_Sym;
using Elf_Xword = Elf64_Xword;

#define ELF_R_SYM ELF64_R_SYM
#define ELF_R_TYPE ELF64_R_TYPE
#define ELF_ST_BIND ELF64_ST_BIND
#define ELF_ST_VISIBILITY ELF64_ST_VISIBILITY

#define ARCH_RELATIVE R_AARCH64_RELATIVE
#define ARCH_JUMP_SLOT R_AARCH64_JUMP_SLOT
#define ARCH_GLOB_DAT R_AARCH64_GLOB_DAT
#define ARCH_IS_REL_ABSOLUTE(type) \
    (type == R_AARCH64_ABS32 || type == R_AARCH64_ABS64)
#else
using Elf_Addr = Elf32_Addr;
using Elf_Rel = Elf32_Rel;
using Elf_Rela = Elf32_Rela;
using Elf_Dyn = Elf32_Dyn;
using Elf_Sym = Elf32_Sym;
using Elf_Xword = Elf32_Xword;

#define ELF_R_SYM ELF32_R_SYM
#define ELF_R_TYPE ELF32_R_TYPE
#define ELF_ST_BIND ELF32_ST_BIND
#define ELF_ST_VISIBILITY ELF32_ST_VISIBILITY

#define ARCH_RELATIVE R_AARCH64_RELATIVE
#define ARCH_JUMP_SLOT R_AARCH64_JUMP_SLOT
#define ARCH_GLOB_DAT R_AARCH64_GLOB_DAT
#define ARCH_IS_REL_ABSOLUTE(type) \
    (type == R_AARCH64_P32_ABS32)
#endif
