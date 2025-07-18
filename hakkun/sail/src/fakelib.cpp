#include "fakelib.h"

#include "config.h"
#include "hash.h"
#include "util.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <unordered_map>

namespace sail {
    static void compile(const char* outPath, const char* clangBinary, const char* language, const std::string& source, const std::string& flags, const char* filename) {
        std::string cmd = clangBinary;

        if (is32Bit()) {
            cmd.append(" --target=armv7a-none-eabi -march=armv7-a");
        } else
            cmd.append(" --target=aarch64-none-elf -march=armv8-a");
        cmd.append(" -mtune=cortex-a57 -nodefaultlibs -nostartfiles -Wno-unused-command-line-argument -o ");

        cmd.append(outPath);
        cmd.append("/");
        cmd.append(filename);
        cmd.append(" ");
        cmd.append(flags);
        cmd.append(" -x ");
        cmd.append(language);
        cmd.append(" -");

        // printf("   - %s\n", cmd.c_str());
        // printf("Source\n%s\n", source.c_str());
        FILE* compilerPipe = popen(cmd.c_str(), "w");
        if (compilerPipe == nullptr) {
            fail<4>("pipe fail");
        }

        fputs(source.c_str(), compilerPipe);

        int compileResult = pclose(compilerPipe);
        if (compileResult != 0) {
            fail<5>("clang compilation failed");
        }
    }

    void generateFakeLib(const std::vector<Symbol>& symbols, const char* outPath, const char* clangBinary) {
        std::vector<std::string> symbolNames;
        clearDestinationSymbols();
        for (const Symbol& symbol : symbols)
            if (addDestinationSymbolAndCheckDuplicate(symbol.name))
                symbolNames.push_back(symbol.name);

        size_t bufSize = 0x100;
        for (auto sym : symbolNames)
            bufSize += sym.size() + 1;

        std::string asmFile;
        asmFile.reserve(bufSize);

        asmFile.append(".section \".text\",\"a\"\n");
        asmFile.append(
            ".quad 0x0\n");

        for (auto sym : symbolNames) {
            asmFile.append("\n.global ");
            asmFile.append(sym);
        }
        asmFile.append("\n");
        for (auto sym : symbolNames) {
            asmFile.append(sym);
            asmFile.append(":\n\t.quad 0x0\n");
        }

        compile(outPath, clangBinary, "assembler", asmFile, "-Wl,--shared -s", "fakesymbols.so");
    }

