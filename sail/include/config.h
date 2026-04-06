#pragma once

#include "parser.h"
#include "types.h"
#include <string>
#include <vector>

namespace sail {

    void loadConfig(const std::string& moduleListPath, const std::string& versionListPath);
    int getModuleIndex(const std::string& moduleName);
    int getVersionIndex(const std::string& moduleName, const std::string& version);

    const ModuleList& getVersionList();

    void clearDestinationSymbols();
    bool addDestinationSymbolAndCheckDuplicate(const std::string& symbol);

    bool& is32Bit();

} // namespace sail
