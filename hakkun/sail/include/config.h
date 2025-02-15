#pragma once

#include "types.h"
#include <string>
#include <vector>

namespace sail {

    void loadConfig(const std::string& moduleListPath, const std::string& versionListPath);
    int getModuleIndex(const std::string& module);
    int getVersionIndex(int moduleIdx, const std::string& version);

    const std::vector<std::vector<std::pair<std::string, std::vector<u8>>>>& getVersionList();

    void clearDestinationSymbols();
    bool addDestinationSymbolAndCheckDuplicate(const std::string& symbol);

} // namespace sail
