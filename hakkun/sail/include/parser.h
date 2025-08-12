#pragma once

#include "symbol.h"
#include <map>
#include <vector>

namespace sail {

    std::vector<Symbol> parseSymbolFile(const std::string& data, const std::string& filePath);

    constexpr static size cBuildIdSize = 0x10;
    struct BuildId {
        u8 id[cBuildIdSize] { 0 };
    };

    using VersionListModule = std::map<std::string, BuildId>;
    using ModuleList = std::map<std::string, VersionListModule>;

    ModuleList parseVersionList(const std::string& data, const std::string& filePath);

} // namespace sail
