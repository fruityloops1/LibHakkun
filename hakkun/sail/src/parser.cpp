#include "parser.h"
#include "config.h"
#include "hash.h"
#include "symbol.h"
#include "util.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sail {

    static std::deque<std::string> splitByWhitespaceAndRemoveComments(const std::string& data) {
        auto parts = split(data, ' ');
        std::deque<std::string> newParts;
        std::string braceBuf;

        for (auto& part : parts) {
            auto pos = part.find("//");
            if (pos != std::string::npos) {
                part.resize(pos);
                if (braceBuf.size() > 1) {
                    newParts.push_back(braceBuf.substr(1));
                }

                if (!part.empty())
                    newParts.push_back(part);
                break;
            }
            pos = part.find("#");
            if (pos != std::string::npos) {
                part.resize(pos);
                if (braceBuf.size() > 1) {
                    newParts.push_back(braceBuf.substr(1));
                }

                if (!part.empty())
                    newParts.push_back(part);
                break;
            }

            if (!braceBuf.empty()) {
                pos = part.find('}');
                if (pos != std::string::npos) {
                    braceBuf += part.substr(0, pos);
                    part = part.substr(pos + 1);
                    if (braceBuf.size() > 1) {
                        newParts.push_back(braceBuf.substr(1));
                    }

                    braceBuf.clear();
                    if (part.empty()) {
                        continue;
                    }
                } else {
                    braceBuf += part;
                    continue;
                }
            }

            pos = part.find('{');
            if (pos != std::string::npos) {
                braceBuf = part.substr(pos);
                part.resize(pos);
                if (!part.empty()) {
                    newParts.push_back(part);
                }
                continue;
            }

            newParts.push_back(part);
        }

        while (!newParts.empty() && newParts[0].empty())
            newParts.pop_front();

        return newParts;
    }

#define SYNTAX_ERROR(...)                                                                \
    {                                                                                    \
        char buf[1024];                                                                  \
        snprintf(buf, 1024, __VA_ARGS__);                                                \
        fail<3>("sail: syntax error at %s:%d: %s\n", filePath.c_str(), lineNumber, buf); \
    }

