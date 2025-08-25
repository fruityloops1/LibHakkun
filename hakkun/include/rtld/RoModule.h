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

    /**
     * @brief RTLD-managed class representing a NSO/NRO module loaded in memory
     *
     */
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
            const Elf_Rel* rel;
            const Elf_Rela* rela;
            ptr raw;
        } m_pPlt;
        union {
            const Elf_Rel* rel;
            const Elf_Rela* rela;
        } m_pDyn;
#ifdef __RTLD_PAST_19XX__
        bool m_IsPltRela;
        uintptr_t m_Base;
        Elf_Dyn* m_pDynamicSection;
#else
        uintptr_t m_Base;
        const Elf_Dyn* m_pDynamicSection;
        bool m_IsPltRela;
#endif
        size_t m_PltRelSz;
        void (*m_pInit)();
        void (*m_pFini)();
        union {
            struct {
                uint32_t* m_pBuckets;
                uint32_t* m_pChains;
            };
            struct {
                uint64_t* m_pGnuBloomFilter;
                uint32_t m_GnuHashMaskwords;
            };
        };
        const char* m_pStrTab;
        const Elf_Sym* m_pDynSym;
        size_t m_StrSz;
        uintptr_t* m_pGot;
        size_t m_DynRelaSz;
        size_t m_DynRelSz;
        size_t m_RelCount;
        size_t m_RelaCount;
        union {
            struct {
                size_t m_Symbols;
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
            bool m_IsGnuHash : 1;
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
