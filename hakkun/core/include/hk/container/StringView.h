#pragma once

#include "hk/container/Span.h"
#include "hk/util/hash.h"
#include <string>

namespace hk {

    namespace detail {

        template <typename T>
        class StringViewBase : public Span<T> {
            static StringViewBase getEmpty() { return { &cNullChar }; }

            using Super = Span<T>;

            using CharTraits = std::char_traits<std::remove_const_t<T>>;

        public:
            using Super::Super;

            using Super::data;
            using Super::length;

            constexpr StringViewBase()
                : StringViewBase(getEmpty()) { }

            constexpr StringViewBase(T* str)
                : StringViewBase(str, str != nullptr ? CharTraits::length(str) : 0) {
            }

            StringViewBase(const Span<const u8>& data)
                : StringViewBase(cast<T>(data)) { }

            constexpr bool operator==(StringViewBase other) const {
                return this->length() == other.length() && __builtin_memcmp(this->data(), other.data(), length()) == 0;
            }

            constexpr bool startsWith(StringViewBase start) const {
                return this->length() >= start.length() && __builtin_memcmp(this->data(), start.data(), start.length()) == 0;
            }

            constexpr StringViewBase operator+(size offs) const {
                if (offs >= length())
                    return StringViewBase();

                return { data() + offs, length() - offs };
            }

            constexpr ::size findIndex(StringViewBase needle) const {
                if (needle.length() > this->length())
                    return -1;

                for (size curIdx = 0; curIdx < this->length() - needle.length(); curIdx++) {
                    if ((*this + curIdx).startsWith(needle))
                        return curIdx;
                }

                return -1;
            }

            static constexpr const T cNullChar = 0;
        };

    } // namespace detail

    template <typename T>
    using StringViewBase = detail::StringViewBase<const T>;

    using StringView = StringViewBase<char>;
    using WideStringView = StringViewBase<wchar_t>;
    using StringView16 = StringViewBase<char16_t>;

    template <typename T>
    class MutableStringViewBase : public detail::StringViewBase<T> {
        size mCapacity = 0; // includes null terminator

        using StringView = StringViewBase<T>;

        using Super = detail::StringViewBase<T>;
        using Super::set;

    protected:
        using Super::cNullChar;
        using Super::getData;
        using Super::getSize;
        using Super::setSize;

        constexpr void ensureNullTerminated() {
            setSize(util::min(getSize(), mCapacity - 1));

            getData()[getSize()] = cNullChar;
        }

    public:
        using Super::data;

        constexpr MutableStringViewBase() = default;

        constexpr MutableStringViewBase(Span<T> buffer)
            : detail::StringViewBase<T>(buffer.data())
            , mCapacity(buffer.size()) { }

        constexpr MutableStringViewBase(Span<T> buffer, size length)
            : detail::StringViewBase<T>(buffer.data(), length)
            , mCapacity(buffer.size()) { }

        constexpr void truncate(size newSize) {
            HK_ABORT_UNLESS(newSize <= mCapacity, "hk::MutableStringViewBase<%s>::truncate(%zu): new size exceeds capacity (capacity: %zu)", util::getTypeName<T>, newSize, mCapacity);
            setSize(newSize);
            ensureNullTerminated();
        }

        constexpr bool append(StringView other) {
            size desiredSize = getSize() + other.size();
            size newSize = util::min(desiredSize, mCapacity - 1);
            size left = mCapacity - getSize() - 1;

            util::copy(getData() + getSize(), other.data(), util::min(other.length(), left));
            setSize(newSize);
            ensureNullTerminated();
            return desiredSize == newSize;
        }

        constexpr bool append(T value) {
            if (getSize() >= mCapacity - 1)
                return false;

            getData()[getSize()] = value;
            setSize(getSize() + 1);
            ensureNullTerminated();
            return true;
        }

        constexpr MutableStringViewBase& operator+=(const StringView other) {
            append(other);
            return *this;
        }

        constexpr operator T*() { return data(); }
        constexpr operator const T*() const { return data(); }

        const T* cstr() const { return data(); }
    };

    using MutableStringView = MutableStringViewBase<char>;
    using MutableWideStringView = MutableStringViewBase<wchar_t>;
    using MutableStringView16 = MutableStringViewBase<char16_t>;

    namespace util {

        constexpr u32 hashMurmur(StringView str, u32 seed = 0) {
            return detail::hashMurmurImpl<char, detail::ReadDefault<char>>(str.data(), str.length(), seed);
        }

        constexpr u64 hashMurmur64(StringView str) {
            return detail::hashMurmur64Impl<char, detail::ReadDefault<char>>(str.data(), str.length());
        }

        template <typename T>
            requires(std::is_convertible_v<T, Span<const char>> or std::is_convertible_v<T, StringView>)
        struct MurmurHash3<T> {
            static size hash(StringView str) {
                if constexpr (is64Bit())
                    return hashMurmur64(str);
                else
                    return hashMurmur(str);
            }
        };

    } // namespace util

} // namespace hk
