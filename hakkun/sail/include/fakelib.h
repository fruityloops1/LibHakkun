#pragma once

#include "symbol.h"

namespace sail {

    void generateFakeLib(const std::vector<Symbol>& symbols, const char* outPath, const char* clangBinary);
    void generateSymbolDb(const std::vector<Symbol>& symbols, const char* outPath, const char* clangBinary);

} // namespace sail
