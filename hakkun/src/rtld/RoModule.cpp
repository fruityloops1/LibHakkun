#include "rtld/RoModule.h"
#include "hk/ro/ElfUtil.h"
#include "hk/ro/results.h"
#include "hk/types.h"
#include "hk/util/hash.h"
#include "rtld/types.h"
#include <cstring>

namespace nn::ro::detail {

    void RoModule::Initialize(ptr moduleBase, const Elf_Dyn* dynamic) {
        this->next = this;
        this->prev = this;

        m_Base = moduleBase;
        m_pDynamicSection = dynamic;

        const auto data = hk::ro::parseDynamic(m_Base, m_pDynamicSection);

        m_PltRelSz = data.pltRelSize;
        m_pGot = data.pltGot;

        m_IsGnuHash = data.gnuHashTable != nullptr;

        if (m_IsGnuHash) {
            m_pGnuBloomFilter = cast<uint64_t*>(&data.gnuHashTable[4]);
            m_GnuHashMaskwords = data.gnuHashTable[2];
            m_GnuHashShift2 = data.gnuHashTable[3];
            m_pGnuHash = data.gnuHashTable;
        } else {
            m_HashSize = data.hashTable[0];
            m_Symbols = data.hashTable[1];
            m_pBuckets = &data.hashTable[2];
            m_pChains = &data.hashTable[2 + m_HashSize];
        }

        m_pStrTab = data.dynstr;
        m_pDynSym = data.dynsym;
        m_StrSz = data.dynstrSize;

        if (data.rel != nullptr)
            m_pDyn.rel = data.rel;
        if (data.rela != nullptr)
            m_pDyn.rela = data.rela;

        m_DynRelSz = data.relSize;
        m_DynRelaSz = data.relaSize;

        m_pInit = data.init;
        m_pFini = data.fini;

        m_IsPltRela = data.isRela;
        m_pPlt.raw = data.plt;

        m_RelCount = data.relCount;
        m_RelaCount = data.relaCount;

        // m_BindNow = data.bindNow;
        m_BindNow = true;
    }

    void RoModule::Relocate() {
        ForEachRelocation(m_pDyn.rel, m_RelCount, [&](const Elf_Rel* entry) {
            if (ELF_R_TYPE(entry->r_info) == ARCH_RELATIVE) {
                Elf_Addr* ptr = cast<Elf_Addr*>(m_Base + entry->r_offset);

                *ptr += m_Base;
            }
        });

        ForEachRelocation(m_pDyn.rela, m_RelaCount, [&](const Elf_Rela* entry) {
            if (ELF_R_TYPE(entry->r_info) == ARCH_RELATIVE) {
                Elf_Addr* ptr = cast<Elf_Addr*>(m_Base + entry->r_offset);

                *ptr = m_Base + entry->r_addend;
            }
        });
    }

    hk::ValueOrResult<ptr> RoModule::ResolveSymbol(const Elf_Sym* symbol) const {
        auto visiblity = ELF_ST_VISIBILITY(symbol->st_other);
        bool weak = ELF_ST_BIND(symbol->st_info) == STB_WEAK;
        const char* name = m_pStrTab + symbol->st_name;

        if (visiblity != STV_DEFAULT) {
            symbol = GetSymbolByName(name);

            if (symbol == nullptr) {
                if (weak)
                    return ptr(0);
                else
                    return hk::ro::ResultSymbolUnresolved();
            }

            return ptr(m_Base + symbol->st_value);
        } else {
            ptr addr = LookupGlobalAuto(name);

            if (addr == 0 && g_LookupGlobalManualFunctionPointer)
                addr = g_LookupGlobalManualFunctionPointer(name);

            if (addr != 0)
                return addr;
        }

        return hk::ro::ResultSymbolUnresolved();
    }

    template <typename Entry>
        requires std::is_same_v<Entry, Elf_Rel> or std::is_same_v<Entry, Elf_Rela>
    void RoModule::ResolveSymbol(const Entry* rel) {
        auto type = ELF_R_TYPE(rel->r_info);
        auto symIndex = ELF_R_SYM(rel->r_info);

        if (ARCH_IS_REL_ABSOLUTE(type) or type == ARCH_GLOB_DAT or type == ARCH_RELATIVE or type == ARCH_JUMP_SLOT) {
            const Elf_Sym* symbol = &m_pDynSym[symIndex];
            Elf_Addr* dest = cast<Elf_Addr*>(m_Base + rel->r_offset);

            auto res = ResolveSymbol(symbol);

            res.map([&](ptr addr) {
                if constexpr (std::is_same_v<Entry, Elf_Rel>)
                    *dest += addr;
                else
                    *dest = addr + rel->r_addend;
            });
        }
    }

    template <typename Entry, typename Func>
        requires std::is_same_v<Entry, Elf_Rel> or std::is_same_v<Entry, Elf_Rela>
    void RoModule::ForEachRelocation(const Entry* entries, size numEntries, const Func& func) {
        for (size i = 0; i < numEntries; i++) {
            const Entry* entry = &entries[i];

            func(entry);
        }
    }

    void RoModule::ResolveSymbols() {
        ForEachRelocation(m_pDyn.rel, m_RelCount, [&](const Elf_Rel* entry) {
            ResolveSymbol(entry);
        });

        ForEachRelocation(m_pDyn.rela, m_RelaCount, [&](const Elf_Rela* entry) {
            ResolveSymbol(entry);
        });

        if (m_IsPltRela)
            ForEachRelocation(m_pPlt.rela, m_PltRelSz / sizeof(Elf_Rela), [&](const Elf_Rela* entry) {
                ResolveSymbol(entry);
            });
        else
            ForEachRelocation(m_pPlt.rel, m_PltRelSz / sizeof(Elf_Rel), [&](const Elf_Rel* entry) {
                ResolveSymbol(entry);
            });

        if (m_pGot)
            m_pGot[2] = 0;
    }

