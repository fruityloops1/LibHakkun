#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "rtld/elf.h"
#include "rtld/types.h"

namespace hk::ro {

    namespace detail {

        struct DynamicData {
            using InitFiniFunc = void (*)();

            ptr* pltGot = 0;
            u32* hashTable = nullptr;
            u32* gnuHashTable = nullptr;
            const Elf_Rel* rel = 0;
            const Elf_Rela* rela = 0;
            Elf_Addr plt = 0;
            const Elf_Sym* dynsym = nullptr;
            const char* dynstr = nullptr;
            InitFiniFunc init = nullptr;
            InitFiniFunc fini = nullptr;
            Elf_Xword relSize = 0;
            Elf_Xword relaSize = 0;
            Elf_Xword relCount = 0;
            Elf_Xword relaCount = 0;
            Elf_Xword pltRelSize = 0;
            Elf_Xword dynstrSize = 0;
            Elf_Xword sonameIdx = 0;
            bool isRela = true;
            bool bindNow = false;

#ifdef __aarch64__
#define _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE \
    if constexpr (FilterAbsolute) {              \
        bool skip = true;                        \
        switch (ELF_R_TYPE(entry->r_info)) {     \
        case ARCH_ABS32:                         \
        case ARCH_ABS64:                         \
        case ARCH_JUMP_SLOT:                     \
        case ARCH_RELATIVE:                      \
        case ARCH_GLOB_DAT:                      \
            skip = false;                        \
        }                                        \
                                                 \
        if (skip)                                \
            continue;                            \
    }
#else
#define _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE \
    if constexpr (FilterAbsolute) {              \
        bool skip = true;                        \
        switch (ELF_R_TYPE(entry->r_info)) {     \
        case ARCH_ABS32:                         \
        case ARCH_JUMP_SLOT:                     \
        case ARCH_RELATIVE:                      \
        case ARCH_GLOB_DAT:                      \
            skip = false;                        \
        }                                        \
                                                 \
        if (skip)                                \
            continue;                            \
    }
#endif

            template <bool FilterAbsolute = true, typename Func>
            void forEachRel(const Func& callback) const {
                if (rel == 0 || relSize == 0 || relCount == 0)
                    return;

                for (size i = 0; i < relCount; i++) {
                    const Elf_Rel* entry = &rel[i];

                    _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE
                    callback(*entry);
                }
            }

            template <bool FilterAbsolute = true, typename Func>
            void forEachRela(const Func& callback) const {
                if (rela == 0 || relaSize == 0 || relaCount == 0)
                    return;

                for (size i = 0; i < relaCount; i++) {
                    const Elf_Rela* entry = &rela[i];

                    _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE
                    callback(*entry);
                }
            }

            template <bool FilterAbsolute = true, typename Func>
            void forEachPlt(const Func& callback) const {
                if (plt == 0)
                    return;

                if (isRela) {
                    const Elf_Rela* pltEntries = cast<const Elf_Rela*>(plt);
                    for (size i = 0; true; i++) {
                        const Elf_Rela* entry = &pltEntries[i];
                        if (entry->r_offset == 0)
                            break;

                        _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE
                        callback(nullptr, entry);
                    }
                } else {
                    const Elf_Rel* pltEntries = cast<const Elf_Rel*>(plt);
                    for (size i = 0; true; i++) {
                        const Elf_Rel* entry = &pltEntries[i];
                        if (entry->r_offset == 0)
                            break;

                        _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE
                        callback(entry, nullptr);
                    }
                }
            }

            template <bool FilterAbsolute = true, typename Func>
            void forEachPltRelRela(const Func& callback) const {
                forEachPlt<FilterAbsolute>(callback);
                forEachRel<FilterAbsolute>([&](const Elf_Rel& entry) {
                    callback(&entry, nullptr);
                });
                forEachRela<FilterAbsolute>([&](const Elf_Rela& entry) {
                    callback(nullptr, &entry);
                });
            }

#undef _HK_RO_DETAIL_DYNAMICDATA_FILTERABSOLUTE
        };

    } // namespace detail

    /**
     * @brief Parses ELF DYNAMIC section
     *
     * @param moduleBase
     * @param dynamic
     * @return Object with parsed data and util functions
     */
    hk_alwaysinline inline detail::DynamicData parseDynamic(ptr moduleBase, const Elf_Dyn* dynamic) {
        detail::DynamicData data;

        for (; dynamic->d_tag != DT_NULL; dynamic++) {
            switch (dynamic->d_tag) {
            case DT_PLTRELSZ:
                data.pltRelSize = dynamic->d_un.d_val;
                continue;
            case DT_PLTGOT:
                data.pltGot = cast<ptr*>(moduleBase + dynamic->d_un.d_ptr);
                continue;
            case DT_HASH:
                data.hashTable = cast<u32*>(moduleBase + dynamic->d_un.d_ptr);
                continue;
            case DT_GNU_HASH:
                data.gnuHashTable = cast<u32*>(moduleBase + dynamic->d_un.d_ptr);
                continue;
            case DT_STRTAB:
                data.dynstr = cast<const char*>(moduleBase + dynamic->d_un.d_val);
                continue;
            case DT_STRSZ:
                data.dynstrSize = dynamic->d_un.d_val;
                continue;
            case DT_SYMTAB:
                data.dynsym = cast<const Elf_Sym*>(moduleBase + dynamic->d_un.d_val);
                continue;
            case DT_REL:
                data.rel = cast<const Elf_Rel*>(moduleBase + dynamic->d_un.d_ptr);
                continue;
            case DT_RELA:
                data.rela = cast<const Elf_Rela*>(moduleBase + dynamic->d_un.d_ptr);
                continue;
            case DT_JMPREL:
                data.plt = Elf_Addr(moduleBase) + dynamic->d_un.d_ptr;
                continue;
            case DT_RELSZ:
                data.relSize = dynamic->d_un.d_val;
                continue;
            case DT_RELASZ:
                data.relaSize = dynamic->d_un.d_val;
                continue;
            case DT_RELENT:
                HK_ASSERT(dynamic->d_un.d_val == sizeof(Elf_Rel));
                continue;
            case DT_RELAENT:
                HK_ASSERT(dynamic->d_un.d_val == sizeof(Elf_Rela));
                continue;
            case DT_SYMENT:
                HK_ASSERT(dynamic->d_un.d_val == sizeof(Elf_Sym));
                continue;
            case DT_INIT:
                data.init = detail::DynamicData::InitFiniFunc(moduleBase + dynamic->d_un.d_val);
                continue;
            case DT_FINI:
                data.fini = detail::DynamicData::InitFiniFunc(moduleBase + dynamic->d_un.d_val);
                continue;
            case DT_PLTREL: {
                auto type = dynamic->d_un.d_val;
                data.isRela = type == DT_RELA;
                HK_ASSERT(type == DT_REL || type == DT_RELA);
                continue;
            }
            case DT_SONAME:
                data.sonameIdx = dynamic->d_un.d_val;
                continue;
            case DT_BIND_NOW:
                data.bindNow = true;
                continue;

            case DT_NEEDED:
            case DT_RPATH:
            case DT_SYMBOLIC:
            case DT_DEBUG:
            case DT_TEXTREL:
            default:
                continue;
            }
        }

        if (data.relCount == 0)
            data.relCount = data.relSize / sizeof(Elf_Rel);
        if (data.relaCount == 0)
            data.relaCount = data.relaSize / sizeof(Elf_Rela);

        return data;
    }

} // namespace hk::ro