    template <typename T>
    std::string toHexString(T value) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << value;
        return ss.str();
    }

    static void generateDataBlockSearchFunction(std::string& outSrcCpp, const Symbol& _symbol) {
        const auto& sym = _symbol.dataDataBlock;

        outSrcCpp.append(
            "\nextern \"C\" uintptr_t ");
        outSrcCpp.append(_symbol.getDataBlockSearchFunctionName());
        outSrcCpp.append("(uintptr_t start, size_t len) {");

        struct Block {
            int bitCount = 0;
        };

        const auto addArr = [&](const char* name, const std::vector<u8>& data) {
            outSrcCpp.append("\nconstexpr uint8_t ");
            outSrcCpp.append(name);
            outSrcCpp.append("[] {");
            for (size_t i = 0; i < sym.data.size(); i++) {
                outSrcCpp.append("0x");
                outSrcCpp.append(toHexString(u32(data[i])));
                outSrcCpp.append(",");
            }
            outSrcCpp.append("};");
        };

        addArr("cData", sym.data);
        addArr("cDataMask", sym.dataMask);

        outSrcCpp.append("\nconst uintptr_t end = start + len - sizeof(cData);");

        outSrcCpp.append("\nfor (uintptr_t search = start; search < end; search += 4)");
        outSrcCpp.append("\nif (compareMask((const uint8_t*)search, cData, cDataMask, sizeof(cData)))");
        outSrcCpp.append("\nreturn search;");
        outSrcCpp.append("\nreturn 0;");

        outSrcCpp.append("}");
    }

    void generateSymbolDb(const std::vector<Symbol>& symbols, const char* outPath, const char* clangBinary) {

        std::vector<Symbol> sorted = symbols;

        std::sort(sorted.begin(), sorted.end(), [](const Symbol& a, const Symbol& b) -> bool {
            return hashMurmur(a.name.c_str()) < hashMurmur(b.name.c_str());
        });

        size_t bufSize = 0x1000 + symbols.size() * 0x80;
        size_t curNumSymbols = 0;
        size_t numDataBlocks = 0;

        std::string asmFile;
        asmFile.reserve(bufSize);

        asmFile.append(
            ".section \".saildb\",\"a\"\n"
            "\t.quad 0x0\n"
            ".global _ZN2hk4sail11gNumSymbolsE\n"
            ".global _ZN2hk4sail8gSymbolsE\n"
            ".global _ZN2hk4sail12gNumVersionsE\n"
            ".global _ZN2hk4sail9gVersionsE\n");

        asmFile.append(
            "\n.align 8\n"
            "_ZN2hk4sail8gSymbolsE:\n");

        auto insertSymbol = [&](std::string& out, const Symbol& sym) {
            u32 hash = hashMurmur(sym.name.c_str());

            out.append("\n.word 0x");
            out.append(toHexString(hash));
            out.append("\n.word 0x");
            out.append(toHexString(sym.type));
            out.append("\n.quad 0x"); // symbol cache
            out.append(sym.useCache ? "0" : "1");

            if (sym.type == Symbol::Type::DataBlock) {
                const auto funcName = sym.getDataBlockSearchFunctionName();
                out.append("\n.quad ");
                out.append(funcName);
                out.append("- _ZN2hk4sail8gSymbolsE");
                numDataBlocks++;
            } else if (sym.type == Symbol::Type::Dynamic) {
                out.append("\n.quad 0x");
                out.append(toHexString(rtldElfHash(sym.dataDynamic.name.c_str())));
            } else {
                u64 versions = 0;
                for (auto ver : sym.versionIndices)
                    versions |= 1 << ver;

                out.append("\n.quad 0x");
                out.append(toHexString(versions));
            }

            switch (sym.type) {
            case Symbol::Immediate: {
                out.append("\n.word 0x");
                out.append(toHexString(sym.dataImmediate.moduleIdx));
                out.append("\n.word 0x");
                out.append(toHexString(sym.dataImmediate.offset));
                break;
            }
            case Symbol::Dynamic: {
                out.append("\n.word 0x");
                out.append(toHexString(hashMurmur(sym.dataDynamic.name.c_str())));
                out.append("\n.word 0x");
                out.append(toHexString(djb2Hash(sym.dataDynamic.name.c_str())));
                break;
            }
            case Symbol::DataBlock: {
                union {
                    struct {
                        u8 moduleIdx;
                        u8 versionBoundaryType;
                        u8 versionBoundary;
                        u8 sectionLimit;
                    };
                    u32 data;
                } conv { u8(sym.dataDataBlock.moduleIdx), u8(sym.dataDataBlock.boundary), u8(sym.dataDataBlock.versionBoundary), u8(sym.dataDataBlock.sectionLimit) };

                out.append("\n.word 0x");
                out.append(toHexString(conv.data));
                out.append("\n.word 0x");
                out.append(toHexString(sym.dataDataBlock.offs));
                break;
            }
            case Symbol::Arithmetic: {
                u32 searchHash = hashMurmur(sym.dataArithmetic.symbolToAddTo.c_str());
                const auto foundIdx = std::find_if(sorted.begin(), sorted.end(), [&](const Symbol& symbol) -> bool {
                    if (hashMurmur(symbol.name.c_str()) == searchHash)
                        return true;
                    return false;
                });

                if (foundIdx == sorted.end()) {
                    printf("Arithmetic symbol: %s does not exist\n", sym.dataArithmetic.symbolToAddTo.c_str());
                    abort();
                }

                out.append("\n.word 0x");
                out.append(toHexString(hashMurmur(foundIdx->name.c_str())));
                out.append("\n.word 0x");
                out.append(toHexString(*reinterpret_cast<const u32*>(&sym.dataArithmetic.offset)));
                break;
            }
            case Symbol::ReadADRPGlobal: {
                u32 searchHash = hashMurmur(sym.dataReadADRPGlobal.atSymbol.c_str());
                const auto foundIdx = std::find_if(sorted.begin(), sorted.end(), [&](const Symbol& symbol) -> bool {
                    if (hashMurmur(symbol.name.c_str()) == searchHash)
                        return true;
                    return false;
                });

                if (foundIdx == sorted.end()) {
                    printf("ReadADRPGlobal symbol: %s does not exist\n", sym.dataReadADRPGlobal.atSymbol.c_str());
                    abort();
                }

                out.append("\n.word 0x");
                out.append(toHexString(hashMurmur(foundIdx->name.c_str())));
                out.append("\n.word 0x");
                out.append(toHexString(*reinterpret_cast<const u32*>(&sym.dataReadADRPGlobal.offsetToLoInstr)));
                break;
            }
            default:
                break;
            }
        };

        std::unordered_map<u32, std::vector<Symbol>> hashGroups;

        for (auto sym : sorted) {
            u32 hash = hashMurmur(sym.name.c_str());
            hashGroups[hash].push_back(sym);
        }

        std::string candidateSyms;

        // symbols
        int i = 0;
        for (const auto& sym : sorted) {
            u32 hash = hashMurmur(sym.name.c_str());
            auto& group = hashGroups[hash];
            if (group.empty())
                continue;
            if (group.size() > 1) {
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(hash));
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(Symbol::MultipleCandidate));
                asmFile.append("\n.quad 0x0");
                asmFile.append("\n.quad symlist_" + sym.name + " - _ZN2hk4sail8gSymbolsE");
                asmFile.append("\n.quad 0x");
                asmFile.append(toHexString(group.size()));

                candidateSyms.append("\nsymlist_" + sym.name + ":\n");
                for (auto msym : group)
                    insertSymbol(candidateSyms, msym);
                group.clear();
            } else
                insertSymbol(asmFile, sym);
            curNumSymbols++;
        }

        asmFile.append("\n_ZN2hk4sail11gNumSymbolsE:");
        asmFile.append("\n.quad 0x" + toHexString(curNumSymbols));

        asmFile.append(candidateSyms);

        // module version list

        asmFile.append("\n.align 8");
        asmFile.append("\n_ZN2hk4sail9gVersionsE:");
        {
            int i = 0;
            for (auto module : getVersionList()) {

                if (module.empty())
                    asmFile.append("\n.word 0x0");
                else {
                    asmFile.append("\n.word module_versions_");
                    asmFile.append(std::to_string(i));
                    asmFile.append(" - _ZN2hk4sail9gVersionsE");
                }

                i++;
            }
        }

        // version lists

        {
            int i = 0;
            for (auto module : getVersionList()) {
                asmFile.append("\nmodule_versions_");
                asmFile.append(std::to_string(i));
                asmFile.append(":");

                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(module.size()));

                for (auto version : module) {
                    asmFile.append("\n.byte ");
                    int i = 0;
                    for (u8 byte : version.second) {
                        if (i != 0)
                            asmFile.append(",");
                        char buf[4] { 0 };
                        snprintf(buf, 4, "%3u", byte);
                        asmFile.append(buf);
                        i++;
                    }
                    for (int i = 0; i < 8; i++) {
                        if (i < version.first.size()) {
                            asmFile.append("\n.ascii \"");
                            asmFile += version.first.at(i);
                            asmFile.append("\"");
                        } else
                            asmFile.append("\n.ascii \"\\0\"");
                    }
                }

                i++;
            }
        }

        // data blocks

        std::string dataBlockSearchCppSource = R"(
            using uintptr_t = unsigned long;
            using size_t = unsigned long;
            using uint8_t = unsigned char;
            using uint16_t = unsigned short;
            using uint32_t = unsigned int;
            using uint64_t = unsigned long long;

            static bool compareMask(const uint8_t* compareData, const uint8_t* data, const uint8_t* maskData, size_t n)
            {
                size_t n8 = n / sizeof(uint64_t);
                for (size_t i = 0; i < n8; i++) {
                    const uint64_t* compare64 = reinterpret_cast<const uint64_t*>(compareData + i * sizeof(uint64_t));
                    const uint64_t* data64 = reinterpret_cast<const uint64_t*>(data + i * sizeof(uint64_t));
                    const uint64_t* mask64 = reinterpret_cast<const uint64_t*>(maskData + i * sizeof(uint64_t));

                    if ((*compare64 & *mask64) != (*data64 & *mask64))
                        return false;
                }

                size_t processed = n8 * sizeof(uint64_t);

                if ((n - processed) >= sizeof(uint32_t)) {
                    const uint32_t* compare32 = reinterpret_cast<const uint32_t*>(compareData + processed);
                    const uint32_t* data32 = reinterpret_cast<const uint32_t*>(data + processed);
                    const uint32_t* mask32 = reinterpret_cast<const uint32_t*>(maskData + processed);

                    if ((*compare32 & *mask32) != (*data32 & *mask32))
                        return false;

                    processed += sizeof(uint32_t);
                }

                if ((n - processed) >= sizeof(uint16_t)) {
                    const uint16_t* compare16 = reinterpret_cast<const uint16_t*>(compareData + processed);
                    const uint16_t* data16 = reinterpret_cast<const uint16_t*>(data + processed);
                    const uint16_t* mask16 = reinterpret_cast<const uint16_t*>(maskData + processed);

                    if ((*compare16 & *mask16) != (*data16 & *mask16))
                        return false;

                    processed += sizeof(uint16_t);
                }

                for (size_t i = processed; i < n; i++)
                {
                    const uint8_t mask = maskData[i];
                    if ((compareData[i] & mask) != (data[i] & mask))
                        return false;
                }
                return true;
            }
        )";
        dataBlockSearchCppSource.reserve(0x1000 + numDataBlocks * 0x100);

        for (const Symbol& sym : sorted)
            if (sym.type == Symbol::Type::DataBlock)
                generateDataBlockSearchFunction(dataBlockSearchCppSource, sym);

        compile(outPath, clangBinary, "c++", dataBlockSearchCppSource, "-c -O3", "datablocks.o");
        compile(outPath, clangBinary, "assembler", asmFile, "-c", "symboldb.o");
    }

} // namespace sail
