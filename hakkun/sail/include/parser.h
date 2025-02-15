#pragma once

#include "symbol.h"
#include <vector>

namespace sail {

    std::vector<Symbol> parseSymbolFile(const std::string& data, const std::string& filePath);
    std::vector<std::pair<std::string, int>> parseModuleList(const std::string& data, const std::string& filePath);
    std::vector<std::vector<std::pair<std::string, std::vector<u8>>>> parseVersionList(const std::string& data, const std::string& filePath, const std::vector<std::pair<std::string, int>>& moduleList);

} // namespace sail
