#pragma once

#include "hk/util/Algorithm.h"
#include <source_location>

namespace hk::diag {

    class SourceLocation {
        std::source_location mLoc;

    public:
        constexpr SourceLocation(const SourceLocation&) = default;
        constexpr SourceLocation(std::source_location loc)
            : mLoc(loc) { }

        constexpr const char* file() { return mLoc.file_name(); }
        constexpr const char* function() { return mLoc.function_name(); }
        constexpr u32 line() { return mLoc.line(); }
        constexpr u16 column() { return u16(mLoc.column()); }

        consteval static SourceLocation current(std::source_location cur = std::source_location::current()) { return cur; }
    };

} // namespace hk::diag
