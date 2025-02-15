#include "rtld/RoModule.h"
#include "hk/types.h"
#include "hk/util/hash.h"
#include <cstring>

namespace nn::ro::detail {

    Elf_Sym* RoModule::GetSymbolByName(const char* name) const {
        u64 name_hash = hk::util::rtldElfHash(name);

        for (u32 i = this->m_pBuckets[name_hash % this->m_HashSize];
             i; i = this->m_pChains[i]) {
            bool is_common = this->m_pDynSym[i].st_shndx
                ? this->m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!is_common && strcmp(name, this->m_pStrTab + this->m_pDynSym[i].st_name) == 0) {
                return &this->m_pDynSym[i];
            }
        }

        return nullptr;
    }

    Elf_Sym* RoModule::GetSymbolByHashes(uint64_t bucket_hash, uint32_t murmur_hash) const {
        for (u32 i = this->m_pBuckets[bucket_hash % this->m_HashSize];
             i; i = this->m_pChains[i]) {
            bool is_common = this->m_pDynSym[i].st_shndx
                ? this->m_pDynSym[i].st_shndx == SHN_COMMON
                : true;
            if (!is_common && hk::util::hashMurmur(this->m_pStrTab + this->m_pDynSym[i].st_name) == murmur_hash) {
                return &this->m_pDynSym[i];
            }
        }

        return nullptr;
    }

} // namespace nn::ro::detail
