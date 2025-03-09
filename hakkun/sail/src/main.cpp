#include "config.h"
#include "fakelib.h"
#include "parser.h"
#include "util.h"
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc != 7)
        sail::fail<1>("%s <ModuleList> <VersionList> <SymbolTraversePath> <OutFolder> <ClangBinary> <Is32Bit>\n", argv[0]);

    const char* moduleListPath = argv[1];
    const char* versionListPath = argv[2];
    const char* symbolTraversePath = argv[3];
    const char* outFolder = argv[4];
    const char* clangBinary = argv[5];
    sail::is32Bit() = argc == 7 && std::string(argv[6]) == "1";

    sail::loadConfig(moduleListPath, versionListPath);

    printf("-- Parsing symbols\n");

    std::vector<sail::Symbol> symbols;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(symbolTraversePath)) {
        if (!entry.path().string().ends_with(".sym"))
            continue;

        const char* path = entry.path().c_str();

        std::string data = sail::readFileString(path);
        auto syms = sail::parseSymbolFile(data, path);

        for (const auto& sym : syms)
            symbols.push_back(sym);
    }

    printf("-- Generating symbols\n");
    sail::generateFakeLib(symbols, outFolder, clangBinary);
    sail::generateSymbolDb(symbols, outFolder, clangBinary);
    return 0;
}
