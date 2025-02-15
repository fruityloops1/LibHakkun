#include "fakelib.h"

#include "config.h"
#include "hash.h"
#include "util.h"
#include <algorithm>
#include <string>

namespace sail {
    static void compile(const char* outPath, const char* clangBinary, const std::string& source, const std::string& flags, const char* filename) {
        std::string cmd = clangBinary;
        cmd.append(" --target=aarch64-none-elf -march=armv8-a -mtune=cortex-a57 -nodefaultlibs -o ");
        cmd.append(outPath);
        cmd.append("/");
        cmd.append(filename);
        cmd.append(" ");
        cmd.append(flags);
        cmd.append(" -x assembler -");

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

        compile(outPath, clangBinary, asmFile, "-Wl,--shared -s", "fakesymbols.so");
    }

    template <typename T>
    std::string toHexString(T value) {
        std::stringstream ss;
        ss << std::hex << std::uppercase << value;
        return ss.str();
    }

    void generateSymbolDb(const std::vector<Symbol>& symbols, const char* outPath, const char* clangBinary) {

        std::vector<Symbol> sorted = symbols;

        std::sort(sorted.begin(), sorted.end(), [](const Symbol& a, const Symbol& b) -> bool {
            return hashMurmur(a.name.c_str()) < hashMurmur(b.name.c_str());
        });

        size_t bufSize = 0x1000 + symbols.size() * 0x80;

        std::string asmFile;
        asmFile.reserve(bufSize);

        asmFile.append(
            ".section \".saildb\",\"a\"\n"
            "\t.quad 0x0\n"
            ".global _ZN2hk4sail11gNumSymbolsE\n"
            ".global _ZN2hk4sail8gSymbolsE\n"
            ".global _ZN2hk4sail12gNumVersionsE\n"
            ".global _ZN2hk4sail9gVersionsE\n"
            "_ZN2hk4sail11gNumSymbolsE:\n"
            "\t.quad ");

        asmFile.append(std::to_string(symbols.size()));

        asmFile.append(
            "\n.align 8\n"
            "_ZN2hk4sail8gSymbolsE:\n");

        // symbols
        for (auto sym : sorted) {
            u32 hash = hashMurmur(sym.name.c_str());

            asmFile.append("\n.word 0x");
            asmFile.append(toHexString(hash));
            asmFile.append("\n.word 0x");
            asmFile.append(toHexString(sym.type));
            asmFile.append("\n.quad 0x"); // symbol cache
            asmFile.append(sym.useCache ? "0" : "1");

            if (sym.type == Symbol::Type::DataBlock) {
                asmFile.append("\n.quad ");
                asmFile.append(sym.getDataBlockName());
                asmFile.append(" - _ZN2hk4sail8gSymbolsE");
            } else if (sym.type == Symbol::Type::Dynamic) {
                asmFile.append("\n.quad 0x");
                asmFile.append(toHexString(rtldElfHash(sym.dataDynamic.name.c_str())));
            } else {
                u64 versions = 0;
                for (auto ver : sym.versionIndices)
                    versions |= 1 << ver;

                asmFile.append("\n.quad 0x");
                asmFile.append(toHexString(versions));
            }

            switch (sym.type) {
            case Symbol::Immediate: {
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(sym.dataImmediate.moduleIdx));
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(sym.dataImmediate.offset));
                break;
            }
            case Symbol::Dynamic: {
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(hashMurmur(sym.dataDynamic.name.c_str())));
                asmFile.append("\n.word 0x0");
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

                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(conv.data));
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(sym.dataDataBlock.offs));
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

                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(foundIdx - sorted.begin()));
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(*reinterpret_cast<u32*>(&sym.dataArithmetic.offset)));
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

                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(foundIdx - sorted.begin()));
                asmFile.append("\n.word 0x");
                asmFile.append(toHexString(*reinterpret_cast<u32*>(&sym.dataReadADRPGlobal.offsetToLoInstr)));
                break;
            }
            default:
                break;
            }
        }

        // data blocks

        for (auto sym : sorted) {
            if (sym.type != Symbol::Type::DataBlock)
                continue;

            std::string name = sym.getDataBlockName();

            asmFile.append("\n.global ");
            asmFile.append(name);
            asmFile.append("\n.align 8\n");
            asmFile.append(name);
            asmFile.append(":\n\t.quad 0x");
            asmFile.append(toHexString(sym.dataDataBlock.data.size()));

            asmFile.append("\n\t.byte ");

            {
                int i = 0;
                for (u8 byte : sym.dataDataBlock.data) {
                    if (i != 0)
                        asmFile.append(",");
                    char buf[4] { 0 };
                    snprintf(buf, 4, "%3u", byte);
                    asmFile.append(buf);
                    i++;
                }
            }
        }

        // module version list

        asmFile.append("\n.align 8");
        asmFile.append("\n_ZN2hk4sail9gVersionsE:");
        {
            int i = 0;
            for (auto module : getVersionList()) {
                if (module.empty())
                    asmFile.append("\n.quad 0x0");
                else {
                    asmFile.append("\n.quad module_versions_");
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
                }

                i++;
            }
        }

        compile(outPath, clangBinary, asmFile, "-c", "symboldb.o");
    }

} // namespace sail
