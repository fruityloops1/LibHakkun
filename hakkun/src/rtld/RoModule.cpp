#include "rtld/RoModule.h"
#include "hk/types.h"
#include "hk/util/hash.h"
#include <cstring>

namespace nn::ro::detail {

    void RoModule::Initialize(ptr moduleBase, const Elf_Dyn* dynamic) {
    }

    void RoModule::Relocate() {
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
