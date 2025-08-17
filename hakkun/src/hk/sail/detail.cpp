#ifndef HK_DISABLE_SAIL

#include "hk/sail/detail.h"
#include "hk/diag/diag.h"
#include "hk/hook/InstrUtil.h"
#include "hk/ro/RoModule.h"
#include "hk/ro/RoUtil.h"
#include "hk/types.h"
#include "hk/util/hash.h"

namespace hk::sail {

    namespace detail {

        void VersionLoader::loadVersions() {
            diag::debugLog("hk::sail: loading module versions");
            for (int i = 0; i < gNumModules; i++) {
                uintptr_t versionsStart = uintptr_t(gVersions);

                uintptr_t versionsOffset = gVersions[i * 2];
                uintptr_t mod0NameOffsetOrVersionIndex = gVersions[i * 2 + 1];
                int moduleIndexOverride = mod0NameOffsetOrVersionIndex & bit(16) ? mod0NameOffsetOrVersionIndex & bits(4) : -1;
                const char* mod0Name = mod0NameOffsetOrVersionIndex == 0 || moduleIndexOverride != -1 ? nullptr : cast<const char*>(versionsStart + mod0NameOffsetOrVersionIndex);

                if (versionsOffset == 0) {
                    if (mod0Name != nullptr)
                        diag::debugLog("hk::sail: Module[%s] Version: Skipped", mod0Name);
                    else
                        diag::debugLog("hk::sail: Module[%d] Version: Skipped", i);
                    continue;
                }
                const u32* versions = cast<const u32*>(versionsStart + versionsOffset);

                u32 numVersions = versions[0];
                versions++;

                struct {
                    u8 data[ro::cBuildIdSize];
                    char name[8];
                }* buildIds { decltype(buildIds)(versions) };

                ro::RoModule* module = nullptr;

                bool versionFound = false;
                for (int moduleIndex = 0; moduleIndex < ro::getNumModules(); moduleIndex++) {
                    module = ro::getModuleByIndex(moduleIndex);

                    const char* moduleName = module->getModuleName();
                    if (mod0Name != nullptr && moduleName != nullptr && strstr(module->getModuleName(), mod0Name))
                        gModules[i] = module;
                    if (moduleIndex == moduleIndexOverride)
                        gModules[i] = module;

                    const u8* curBuildId = module->getBuildId();

                    if (curBuildId == nullptr)
                        continue;

                    for (int versionIndex = 0; versionIndex < numVersions; versionIndex++) {
                        if (__builtin_memcmp(buildIds[versionIndex].data, curBuildId, ro::cBuildIdSize) == 0) {
                            constexpr size cNameSize = sizeof(buildIds[versionIndex].name);

                            module->mVersionIndex = versionIndex;

                            __builtin_memcpy(module->mVersionName, buildIds[versionIndex].name, cNameSize);
                            module->mVersionName[cNameSize] = '\0';
                            module->mVersionNameHash = util::hashMurmur(module->mVersionName);

                            gModules[i] = module;
                            versionFound = true;
                            goto found;
                        }
                    }
                }
            found:

                if (mod0Name != nullptr) {
                    if (versionFound)
                        diag::debugLog("hk::sail: Module[%s] Version: %s (idx: %d)", mod0Name, module->getVersionName(), module->getVersionIndex());
                    else
                        diag::debugLog("hk::sail: Module[%s] Version: NotFound", mod0Name);
                } else {
                    if (versionFound)
                        diag::debugLog("hk::sail: Module[%d] Version: %s (idx: %d)", i, module->getVersionName(), module->getVersionIndex());
                    else
                        diag::debugLog("hk::sail: Module[%d] Version: NotFound", i);
                }
            }
        }

        // Versioned

        bool SymbolVersioned::isVersion(u32 moduleIdx) const {
            return (versionsBitset >> gModules[moduleIdx]->getVersionIndex()) & 0b1;
        }

        // DataBlock

        _HK_SAIL_PRECALC_TEMPLATE
        hk_alwaysinline void applyDataBlockSymbol(bool abort, const SymbolDataBlock* sym, ptr* out, const T* destSymbol) {
            const auto* module = gModules[sym->moduleIdx];

            HK_ABORT_UNLESS(module != nullptr, "UnknownModule with idx %d", sym->moduleIdx);

            const SymbolDataBlock::SearchFunction search = cast<const SymbolDataBlock::SearchFunction>(uintptr_t(gSymbols) + sym->offsetToSearchFunction);

            s32 loadedVer = module->getVersionIndex();

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
                range = module->text();
                break;
            case 2:
                range = module->rodata();
                break;
            case 3:
                range = module->data();
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
            ptr address = ro::lookupSymbol(sym->lookupNameRtldHash, sym->lookupNameDjb2, sym->lookupNameMurmur);

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
            auto* module = gModules[sym->moduleIdx];

            HK_ABORT_UNLESS(module != nullptr, "UnknownModule with idx %d", sym->moduleIdx);

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
            SymbolEntry* srcSym = lookupSymbolByHash(sym->srcNameMurmur);

            HK_ASSERT(srcSym != nullptr);

            ptr at;
            srcSym->apply(abort, &at, &sym->destNameMurmur);

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
            SymbolEntry* srcSym = lookupSymbolByHash(sym->srcNameMurmur);

            HK_ASSERT(srcSym != nullptr);

            ptr at;
            srcSym->apply(abort, &at, &sym->destNameMurmur);
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
            // prioritize immediates
            for (fs32 i = 0; i < sym->numCandidates; i++) {
                SymbolEntry& cur = cast<SymbolEntry*>(uintptr_t(gSymbols) + sym->offsetToCandidates)[i];

                if (cur.getType() != Symbol::Type_Immediate)
                    continue;

                cur.apply(false, out, destSymbol);
                if (*out)
                    return;
            }

            // other
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