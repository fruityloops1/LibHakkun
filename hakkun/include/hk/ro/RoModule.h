#pragma once

#include "hk/types.h"
#include "rtld/RoModule.h"

namespace hk::ro {

    class RoModule {
    public:
        class Range {
            ptr mStart = 0;
            ::size mSize = 0;

        public:
            ::size size() const { return mSize; }
            ptr start() const { return mStart; }
            ptr end() const { return mStart + mSize; }

            Range() = default;
            Range(ptr start, ::size size)
                : mStart(start)
                , mSize(size) { }
        };

    private:
        nn::ro::detail::RoModule* mModule = nullptr;
        Range mTextRange;
        Range mRodataRange;
        Range mDataRange;

        Range mTextRwMapping;
        Range mRodataRwMapping;

    public:
        Range range() const { return { mTextRange.start(), mTextRange.size() + mRodataRange.size() + mDataRange.size() }; }
        Range text() const { return mTextRange; }
        Range rodata() const { return mRodataRange; }
        Range data() const { return mDataRange; }

        nn::ro::detail::RoModule* getNnModule() const { return mModule; }

        Result findRanges();
        Result mapRw();

        Result writeRo(ptr offset, const void* source, size writeSize) const;

        template <typename T>
        Result writeRo(ptr offset, const T& value) const {
            return writeRo(offset, &value, sizeof(T));
        }

        const char* getModuleName() const {
            struct {
                u32 _0;
                u32 nameLength;
                char name[];
            }* name { (typeof(name))mRodataRange.start() };
            return name->name;
        }

        friend class RoUtil;
    };

    using RoWriteCallback = void (*)(const RoModule* module, ptr offsetIntoModule, const void* source, size writeSize);
    void setRoWriteCallback(RoWriteCallback callback);

} // namespace hk::ro
