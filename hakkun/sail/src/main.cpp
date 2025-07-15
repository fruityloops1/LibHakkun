#include "config.h"
#include "fakelib.h"
#include "parser.h"
#include "util.h"
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 8)
        sail::fail<1>("%s <ModuleList> <VersionList> <OutFolder> <ClangBinary> <Is32Bit> <Version> <SymbolTraversePaths>\n", argv[0]);

    const char* moduleListPath = argv[1];
    const char* versionListPath = argv[2];
    const char* outFolder = argv[3];
    const char* clangBinary = argv[4];
    sail::is32Bit() = std::string(argv[5]) == "1";
    if (*argv[6] != 'F') {
        printf("Wrong sail version! Have you run setup_sail.py after updating?\n");
        return 1;
    }
    std::vector<const char*> symbolTraversePaths;
    for (int i = 7; i < argc; i++) {
        symbolTraversePaths.push_back(argv[i]);
    }

    sail::loadConfig(moduleListPath, versionListPath);

    printf("-- Parsing symbols\n");

    std::vector<sail::Symbol> symbols;

    for (auto i : symbolTraversePaths) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(i)) {
            if (!entry.path().string().ends_with(".sym"))
                continue;

            const char* path = entry.path().c_str();

            std::string data = sail::readFileString(path);
            auto syms = sail::parseSymbolFile(data, path);

            for (const auto& sym : syms)
                symbols.push_back(sym);
        }
    }

    printf("-- Generating symbols\n");
    sail::generateFakeLib(symbols, outFolder, clangBinary);
    sail::generateSymbolDb(symbols, outFolder, clangBinary);
    return 0;
}
