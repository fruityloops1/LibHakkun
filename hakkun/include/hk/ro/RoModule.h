#pragma once

#include "hk/types.h"
#include "rtld/RoModule.h"

namespace hk::ro {

    struct RoModule {
        struct Range {
            ::size size() const { return mSize; }
            ptr start() const { return mStart; }
            ptr end() const { return mStart + mSize; }

            Range() = default;
            Range(ptr start, ::size size)
                : mStart(start)
                , mSize(size) { }

        private:
            ptr mStart = 0;
            ::size mSize = 0;
        };

        nn::ro::detail::RoModule* module = nullptr;
        Range text;
        Range rodata;
        Range data;

        Range range() const { return { text.start(), text.size() + rodata.size() + data.size() }; }
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
            }* name { (typeof(name))rodata.start() };
            return name->name;
        }

    private:
        Range textRw;
        Range rodataRw;
    };

    using RoWriteCallback = void (*)(const RoModule* module, ptr offsetIntoModule, const void* source, size writeSize);
    void setRoWriteCallback(RoWriteCallback callback);

} // namespace hk::ro