#define CHECK_DUPLICATE(NAME)                                 \
    if (addDestinationSymbolAndCheckDuplicate(NAME) == false) \
        SYNTAX_ERROR("duplicate symbol: %s", NAME.c_str());

    std::vector<Symbol> parseSymbolFile(const std::string& data, const std::string& filePath) {
        std::stringstream ss(data);

        std::string line;
        int lineNumber = 0;

        int currentModuleIndex = -1;
        std::string currentModule;
        std::deque<std::string> currentVersions;
        std::vector<u32> currentVersionIndices;

        std::vector<Symbol> symbols;

        while (std::getline(ss, line, '\n')) {
            lineNumber++;
            // printf("%d\n", lineNumber);
            auto parts = splitByWhitespaceAndRemoveComments(line);

            if (parts.empty())
                continue;

            // module/version marker
            if (parts[0].starts_with('@')) {
                parts[0] = parts[0].substr(1);
                if (parts[0].empty())
                    SYNTAX_ERROR("stray @");

                auto at = split(parts[0], ':');

                if (at.size() > 2)
                    SYNTAX_ERROR("invalid module/version marker");

                currentModule = at[0];
                currentModuleIndex = getModuleIndex(currentModule);
                if (currentModuleIndex == -1)
                    SYNTAX_ERROR("unknown module '%s'", currentModule.c_str());

                if (at.size() > 1)
                    currentVersions = split(at[1], ',');
                else
                    currentVersions.clear();

                currentVersionIndices.clear();
                for (auto version : currentVersions) {
                    int verIndex = getVersionIndex(currentModule, version);
                    if (verIndex == -1)
                        SYNTAX_ERROR("unknown version '%s'", version.c_str());
                    currentVersionIndices.push_back(verIndex);
                }

                // printf("Switch to module %s with versions ", currentModule.c_str());
                for (auto version : currentVersions) {
                    // printf("%s, ", version.c_str());
                }
                // printf("\n");
                continue;
            }

            auto addDynamicSymbol = [&symbols, filePath, lineNumber](const std::string& name, const std::string& dynamicSymbol) {
                Symbol newSymbol;
                newSymbol.name = name;
                newSymbol.type = Symbol::Type::Dynamic;
                newSymbol.dataDynamic.name = dynamicSymbol;
                newSymbol.versionIndices = {};
                symbols.push_back(newSymbol);
                CHECK_DUPLICATE(name);

                // printf("Dynamic Symbol: %s = %s\n", name.c_str(), dynamicSymbol.c_str());
            };

            auto addImmediateSymbol = [&symbols, currentVersionIndices, currentModuleIndex, filePath, lineNumber](const std::string& name, u32 offset) {
                Symbol newSymbol;
                newSymbol.name = name;
                newSymbol.type = Symbol::Type::Immediate;
                newSymbol.dataImmediate.moduleIdx = currentModuleIndex;
                newSymbol.dataImmediate.offset = offset;
                newSymbol.versionIndices = currentVersionIndices;
                symbols.push_back(newSymbol);

                // printf("Immediate Symbol: %s = 0x%08x\n", name.c_str(), offset);
            };

            // determine symbol type
            if (parts.size() == 3 && parts[1] == "=") { // immediate or dynamic or adrp global
                std::string destinationSymbol = parts[0];

                if (parts[2].starts_with("0x")) { // immediate
                    std::string num = parts[2].substr(2);
                    char* out;
                    u32 offset = strtoul(num.c_str(), &out, 16); // < this stdlib is ASS
                    if (out == num.c_str())
                        SYNTAX_ERROR("not a number '%s'", parts[2].c_str());
                    addImmediateSymbol(parts[0], offset);
                } else if (parts[2].starts_with('"')) { // dynamic
                    auto endquote = parts[2].rfind('"');

                    if (endquote == std::string::npos || endquote == 0)
                        SYNTAX_ERROR("missing end of quote");

                    std::string dynamicSymbol = parts[2].substr(1, parts[2].size() - 2);

                    addDynamicSymbol(parts[0], dynamicSymbol);
                } else if (parts[2].starts_with("readAdrpGlobal")) {
                    std::string symbolToAddTo = parts[2];
                    auto leftBracketAt = symbolToAddTo.find('(');
                    if (leftBracketAt == std::string::npos)
                        SYNTAX_ERROR("missing opening bracket");
                    if (*(symbolToAddTo.end() - 1) != ')')
                        SYNTAX_ERROR("missing closing bracket");
                    symbolToAddTo = symbolToAddTo.substr(leftBracketAt + 1);
                    symbolToAddTo = symbolToAddTo.substr(0, symbolToAddTo.size() - 1);

                    s32 offsetToLoInstr = 1;

                    auto commaAt = symbolToAddTo.find(',');
                    if (commaAt != std::string::npos) {
                        auto components = split(symbolToAddTo, ',');
                        symbolToAddTo = symbolToAddTo.substr(0, commaAt);
                        if (components.size() != 2)
                            SYNTAX_ERROR("readAdrpGlobal commas invalid");
                        if (components[1].starts_with("0x")) { // immediate
                            std::string num = components[1].substr(2);
                            char* out;
                            s32 offset = strtol(num.c_str(), &out, 16); // < this stdlib is ASS
                            if (out == num.c_str())
                                SYNTAX_ERROR("not a number '%s'", components[1].c_str());
                            offsetToLoInstr = offset;
                        } else
                            SYNTAX_ERROR("not a hex number '%s'", components[1].c_str());
                    }

                    Symbol newSymbol;
                    newSymbol.type = Symbol::Type::ReadADRPGlobal;
                    newSymbol.name = parts[0];
                    newSymbol.dataReadADRPGlobal.offsetToLoInstr = offsetToLoInstr;
                    newSymbol.dataReadADRPGlobal.atSymbol = symbolToAddTo;

                    symbols.push_back(newSymbol);
                } else
                    SYNTAX_ERROR("unknown symbol source '%s'", parts[2].c_str());
            } else if (parts.size() == 1) // dynamic
            {
                addDynamicSymbol(parts[0], parts[0]);
            } else if (parts.size() == 5 && (parts[3] == "+" or parts[3] == "-")) {
                s32 sign = parts[3] == "+" ? 1 : -1;

                std::string symbolToAddTo = parts[2];

                s32 offset = 0;
                if (parts[4].starts_with("0x")) {
                    std::string num = parts[4].substr(2);
                    char* out;
                    offset = strtoul(num.c_str(), &out, 16); // < this stdlib is ASS
                    if (out == num.c_str())
                        SYNTAX_ERROR("not a number '%s'", parts[4].c_str());
                } else
                    SYNTAX_ERROR("not a number '%s'", parts[4].c_str());

                offset *= sign;

                Symbol newSymbol;
                newSymbol.type = Symbol::Type::Arithmetic;
                newSymbol.name = parts[0];
                newSymbol.dataArithmetic.symbolToAddTo = symbolToAddTo.c_str();
                newSymbol.dataArithmetic.offset = offset;

                symbols.push_back(newSymbol);
            } else if (parts.size() == 5 || parts.size() == 7) { // data block
                std::vector<u8> bytes;
                std::vector<u8> bytesMask;
                if (!hexStringToBytesWithMatch(bytes, bytesMask, parts[2]))
                    SYNTAX_ERROR("failed parsing bytes '%s'", parts[2].c_str());

                if (parts[3] != "@")
                    SYNTAX_ERROR("weird symbol");

                std::string module = parts[4];
                std::deque<std::string> moduleParts = { module };

                Symbol::DataBlockVersionBoundary boundary = Symbol::DataBlockVersionBoundary::None;
                if (module.find('>') != std::string::npos) {
                    moduleParts = split(module, '>');
                    boundary = Symbol::DataBlockVersionBoundary::From;
                } else if (module.find('<') != std::string::npos) {
                    moduleParts = split(module, '<');
                    boundary = Symbol::DataBlockVersionBoundary::Older;
                }

                if (boundary != Symbol::DataBlockVersionBoundary::None && moduleParts.size() != 2)
                    SYNTAX_ERROR("invalid module");

                auto sectionParts = split(moduleParts[0], ':');

                int moduleIdx = getModuleIndex(sectionParts[0]);
                if (moduleIdx == -1)
                    SYNTAX_ERROR("unknown module '%s'", sectionParts[0].c_str());

                s32 versionIndex = 0;
                if (moduleParts.size() > 1) {
                    versionIndex = getVersionIndex(sectionParts[0], moduleParts[1]);
                    if (versionIndex == -1)
                        SYNTAX_ERROR("unknown version '%s'", moduleParts[1].c_str());
                }

                Symbol::Section section = Symbol::Section::All;
                if (sectionParts.size() == 1) {
                    // All
                } else if (sectionParts.size() == 2) {
                    if (sectionParts[1] == "text")
                        section = Symbol::Section::Text;
                    else if (sectionParts[1] == "rodata")
                        section = Symbol::Section::Rodata;
                    else if (sectionParts[1] == "data")
                        section = Symbol::Section::Data;
                    else
                        SYNTAX_ERROR("unknown section type '%s'", sectionParts[1].c_str());
                } else
                    SYNTAX_ERROR("invalid module section");

                s32 offset = 0;
                if (parts.size() == 7) {
                    if (parts[6].starts_with("0x")) {
                        std::string num = parts[6].substr(2);
                        char* out;
                        offset = strtoul(num.c_str(), &out, 16); // < this stdlib is ASS
                        if (out == num.c_str())
                            SYNTAX_ERROR("not a number '%s'", parts[6].c_str());
                    } else
                        SYNTAX_ERROR("not a number '%s'", parts[6].c_str());

                    if (parts[5] == "+")
                        ; // +
                    else if (parts[5] == "-")
                        offset *= -1;
                    else
                        SYNTAX_ERROR("unknown arithmetic operator '%s'", parts[5].c_str());
                }

                Symbol newSymbol;
                newSymbol.type = Symbol::Type::DataBlock;
                newSymbol.name = parts[0];
                newSymbol.dataDataBlock.data = bytes;
                newSymbol.dataDataBlock.dataMask = bytesMask;
                newSymbol.dataDataBlock.boundary = boundary;
                newSymbol.dataDataBlock.versionBoundary = versionIndex;
                newSymbol.dataDataBlock.sectionLimit = section;
                newSymbol.dataDataBlock.offs = offset;
                newSymbol.dataDataBlock.moduleIdx = moduleIdx;

                symbols.push_back(newSymbol);

                // printf("DataBlock Symbol: %s = bytes in module %d:section%d ver %d %x offsetted %d\n", parts[0].c_str(), 0, section, boundary, versionIndex, offset);

            } else
                SYNTAX_ERROR("not sure what this is");
        }

        return symbols;
    }

    ModuleList parseVersionList(const std::string& data, const std::string& filePath) {
        std::stringstream ss(data);

        std::string line;
        int lineNumber = 0;

        ModuleList modules;
        std::string currentModuleName;

        while (std::getline(ss, line, '\n')) {
            lineNumber++;

            auto parts = splitByWhitespaceAndRemoveComments(line);

            if (parts.empty())
                continue;

            if (parts[0].starts_with('@')) {
                if (parts.size() != 1 && parts.size() != 3)
                    SYNTAX_ERROR("not sure what this is");

                parts[0] = parts[0].substr(1);
                if (parts[0].empty())
                    SYNTAX_ERROR("stray @");

                currentModuleName = parts[0];

                if (parts.size() == 3) {
                    if (parts[1] != "=")
                        SYNTAX_ERROR("unknown operator");
                    modules[currentModuleName].mod0Name = parts[2];
                }
                continue;
            }

            if (parts.size() != 3)
                SYNTAX_ERROR("not sure what this is");

            if (parts[1] != "=")
                SYNTAX_ERROR("unknown operator");

            std::vector<u8> bytes;
            if (!hexStringToBytes(bytes, parts[2]))
                SYNTAX_ERROR("failed parsing bytes '%s'", parts[2].c_str());
            if (bytes.size() > 0x20)
                SYNTAX_ERROR("build id size cannot exceed 32 bytes");
            bytes.resize(cBuildIdSize);

            std::memcpy(modules[currentModuleName].versions[parts[0]].id, bytes.data(), cBuildIdSize);
        }

        return modules;
    }

} // namespace sail
