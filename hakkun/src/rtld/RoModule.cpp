#include "rtld/RoModule.h"
#include "hk/types.h"
#include "hk/util/hash.h"
#include <cstring>

namespace nn::ro::detail {

    Elf_Sym* RoModule::GetSymbolByNameElf(const char* name) const {
        u64 nameHash = hk::util::rtldElfHash(name);

        for (u32 i = this->m_pBuckets[nameHash % this->m_HashSize];
            i; i = this->m_pChains[i]) {
            bool isCommon = this->m_pDynSym[i].st_shndx
                ? this->m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!isCommon && strcmp(name, this->m_pStrTab + this->m_pDynSym[i].st_name) == 0)
                return &this->m_pDynSym[i];
        }

        return nullptr;
    }

    Elf_Sym* RoModule::GetSymbolByHashesElf(uint64_t bucketHash, uint32_t murmurHash) const {
        for (u32 i = this->m_pBuckets[bucketHash % this->m_HashSize];
            i; i = this->m_pChains[i]) {
            bool isCommon = this->m_pDynSym[i].st_shndx
                ? this->m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!isCommon && hk::util::hashMurmur(this->m_pStrTab + this->m_pDynSym[i].st_name) == murmurHash)
                return &this->m_pDynSym[i];
        }

        return nullptr;
    }

    Elf_Sym* RoModule::GetSymbolByNameGnu(const char* name) const {
        uint32_t maskwords = m_GnuHashMaskwords;

        uint32_t nameHash = hk::util::djb2Hash(name);
        uint32_t wordIdx = (maskwords - 1) & (nameHash >> 6);
        uint64_t bloomWord = m_pGnuBloomFilter[wordIdx];

        uint32_t bit1 = nameHash & 0b111111;
        uint32_t bit2 = (nameHash >> (this->m_GnuHashShift2 & 0b11111)) & 0b111111;
        uint64_t bloomMask = hk::bit(bit1) | hk::bit(bit2);

        if ((bloomWord & bloomMask) != bloomMask)
            return nullptr;

        uint32_t nbuckets = this->m_pGnuHash[0];
        uint32_t symoffset = this->m_pGnuHash[1];

        uint32_t bucketIdx = nbuckets != 0 ? nameHash / nbuckets : 0;

        uint32_t* bucketsStart = (uint32_t*)m_pGnuBloomFilter + maskwords * 2;
        uint32_t symIdx = bucketsStart[nameHash - bucketIdx * nbuckets];

        if (symIdx < symoffset)
            return nullptr;

        uint32_t* chainPtr = (uint32_t*)m_pGnuBloomFilter + ((symIdx + nbuckets + maskwords * 2) - symoffset);

        for (;; symIdx++, chainPtr++) {
            uint32_t chainhash = *chainPtr;
            if ((chainhash ^ nameHash) < 2) {
                bool isCommon = this->m_pDynSym[symIdx].st_shndx
                    ? this->m_pDynSym[symIdx].st_shndx == SHN_COMMON
                    : true;
                const char* sym_name = this->m_pStrTab + this->m_pDynSym[symIdx].st_name;
                if (!isCommon && strcmp(name, sym_name) == 0)
                    return &this->m_pDynSym[symIdx];
            }
            if (chainhash & 1)
                break;
        }

        return nullptr;
    }

    Elf_Sym* RoModule::GetSymbolByHashesGnu(uint32_t djb2Hash, uint32_t murmurHash) const {
        uint32_t maskwords = m_GnuHashMaskwords;

        uint32_t wordIdx = (maskwords - 1) & (djb2Hash >> 6);
        uint64_t bloomWord = m_pGnuBloomFilter[wordIdx];

        uint32_t bit1 = djb2Hash & 0b111111;
        uint32_t bit2 = (djb2Hash >> (this->m_GnuHashShift2 & 0b11111)) & 0b111111;
        uint64_t bloomMask = hk::bit(bit1) | hk::bit(bit2);

        if ((bloomWord & bloomMask) != bloomMask)
            return nullptr;

        uint32_t nbuckets = this->m_pGnuHash[0];
        uint32_t symoffset = this->m_pGnuHash[1];

        uint32_t bucketIdx = nbuckets != 0 ? djb2Hash / nbuckets : 0;

        uint32_t* bucketsStart = (uint32_t*)m_pGnuBloomFilter + maskwords * 2;
        uint32_t symIdx = bucketsStart[djb2Hash - bucketIdx * nbuckets];

        if (symIdx < symoffset)
            return nullptr;

        uint32_t* chainPtr = (uint32_t*)m_pGnuBloomFilter + ((symIdx + nbuckets + maskwords * 2) - symoffset);

        for (;; symIdx++, chainPtr++) {
            uint32_t chainhash = *chainPtr;
            if ((chainhash ^ djb2Hash) < 2) {
                bool isCommon = this->m_pDynSym[symIdx].st_shndx
                    ? this->m_pDynSym[symIdx].st_shndx == SHN_COMMON
                    : true;
                if (!isCommon && hk::util::hashMurmur(this->m_pStrTab + this->m_pDynSym[symIdx].st_name) == murmurHash)
                    return &this->m_pDynSym[symIdx];
            }
            if (chainhash & 1)
                break;
        }

        return nullptr;
    }

    Elf_Sym* RoModule::GetSymbolByName(const char* name) const {
        return m_IsGnuHash ? GetSymbolByNameGnu(name) : GetSymbolByNameElf(name);
    }

    Elf_Sym* RoModule::GetSymbolByHashes(uint64_t bucketHash, uint32_t djb2Hash, uint32_t murmurHash) const {
        return m_IsGnuHash ? GetSymbolByHashesGnu(djb2Hash, murmurHash) : GetSymbolByHashesElf(bucketHash, murmurHash);
    }

} // namespace nn::ro::detail