    const Elf_Sym* RoModule::GetSymbolByNameElf(const char* name) const {
        u64 nameHash = hk::util::hashElfBucket(name);

        for (u32 i = m_pBuckets[nameHash % m_HashSize];
            i; i = m_pChains[i]) {
            bool isCommon = m_pDynSym[i].st_shndx
                ? m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!isCommon && strcmp(name, m_pStrTab + m_pDynSym[i].st_name) == 0)
                return &m_pDynSym[i];
        }

        return nullptr;
    }

    const Elf_Sym* RoModule::GetSymbolByHashesElf(uint64_t bucketHash, uint32_t murmurHash) const {
        for (u32 i = m_pBuckets[bucketHash % m_HashSize];
            i; i = m_pChains[i]) {
            bool isCommon = m_pDynSym[i].st_shndx
                ? m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!isCommon && hk::util::hashMurmur(m_pStrTab + m_pDynSym[i].st_name) == murmurHash)
                return &m_pDynSym[i];
        }

        return nullptr;
    }

    const Elf_Sym* RoModule::GetSymbolByNameGnu(const char* name) const {
        uint32_t maskwords = m_GnuHashMaskwords;

        uint32_t nameHash = hk::util::hashDjb2(name);
        uint32_t wordIdx = (maskwords - 1) & (nameHash >> 6);
        uint64_t bloomWord = m_pGnuBloomFilter[wordIdx];

        uint32_t bit1 = nameHash & hk::bits(6);
        uint32_t bit2 = (nameHash >> (m_GnuHashShift2 & hk::bits(5))) & hk::bits(6);
        uint64_t bloomMask = hk::bit(bit1) | hk::bit(bit2);

        if ((bloomWord & bloomMask) != bloomMask)
            return nullptr;

        uint32_t nbuckets = m_pGnuHash[0];
        uint32_t symoffset = m_pGnuHash[1];

        uint32_t bucketIdx = nbuckets != 0 ? nameHash / nbuckets : 0;

        uint32_t* bucketsStart = (uint32_t*)m_pGnuBloomFilter + maskwords * 2;
        uint32_t symIdx = bucketsStart[nameHash - bucketIdx * nbuckets];

        if (symIdx < symoffset)
            return nullptr;

        uint32_t* chainPtr = (uint32_t*)m_pGnuBloomFilter + ((symIdx + nbuckets + maskwords * 2) - symoffset);

        for (;; symIdx++, chainPtr++) {
            uint32_t chainhash = *chainPtr;
            if ((chainhash ^ nameHash) < 2) {
                bool isCommon = m_pDynSym[symIdx].st_shndx
                    ? m_pDynSym[symIdx].st_shndx == SHN_COMMON
                    : true;
                const char* sym_name = m_pStrTab + m_pDynSym[symIdx].st_name;
                if (!isCommon && strcmp(name, sym_name) == 0)
                    return &m_pDynSym[symIdx];
            }
            if (chainhash & 1)
                break;
        }

        return nullptr;
    }

    const Elf_Sym* RoModule::GetSymbolByHashesGnu(uint32_t djb2Hash, uint32_t murmurHash) const {
        uint32_t maskwords = m_GnuHashMaskwords;

        uint32_t wordIdx = (maskwords - 1) & (djb2Hash >> 6);
        uint64_t bloomWord = m_pGnuBloomFilter[wordIdx];

        uint32_t bit1 = djb2Hash & hk::bits(6);
        uint32_t bit2 = (djb2Hash >> (m_GnuHashShift2 & hk::bits(5))) & hk::bits(6);
        uint64_t bloomMask = hk::bit(bit1) | hk::bit(bit2);

        if ((bloomWord & bloomMask) != bloomMask)
            return nullptr;

        uint32_t nbuckets = m_pGnuHash[0];
        uint32_t symoffset = m_pGnuHash[1];

        uint32_t bucketIdx = nbuckets != 0 ? djb2Hash / nbuckets : 0;

        uint32_t* bucketsStart = (uint32_t*)m_pGnuBloomFilter + maskwords * 2;
        uint32_t symIdx = bucketsStart[djb2Hash - bucketIdx * nbuckets];

        if (symIdx < symoffset)
            return nullptr;

        uint32_t* chainPtr = (uint32_t*)m_pGnuBloomFilter + ((symIdx + nbuckets + maskwords * 2) - symoffset);

        for (;; symIdx++, chainPtr++) {
            uint32_t chainhash = *chainPtr;
            if ((chainhash ^ djb2Hash) < 2) {
                bool isCommon = m_pDynSym[symIdx].st_shndx
                    ? m_pDynSym[symIdx].st_shndx == SHN_COMMON
                    : true;
                if (!isCommon && hk::util::hashMurmur(m_pStrTab + m_pDynSym[symIdx].st_name) == murmurHash)
                    return &m_pDynSym[symIdx];
            }
            if (chainhash & 1)
                break;
        }

        return nullptr;
    }

    const Elf_Sym* RoModule::GetSymbolByName(const char* name) const {
        return m_IsGnuHash ? GetSymbolByNameGnu(name) : GetSymbolByNameElf(name);
    }

    const Elf_Sym* RoModule::GetSymbolByHashes(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash) const {
        return m_IsGnuHash ? GetSymbolByHashesGnu(djb2Hash, murmurHash) : GetSymbolByHashesElf(bucketHash, murmurHash);
    }

} // namespace nn::ro::detail
