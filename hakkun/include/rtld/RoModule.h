#pragma once

#include <cstdint>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rtld/types.h"

namespace nn::ro::detail {

    class RoModule {

    private:
        // ResolveSymbols internals
        inline void ResolveSymbolRelAbsolute(Elf_Rel* entry);
        inline void ResolveSymbolRelaAbsolute(Elf_Rela* entry);
        inline void ResolveSymbolRelJumpSlot(Elf_Rel* entry, bool do_lazy_got_init);
        inline void ResolveSymbolRelaJumpSlot(Elf_Rela* entry, bool do_lazy_got_init);

    public:
        // nn::util::IntrusiveListBaseNode<nn::ro::detail::RoModule>
        RoModule* next;
        RoModule* prev;

        union {
            Elf_Rel* rel;
            Elf_Rela* rela;
        } m_pPlt;
        union {
            Elf_Rel* rel;
            Elf_Rela* rela;
        } m_pDyn;
#ifdef __RTLD_PAST_19XX__
        bool m_IsPltRela;
        uintptr_t m_Base;
        Elf_Dyn* m_pDynamicSection;
#else
        uintptr_t m_Base;
        Elf_Dyn* m_pDynamicSection;
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
        char* m_pStrTab;
        Elf_Sym* m_pDynSym;
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
            bool _C3_1 : 1;
            bool _C3_2 : 1;
            bool _C3_3 : 1;
            bool m_IsGnuHash : 1;
        };

    private:
        char m_Padding[0x40]; // Not sure what they added, but this is in older versions of RTLD too.

        Elf_Sym* GetSymbolByNameElf(const char* name) const;
        Elf_Sym* GetSymbolByHashesElf(uint64_t bucketHash, uint32_t murmurHash) const;
        Elf_Sym* GetSymbolByNameGnu(const char* name) const;
        Elf_Sym* GetSymbolByHashesGnu(uint32_t djb2Hash, uint32_t murmurHash) const;

    public:
        void Initialize(char* aslr_base, Elf_Dyn* dynamic);
        void Relocate();
        Elf_Sym* GetSymbolByName(const char* name) const;
        Elf_Sym* GetSymbolByHashes(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash) const;
        void ResolveSymbols(bool do_lazy_got_init);
        bool ResolveSym(Elf_Addr* target_symbol_address, Elf_Sym* symbol) const;
    };

} // namespace nn::ro::detail
