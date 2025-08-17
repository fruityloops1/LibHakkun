#pragma once

#include "hk/types.h"
#include "hk/util/hash.h"
#include "rtld/RoModule.h"

namespace hk::sail::detail {
    struct VersionLoader;
} // namespace hk::sail::detail

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

        const u8* mBuildId = nullptr;

#ifndef HK_DISABLE_SAIL
        int mVersionIndex = -1;
        char mVersionName[9] { '\0' };
        u32 mVersionNameHash = 0;
#endif

    public:
        nn::ro::detail::RoModule* getNnModule() const { return mModule; }

        Range range() const { return { mTextRange.start(), mTextRange.size() + mRodataRange.size() + mDataRange.size() }; }
        Range text() const { return mTextRange; }
        Range rodata() const { return mRodataRange; }
        Range data() const { return mDataRange; }

        const u8* getBuildId() const { return mBuildId; }

#ifndef HK_DISABLE_SAIL
        int getVersionIndex() const { return mVersionIndex; }
        const char* getVersionName() const { return mVersionIndex != -1 ? mVersionName : nullptr; }
        hk_alwaysinline bool isVersion(const char* versionName) const {
            return util::hashMurmur(versionName) == mVersionNameHash;
        }
#endif

        Result findRanges();
        Result mapRw();
        Result findBuildId();

        Result writeRo(ptr offset, const void* source, size writeSize) const;

        template <typename T>
        Result writeRo(ptr offset, const T& value) const {
            return writeRo(offset, &value, sizeof(T));
        }

        enum RodataAttribute
        {
            RodataAttribute_ModuleName,
            RodataAttribute_Unknown,

            RodataAttribute_Max,
        };

        struct ModuleNameRodataAttribute {
            u32 nameLength;
            char name[];
        };

        struct UnknownRodataAttribute {
            u32 _0;
            u32 _4;
        };

        template <typename T>
        const T* findRodataAttribute(RodataAttribute type) const {
            ptr iter = mRodataRange.start();
            RodataAttribute current = RodataAttribute(*cast<const u32*>(iter));

            if (*cast<const u32*>(iter) >= RodataAttribute_Max) // Probably not attribute array
                return nullptr;

            while (true) {
                iter += sizeof(RodataAttribute);

                if (current == type)
                    return cast<const T*>(iter);

                if (current == RodataAttribute_ModuleName) // end
                    break;

                switch (current) {
                /*case RodataAttribute_ModuleName: {
                    const ModuleNameRodataAttribute* name = cast<const ModuleNameRodataAttribute*>(iter);
                    iter += sizeof(*name) + name->nameLength;
                    break;
                }*/
                case RodataAttribute_Unknown:
                    iter += sizeof(UnknownRodataAttribute);
                    break;
                default:
                    *(ptr*)iter = ptr(mModule);
                    HK_ABORT("Unknown RodataAttribute: %d %p", current, mModule);
                }

                current = RodataAttribute(*cast<const u32*>(iter));
            }

            return nullptr;
        }

        const char* getModuleName() const {
            auto* name = findRodataAttribute<ModuleNameRodataAttribute>(RodataAttribute_ModuleName);

            return name != nullptr ? name->name : nullptr;
        }

        friend class RoUtil;
        friend class sail::detail::VersionLoader;
    };

    using RoWriteCallback = void (*)(const RoModule* module, ptr offsetIntoModule, const void* source, size writeSize);
    void setRoWriteCallback(RoWriteCallback callback);

} // namespace hk::ro
