#pragma once

#include "symbol.h"
#include <map>
#include <vector>

namespace sail {

    std::vector<Symbol> parseSymbolFile(std::string data, const std::string& filePath);

    constexpr static size cBuildIdSize = 0x10;
    struct BuildId {
        u8 id[cBuildIdSize] { 0 };
    };

    struct VersionListModule {
        std::string mod0Name;
        int index = -1;
        std::map<std::string, BuildId> versions;
    };

    using ModuleList = std::map<std::string, VersionListModule>;

    ModuleList parseVersionList(std::string data, const std::string& filePath);

} // namespace sail
