#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"

namespace hk::util {

    /**
     * @brief Packed array of bits.
     *
     * @tparam Size
     */
    template <size Size>
    class BitArray {
        static constexpr size sSizeBytes = alignUp(Size, 8) / 8;

        u8 mStorage[sSizeBytes] { 0 };

        s32 getByteIdxByBitIdx(s32 bitIdx) const {
            return bitIdx / 8;
        }

    public:
        struct BitAccessor {
            BitAccessor(BitArray<Size>* arr, s32 idx)
                : mArray(arr)
                , mIndex(idx) { }

            operator bool() const { return mArray->get(mIndex); }
            void operator=(const bool& value) { mArray->set(mIndex, value); }

        private:
            BitArray<Size>* const mArray;
            const s32 mIndex = -1;
        };

        struct ConstBitAccessor {
            ConstBitAccessor(const BitArray<Size>* arr, s32 idx)
                : mArray(arr)
                , mIndex(idx) { }

            operator bool() const { return mArray->get(mIndex); }

        private:
            const BitArray<Size>* const mArray;
            const s32 mIndex = -1;
        };

        constexpr BitArray() = default;

        void set(s32 idx) {
            HK_ABORT_UNLESS(idx >= 0 && idx < Size, "BitArray: index out of bounds (%d/%d)", idx, Size);
            u8& byte = mStorage[getByteIdxByBitIdx(idx)];
            int bitIdx = idx % 8;

            byte |= (1 << bitIdx);
        }

        void unset(s32 idx) {
            HK_ABORT_UNLESS(idx >= 0 && idx < Size, "BitArray: index out of bounds (%d/%d)", idx, Size);
            u8& byte = mStorage[getByteIdxByBitIdx(idx)];
            int bitIdx = idx % 8;

            byte &= ~(1 << bitIdx);
        }

        bool get(s32 idx) const {
            HK_ABORT_UNLESS(idx >= 0 && idx < Size, "BitArray: index out of bounds (%d/%d)", idx, Size);
            const u8& byte = mStorage[getByteIdxByBitIdx(idx)];
            int bitIdx = idx % 8;

            return byte & (1 << bitIdx);
        }

        void set(s32 idx, bool value) {
            value ? set(idx) : unset(idx);
        }

        BitAccessor operator[](s32 idx) {
            return { this, idx };
        }

        ConstBitAccessor operator[](s32 idx) const {
            return { this, idx };
        }
    };

} // namespace hk::util
