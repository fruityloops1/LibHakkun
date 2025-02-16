#pragma once

#include "types.h"
#include <string>
#include <vector>

namespace sail {

    struct Symbol {
        enum Type
        {
            Immediate,
            Dynamic,
            DataBlock,
            ReadADRPGlobal,
            Arithmetic,
            MultipleCandidate,
        };
        enum DataBlockVersionBoundary
        {
            None,
            Older, // older than version
            From, // version or newer than version
        };
        enum Section
        {
            All,
            Text,
            Rodata,
            Data, // .data and .bss and etc
        };

        bool useCache = true;
        Type type;
        struct {
            int moduleIdx;
            u32 offset;
        } dataImmediate;
        struct {
            std::string name;
        } dataDynamic;
        struct {
            int moduleIdx;
            std::vector<u8> data;
            u32 offs = 0;
            DataBlockVersionBoundary boundary = None;
            Section sectionLimit;
            u32 versionBoundary = 0;
        } dataDataBlock;
        struct {
            std::string atSymbol;
            s32 offsetToLoInstr;
        } dataReadADRPGlobal;
        struct {
            std::string symbolToAddTo;
            s32 offset;
        } dataArithmetic;

        std::string getDataBlockName() const {
            std::string name = "__sail_datablock_";
            name.append(std::to_string(dataDataBlock.versionBoundary));
            name.append("_");
            name.append(std::to_string(dataDataBlock.boundary));
            name.append("__");
            name.append(this->name);
            return name;
        }

        std::string name;
        std::vector<u32> versionIndices;
    };

} // namespace sail
