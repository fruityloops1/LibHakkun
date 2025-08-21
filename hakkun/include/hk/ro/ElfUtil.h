#pragma once

#include "hk/types.h"
#include "rtld/types.h"

namespace hk::ro {

    namespace detail {

        struct DymamicData {
            Elf_Addr rela = 0;
            Elf_Addr jmprel = 0;
            Elf_Xword relaEntrySize = sizeof(Elf_Rela);
            Elf_Xword relaSize = 0;
            Elf_Xword relaEntryCount = 0;
            const Elf_Sym* dynsym = nullptr;
            const char* dynstr = nullptr;

            template <bool PltFilter, typename Func>
            void forEachRela(const Func& callback) const {
                if (rela == 0 || relaSize == 0 || relaEntrySize == 0 || relaEntryCount == 0)
                    return;

                for (size i = 0; i < relaEntryCount; i++) {
                    const Elf_Rela* entry = cast<const Elf_Rela*>(rela + (i * relaEntrySize));

                    if constexpr (PltFilter) {
                        bool skip = true;
                        switch (ELF_R_TYPE(entry->r_info)) {
                        case ARCH_ABS32:
                        case ARCH_ABS64:
                        case ARCH_JUMP_SLOT:
                        case ARCH_RELATIVE:
                        case ARCH_GLOB_DAT:
                            skip = false;
                        }

                        if (skip)
                            continue;
                    }

                    callback(*entry);
                }
            }

            template <bool PltFilter, typename Func>
            void forEachJmpRel(const Func& callback) const {
                if (jmprel == 0 || relaSize == 0 || relaEntrySize == 0 || relaEntryCount == 0)
                    return;

                for (size i = 0; true; i++) {
                    const Elf_Rela* entry = cast<const Elf_Rela*>(jmprel + (i * relaEntrySize));
                    if (entry->r_offset == 0)
                        break;

                    if constexpr (PltFilter) {
                        bool skip = true;
                        switch (ELF_R_TYPE(entry->r_info)) {
                        case ARCH_ABS32:
                        case ARCH_ABS64:
                        case ARCH_JUMP_SLOT:
                        case ARCH_RELATIVE:
                        case ARCH_GLOB_DAT:
                            skip = false;
                        }

                        if (skip)
                            continue;
                    }
                    callback(*entry);
                }
            }

            template <typename Func>
            void forEachPlt(const Func& callback) const {
                forEachRela<true>(callback);
                forEachJmpRel<true>(callback);
            }
        };

    } // namespace detail

    hk_alwaysinline inline detail::DymamicData parseDynamic(ptr moduleBase, const Elf_Dyn* dynamic) {
        detail::DymamicData data;

        for (; dynamic->d_tag != DT_NULL; dynamic++) {
            switch (dynamic->d_tag) {
            case DT_RELA:
                data.rela = Elf_Addr(moduleBase) + dynamic->d_un.d_ptr;
                continue;

            case DT_JMPREL:
                data.jmprel = Elf_Addr(moduleBase) + dynamic->d_un.d_ptr;
                continue;

            case DT_RELAENT:
                data.relaEntrySize = dynamic->d_un.d_val;
                continue;

            case DT_RELASZ:
                data.relaSize = dynamic->d_un.d_val;
                continue;

            case DT_SYMTAB:
                data.dynsym = cast<const Elf_Sym*>(moduleBase + dynamic->d_un.d_val);
                continue;

            case DT_STRTAB:
                data.dynstr = cast<const char*>(moduleBase + dynamic->d_un.d_val);
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

        data.relaEntryCount = data.relaSize / data.relaEntrySize;

        return data;
    }

} // namespace hk::ro
