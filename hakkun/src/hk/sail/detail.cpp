#ifndef HK_DISABLE_SAIL

#include "hk/sail/detail.h"
#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/ro/RoModule.h"
#include "hk/ro/RoUtil.h"
#include "hk/types.h"

namespace hk::sail {

    namespace detail {

        static s32 sModuleVersions[ro::sMaxModuleNum] {};

        void loadVersions() {
            for (int i = 0; i < ro::getNumModules(); i++) {
                sModuleVersions[i] = -1;
                uintptr_t versionsStart = uintptr_t(gVersions);

                uintptr_t versionsOffset = gVersions[i];
                if (versionsOffset == 0)
                    continue;
                const u32* versions = cast<const u32*>(versionsStart + versionsOffset);

                u8 curBuildId[ro::sBuildIdSize];
                if (ro::getModuleBuildIdByIndex(i, curBuildId).failed())
                    continue;

                u32 numVersions = versions[0];
                versions++;

                struct {
                    u8 data[ro::sBuildIdSize];
                }* buildIds { decltype(buildIds)(versions) };

                for (int versionIndex = 0; versionIndex < numVersions; versionIndex++) {

                    if (__builtin_memcmp(buildIds[versionIndex].data, curBuildId, sizeof(curBuildId)) == 0) {
                        sModuleVersions[i] = versionIndex;
                        break;
                    }
                }
            }
        }

        // Versioned

        bool SymbolVersioned::isVersion(u32 moduleIdx) const {
            return (versionsBitset >> sModuleVersions[moduleIdx]) & 0b1;
        }

        // DataBlock

        template <size DataBlockSize>
        static hk_alwaysinline ptr findDataBlock(const ro::RoModule::Range& range, const u8* searchForData) {
            for (ptr search = range.start(); search < range.end(); search += 4) {
                if (__builtin_memcmp((u8*)search, searchForData, DataBlockSize) == 0)
                    return search;
            }

            return 0;
        }

        static hk_alwaysinline ptr findDataBlockVariableSize(const ro::RoModule::Range& range, const u8* searchForData, size searchForSize) {

            for (ptr search = range.start(); search < range.end(); search += 4) {
                if (__builtin_memcmp((u8*)search, searchForData, searchForSize) == 0)
                    return search;
            }

            return 0;
        }

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyDataBlockSymbol(bool abort, const SymbolDataBlock* sym, ptr* out, const T* destSymbol) {
            const auto* module = ro::getModuleByIndex(sym->moduleIdx);

            HK_ABORT_UNLESS(module != nullptr, "UnknownModule with idx %d", sym->moduleIdx);

            const SymbolDataBlock::DataBlock* block = cast<const SymbolDataBlock::DataBlock*>(uintptr_t(gSymbols) + sym->offsetToDataBlock);

            s32 loadedVer = sModuleVersions[sym->moduleIdx];

            switch (sym->versionBoundaryType) {
            case 1:
                if (loadedVer >= sym->versionBoundary)
                    return;
            case 2:
                if (loadedVer < sym->versionBoundary)
                    return;
            case 0:
            default:
                break;
            }

            ro::RoModule::Range range;

            switch (sym->sectionLimit) {
            case 1:
                range = module->text;
                break;
            case 2:
                range = module->rodata;
                break;
            case 3:
                range = module->data;
                break;
            case 0:
            default:
                range = module->range();
            }

            ptr address = 0;
#define DBSIZE(SIZE)                                       \
    case SIZE:                                             \
        address = findDataBlock<SIZE>(range, block->data); \
        break

            switch (block->size) {
                DBSIZE(4);
                DBSIZE(8);
                DBSIZE(12);
                DBSIZE(16);
                DBSIZE(20);
                DBSIZE(24);
                DBSIZE(28);
                DBSIZE(32);
                DBSIZE(36);
                DBSIZE(40);
                DBSIZE(44);
                DBSIZE(48);
                DBSIZE(52);
                DBSIZE(56);
                DBSIZE(60);
                DBSIZE(64); // starts using normal memcmp after this
            default:
                address = findDataBlockVariableSize(range, block->data, block->size);
            }
#undef DBSIZE

            if (abort) {
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %08x (DataBlock)", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %s (DataBlock)", destSymbol);
                }
            }

