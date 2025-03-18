#include "config.h"
#include "hash.h"
#include "parser.h"
#include "util.h"
#include <algorithm>

namespace sail {

    static std::vector<std::pair<std::string, int>> sModuleList;
    static std::vector<std::vector<std::pair<std::string, std::vector<u8>>>> sVersionList;

    void loadConfig(const std::string& moduleListPath, const std::string& versionListPath) {
        std::string data = sail::readFileString(moduleListPath.c_str());

        sModuleList = sail::parseModuleList(data, moduleListPath);
        data = sail::readFileString(versionListPath.c_str());

        sVersionList = sail::parseVersionList(data, versionListPath, sModuleList);
    }

    int getModuleIndex(const std::string& module) {
        for (const auto& pair : sModuleList)
            if (pair.first == module)
                return pair.second;
        return -1;
    }

    int getVersionIndex(int moduleIdx, const std::string& version) {
        for (int i = 0; i < sVersionList[moduleIdx].size(); i++) {
            if (sVersionList[moduleIdx][i].first == version)
                return i;
        }

        return -1;
    }

    const std::vector<std::vector<std::pair<std::string, std::vector<u8>>>>& getVersionList() { return sVersionList; }

    static std::vector<u32> sSymbolHashes;

    void clearDestinationSymbols() { sSymbolHashes.clear(); }
    bool addDestinationSymbolAndCheckDuplicate(const std::string& symbol) {
        u32 hash = hashMurmur(symbol.c_str());
        auto it = std::find(sSymbolHashes.begin(), sSymbolHashes.end(), hash);
        if (it != sSymbolHashes.end())
            return false;

        sSymbolHashes.push_back(hash);
        return true;
    }

    bool& is32Bit() {
        static bool sIs32Bit = false;
        return sIs32Bit;
    }

} // namespace sail
