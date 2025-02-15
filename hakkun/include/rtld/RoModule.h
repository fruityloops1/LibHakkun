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
        uint32_t* m_pBuckets;
        uint32_t* m_pChains;
        char* m_pStrTab;
        Elf_Sym* m_pDynSym;
        size_t m_StrSz;
        uintptr_t* m_pGot;
        size_t m_DynRelaSz;
        size_t m_DynRelSz;
        size_t m_RelCount;
        size_t m_RelaCount;
        size_t m_Symbols;
        size_t m_HashSize;
        void* got_stub_ptr;
#ifdef __RTLD_6XX__
        Elf_Xword soname_idx;
        size_t nro_size;
        bool cannot_revert_symbols;
#endif

        void Initialize(char* aslr_base, Elf_Dyn* dynamic);
        void Relocate();
        Elf_Sym* GetSymbolByName(const char* name) const;
        Elf_Sym* GetSymbolByHashes(uint64_t bucket_hash, uint32_t murmur_hash) const;
        void ResolveSymbols(bool do_lazy_got_init);
        bool ResolveSym(Elf_Addr* target_symbol_address, Elf_Sym* symbol) const;
    };

} // namespace nn::ro::detail
