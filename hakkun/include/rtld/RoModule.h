#pragma once

#include <cstdint>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <type_traits>

#include "hk/ValueOrResult.h"
#include "hk/types.h"
#include "rtld/types.h"

namespace nn::ro::detail {

    class RoModule {
        const Elf_Sym* GetSymbolByNameElf(const char* name) const;
        const Elf_Sym* GetSymbolByHashesElf(uint64_t bucketHash, uint32_t murmurHash) const;
        const Elf_Sym* GetSymbolByNameGnu(const char* name) const;
        const Elf_Sym* GetSymbolByHashesGnu(uint32_t djb2Hash, uint32_t murmurHash) const;

        static ptr LookupGlobalAuto(const char* name);

        hk::ValueOrResult<ptr> ResolveSymbol(const Elf_Sym* symbol) const;

        template <typename Entry>
            requires std::is_same_v<Entry, Elf_Rel> or std::is_same_v<Entry, Elf_Rela>
        void ResolveSymbol(const Entry* entry);

        template <typename Entry, typename Func>
            requires std::is_same_v<Entry, Elf_Rel> or std::is_same_v<Entry, Elf_Rela>
        void ForEachRelocation(const Entry* entries, size numEntries, const Func& func);

    public:
        // nn::util::IntrusiveListBaseNode<nn::ro::detail::RoModule>
        RoModule* next;
        RoModule* prev;

        union {
            const Elf_Rel* rel = nullptr;
            const Elf_Rela* rela;
            ptr raw;
        } m_pPlt;
        union {
            const Elf_Rel* rel = nullptr;
            const Elf_Rela* rela;
        } m_pDyn;
#ifdef __RTLD_PAST_19XX__
        bool m_IsPltRela = false;
        uintptr_t m_Base = 0;
        Elf_Dyn* m_pDynamicSection = nullptr;
#else
        uintptr_t m_Base = 0;
        const Elf_Dyn* m_pDynamicSection = nullptr;
        bool m_IsPltRela = false;
#endif
        size_t m_PltRelSz = 0;
        void (*m_pInit)() = nullptr;
        void (*m_pFini)() = nullptr;
        union {
            struct {
                uint32_t* m_pBuckets = nullptr;
                uint32_t* m_pChains;
            };
            struct {
                uint64_t* m_pGnuBloomFilter;
                uint32_t m_GnuHashMaskwords;
            };
        };
        const char* m_pStrTab = nullptr;
        const Elf_Sym* m_pDynSym = nullptr;
        size_t m_StrSz = 0;
        uintptr_t* m_pGot = nullptr;
        size_t m_DynRelaSz = 0;
        size_t m_DynRelSz = 0;
        size_t m_RelCount = 0;
        size_t m_RelaCount = 0;
        union {
            struct {
                size_t m_Symbols = 0;
                size_t m_HashSize;
            };
            struct {
                uint64_t m_GnuHashShift2;
                uint32_t* m_pGnuHash;
            };
        };
        void* got_stub_ptr;
        uint64_t _B8;
        uint8_t _C0[3];
        struct {
            bool _C3_0 : 1;
            bool _m_IsPltRela : 1;
            bool m_BindNow : 1;
            bool _C3_3 : 1;
            bool m_IsGnuHash : 1 = 0;
        };

    private:
        char m_Padding[0x40]; // Not sure what they added, but this is in older versions of RTLD too.

    public:
        RoModule() = default;

        void Initialize(ptr moduleBase, const Elf_Dyn* dynamic);
        void Relocate();
        void ResolveSymbols();

        const Elf_Sym* GetSymbolByName(const char* name) const;
        const Elf_Sym* GetSymbolByHashes(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash) const;

        using LookupGlobalManualFunctionPointer = Elf_Addr (*)(const char*);
        static LookupGlobalManualFunctionPointer g_LookupGlobalManualFunctionPointer;
    };

} // namespace nn::ro::detail
