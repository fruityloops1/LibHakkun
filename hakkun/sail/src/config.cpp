#include "config.h"
#include "hash.h"
#include "parser.h"
#include "util.h"
#include <algorithm>

namespace sail {

    static ModuleList sVersionList;

    void loadConfig(const std::string& moduleListPath, const std::string& versionListPath) {
        std::string data = sail::readFileString(versionListPath.c_str());

        sVersionList = sail::parseVersionList(data, versionListPath);
    }

    int getModuleIndex(const std::string& moduleName) {
        int idx = 0;
        for (const auto& module : sVersionList) {
            if (module.first == moduleName)
                return idx;
            idx++;
        }
        return -1;
    }

    int getVersionIndex(const std::string& moduleName, const std::string& version) {
        if (!sVersionList.contains(moduleName))
            return -1;

        const auto& module = sVersionList.at(moduleName);

        int idx = 0;
        for (const auto& pair : module) {
            if (pair.first == version)
                return idx;
            idx++;
        }

        return -1;
    }

    const ModuleList& getVersionList() { return sVersionList; }

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