            address += sym->offsetToFoundBlock;
            *out = address;
        }

        void SymbolDataBlock::apply(bool abort, ptr* out, const char* destSymbol) {
            applyDataBlockSymbol<false>(abort, this, out, destSymbol);
        }

        void SymbolDataBlock::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyDataBlockSymbol<true>(abort, this, out, destSymbol);
        }

        // Dynamic

        _HK_SAIL_PRECALC_TEMPLATE hk_alwaysinline void applyDynamicSymbol(bool abort, const SymbolDynamic* sym, ptr* out, const T* destSymbol) {
            ptr address = ro::lookupSymbol(sym->lookupNameRtldHash, sym->lookupNameMurmur);

            if (abort) {
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %08x (Dynamic)", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %s (Dynamic)", destSymbol);
                }
            }

            *out = address;
        }

        void SymbolDynamic::apply(bool abort, ptr* out, const char* destSymbol) {
            applyDynamicSymbol<false>(abort, this, out, destSymbol);
        }

        void SymbolDynamic::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyDynamicSymbol<true>(abort, this, out, destSymbol);
        }

        // Immediate

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyImmediateSymbol(bool abort, const SymbolImmediate* sym, ptr* out, const T* destSymbol) {
            auto* module = ro::getModuleByIndex(sym->moduleIdx);

            if (sym->isVersion(sym->moduleIdx))
                *out = module->range().start() + sym->offsetIntoModule;
            else if (abort) {
                if (IsPreCalc) {
                    HK_ABORT("UnresolvedSymbol: %08x (Immediate_WrongVersion)", *destSymbol);
                } else {
                    HK_ABORT("UnresolvedSymbol: %s (Immediate_WrongVersion)", destSymbol);
                }
            }
        }

        void SymbolImmediate::apply(bool abort, ptr* out, const char* destSymbol) {
            applyImmediateSymbol<false>(abort, this, out, destSymbol);
        }

        void SymbolImmediate::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyImmediateSymbol<true>(abort, this, out, destSymbol);
        }

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyReadADRPGlobalSymbol(bool abort, const SymbolReadADRPGlobal* sym, ptr* out, const T* destSymbol) {
            /*if (sym->isVersion(sym->moduleIdx)) {
                SymbolEntry& entry = gSymbols[sym->symIdx];
                ptr at;
                entry.apply(&at, &sym->destNameMurmur);
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at)).succeeded(), "ReadADRPGlobal symbol failed %x", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at)).succeeded(), "ReadADRPGlobal symbol failed %s", destSymbol);
                }
            }*/

            SymbolEntry& entry = gSymbols[sym->symIdx];
            ptr at;
            entry.apply(abort, &at, &sym->destNameMurmur);

            if (abort) {
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at), sym->offsetToLoInstr).succeeded(), "ReadADRPGlobal symbol failed %x", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at), sym->offsetToLoInstr).succeeded(), "ReadADRPGlobal symbol failed %s", destSymbol);
                }
            }
        }

        void SymbolReadADRPGlobal::apply(bool abort, ptr* out, const char* destSymbol) {
            applyReadADRPGlobalSymbol<false>(abort, this, out, destSymbol);
        }

        void SymbolReadADRPGlobal::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyReadADRPGlobalSymbol<true>(abort, this, out, destSymbol);
        }

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyArithmeticSymbol(bool abort, const SymbolArithmetic* sym, ptr* out, const T* destSymbol) {
            SymbolEntry& entry = gSymbols[sym->symIdx];
            ptr at;
            entry.apply(abort, &at, &sym->destNameMurmur);
            *out = at + sym->addend;
        }

        void SymbolArithmetic::apply(bool abort, ptr* out, const char* destSymbol) {
            applyArithmeticSymbol<false>(abort, this, out, destSymbol);
        }

        void SymbolArithmetic::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyArithmeticSymbol<true>(abort, this, out, destSymbol);
        }

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyMultipleCandidateSymbol(SymbolMultipleCandidate* sym, ptr* out, const T* destSymbol) {
            *out = 0;
            for (fs32 i = 0; i < sym->numCandidates; i++) {
                SymbolEntry& cur = cast<SymbolEntry*>(uintptr_t(gSymbols) + sym->offsetToCandidates)[i];
                cur.apply(false, out, destSymbol);
                if (*out)
                    return;
            }

            if (IsPreCalc) {
                HK_ABORT("UnresolvedSymbol: %08x (Multiple out of %d candidates)", *destSymbol, sym->numCandidates);
            } else {
                HK_ABORT("UnresolvedSymbol: %s (Multiple out of %d candidates)", destSymbol, sym->numCandidates);
            }
        }

        void SymbolMultipleCandidate::apply(bool abort, ptr* out, const char* destSymbol) {
            applyMultipleCandidateSymbol(this, out, destSymbol);
        }

        void SymbolMultipleCandidate::apply(bool abort, ptr* out, const u32* destSymbol) {
            applyMultipleCandidateSymbol(this, out, destSymbol);
        }

    } // namespace detail

} // namespace hk::sail

#endif