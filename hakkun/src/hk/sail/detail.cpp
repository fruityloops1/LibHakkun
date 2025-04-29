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
            diag::debugLog("hk::sail: loading module versions");
            for (int i = 0; i < ro::getNumModules(); i++) {
                sModuleVersions[i] = -1;
                uintptr_t versionsStart = uintptr_t(gVersions);

                uintptr_t versionsOffset = gVersions[i];
                if (versionsOffset == 0) {
                    diag::debugLog("hk::sail: Module[%d] VersionIndex: Skipped", i);
                    continue;
                }
                const u32* versions = cast<const u32*>(versionsStart + versionsOffset);

                u8 curBuildId[ro::sBuildIdSize];
                if (ro::getModuleBuildIdByIndex(i, curBuildId).failed())
                    continue;

                u32 numVersions = versions[0];
                versions++;

                struct {
                    u8 data[ro::sBuildIdSize];
                }* buildIds { decltype(buildIds)(versions) };

                bool found = false;
                for (int versionIndex = 0; versionIndex < numVersions; versionIndex++) {
                    if (__builtin_memcmp(buildIds[versionIndex].data, curBuildId, sizeof(curBuildId)) == 0) {
                        sModuleVersions[i] = versionIndex;
                        found = true;
                        break;
                    }
                }

                if (found)
                    diag::debugLog("hk::sail: Module[%d] VersionIndex: %d", i, sModuleVersions[i]);
                else
                    diag::debugLog("hk::sail: Module[%d] VersionIndex: NotFound", i);
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

            const SymbolDataBlock::SearchFunction search = cast<const SymbolDataBlock::SearchFunction>(uintptr_t(gSymbols) + sym->offsetToSearchFunction);

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

            ptr address = search(range.start(), range.size());

            if (abort) {
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %08x (DataBlock)", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(address != 0, "UnresolvedSymbol: %s (DataBlock)", destSymbol);
                }
            } else if (address == 0) {
                if (IsPreCalc) {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %08x (DataBlock)", *destSymbol);
                } else {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %s (DataBlock)", destSymbol);
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
            } else if (address == 0) {
                if (IsPreCalc) {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %08x (Dynamic)", *destSymbol);
                } else {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %s (Dynamic)", destSymbol);
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
            } else {
                if (IsPreCalc) {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %08x (Immediate_WrongVersion)", *destSymbol);
                } else {
                    diag::debugLog("hk::sail: UnresolvedSymbol: %s (Immediate_WrongVersion)", destSymbol);
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

#ifdef __aarch64__
            if (abort) {
                if (IsPreCalc) {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at), sym->offsetToLoInstr).succeeded(), "ReadADRPGlobal symbol failed %x", *destSymbol);
                } else {
                    HK_ABORT_UNLESS(hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at), sym->offsetToLoInstr).succeeded(), "ReadADRPGlobal symbol failed %s", destSymbol);
                }
            } else if (hk::hook::readADRPGlobal(out, cast<hook::Instr*>(at), sym->offsetToLoInstr).failed()) {
                if (IsPreCalc) {
                    diag::debugLog("hk::sail: ReadADRPGlobal symbol failed %x", *destSymbol);
                } else {
                    diag::debugLog("hk::sail: ReadADRPGlobal symbol failed %s", destSymbol);
                }
            }
#else
            HK_ABORT("ReadADRPGlobal not implemented for this architecture", 0);
#endif
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