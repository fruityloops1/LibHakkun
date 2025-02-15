#include "hk/sail/init.h"
#include "hk/diag/diag.h"
#include "hk/init/module.h"
#include "hk/sail/detail.h"
#include "hk/svc/api.h"
#include "hk/util/Algorithm.h"
#include "hk/util/hash.h"

namespace hk::sail {

    namespace {

        template <typename Func>
        static void iterateDynSyms(Func callback) {
            const ptr aslrBase = init::getModuleStart();
            const Elf_Dyn* dynamic = init::_DYNAMIC;

            Elf_Addr rela = 0;
            Elf_Addr jmprel = 0;
            const char* dynstr = 0;
            const Elf_Sym* dynsym = 0;

            Elf_Xword rela_entry_size = sizeof(Elf_Rela);

            Elf_Xword rela_entry_count = 0;

            Elf_Xword rela_size = 0;

            for (; dynamic->d_tag != DT_NULL; dynamic++) {
                switch (dynamic->d_tag) {
                case DT_RELA:
                    rela = ((Elf_Addr)aslrBase + dynamic->d_un.d_ptr);
                    continue;

                case DT_JMPREL:
                    jmprel = ((Elf_Addr)aslrBase + dynamic->d_un.d_ptr);
                    continue;

                case DT_RELAENT:
                    rela_entry_size = dynamic->d_un.d_val;
                    continue;

                case DT_RELASZ:
                    rela_size = dynamic->d_un.d_val;
                    continue;

                case DT_SYMTAB:
                    dynsym = (Elf_Sym*)(aslrBase + dynamic->d_un.d_val);
                    continue;

                case DT_STRTAB:
                    dynstr = (const char*)(aslrBase + dynamic->d_un.d_val);
                    continue;

                case DT_RELACOUNT:
                case DT_RELCOUNT:
                case DT_REL:
                case DT_RELENT:
                case DT_RELSZ:
                case DT_NEEDED:
                case DT_PLTRELSZ:
                case DT_PLTGOT:
                case DT_HASH:
                case DT_STRSZ:
                case DT_SYMENT:
                case DT_INIT:
                case DT_FINI:
                case DT_SONAME:
                case DT_RPATH:
                case DT_SYMBOLIC:
                default:
                    continue;
                }
            }

            if (rela_entry_count == 0)
                rela_entry_count = rela_size / rela_entry_size;

            if (rela_entry_count) {
                Elf_Xword i = 0;

                while (i < rela_entry_count) {
                    Elf_Rela* entry = (Elf_Rela*)(rela + (i * rela_entry_size));

                    switch (ELF_R_TYPE(entry->r_info)) {
                    case ARCH_JUMP_SLOT:
                    case ARCH_RELATIVE:
                    case ARCH_GLOB_DAT: {
                        Elf_Addr* ptr = (Elf_Addr*)(aslrBase + entry->r_offset);
                        Elf_Xword symIndex = ELF_R_SYM(entry->r_info);
                        const Elf_Sym& sym = dynsym[symIndex];
                        if (sym.st_name)
                            callback(ptr, dynstr + sym.st_name);
                        break;
                    }
                    }
                    i++;
                }
            }

            if (jmprel) {
                Elf_Xword i = 0;
                while (true) {
                    Elf_Rela* entry = (Elf_Rela*)(jmprel + (i * rela_entry_size));
                    if (entry->r_offset == 0)
                        break;

                    switch (ELF_R_TYPE(entry->r_info)) {
                    case ARCH_JUMP_SLOT:
                    case ARCH_RELATIVE:
                    case ARCH_GLOB_DAT: {
                        Elf_Addr* ptr = (Elf_Addr*)(aslrBase + entry->r_offset);
                        Elf_Xword symIndex = ELF_R_SYM(entry->r_info);
                        const Elf_Sym& sym = dynsym[symIndex];
                        if (sym.st_name)
                            callback(ptr, dynstr + sym.st_name);
                        break;
                    }
                    }
                    i++;
                }
            }
        }

    } // namespace

    static volatile u64 sTimeElapsedLoadSymbols;

    void loadSymbols() {
        detail::loadVersions();

        sTimeElapsedLoadSymbols = svc::getSystemTick();

        iterateDynSyms([](Elf_Addr* ptr, const char* symbol) -> void {
            if constexpr (sail::sUsePrecalcHashes)
                *ptr = lookupSymbolFromDb<true>(cast<const u32*>(symbol));
            else
                *ptr = lookupSymbolFromDb<false>(symbol);
        });

        sTimeElapsedLoadSymbols = svc::getSystemTick() - sTimeElapsedLoadSymbols;

        /* float ms = sTimeElapsedLoadSymbols / 192000000.f;
        u64 µs = sTimeElapsedLoadSymbols / 192.f;
        HK_ABORT("TimeElapsedLoadSymbols: %zutix / %.2fms / %zuµs", sTimeElapsedLoadSymbols, ms, µs); */
    }

} // namespace hk::sail
