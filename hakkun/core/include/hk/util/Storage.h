#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include <new> // IWYU pragma: keep
#include <utility>

namespace hk::util {

    /**
     * @brief Object the lifetime of which is manually managed by the user of the class
     *
     * @tparam T Object
     */
    template <typename T>
    class Storage {
        union {
            T mInstance;
        };
        bool mAlive = false;

        constexpr void destroyImpl() {
            mInstance.~T();
            mAlive = false;
        }

        template <typename... Args>
        constexpr void createImpl(Args&&... args) {
            construct_at(&mInstance, forward<Args>(args)...);
            mAlive = true;
        }

    public:
        constexpr Storage() { }

        constexpr ~Storage() {
            tryDestroy();
        }

        constexpr T* getUnsafe() { return &mInstance; }
        constexpr const T* getUnsafe() const { return &mInstance; }

        constexpr bool isAlive() const { return mAlive; }

        constexpr bool tryDestroy() {
            if (!mAlive)
                return false;
            destroy();
            return true;
        }

        constexpr void destroy() {
            HK_ASSERT(mAlive);
            destroyImpl();
        }

        template <typename... Args>
        constexpr bool tryCreate(Args&&... args) {
            if (mAlive)
                return false;
            createImpl(std::forward<Args>(args)...);
            return true;
        }

        template <typename... Args>
        constexpr void create(Args&&... args) {
            HK_ASSERT(!mAlive);
            createImpl(std::forward<Args>(args)...);
        }

        constexpr hk::ValueOrResult<T> take() {
            HK_UNLESS(mAlive, hk::ResultNoValue());
            T value = move(mInstance);
            destroyImpl();
            return move(value);
        }

        constexpr T* get() {
            HK_ASSERT(mAlive);
            return &mInstance;
        }

        constexpr const T* get() const {
            HK_ASSERT(mAlive);
            return &mInstance;
        }

        constexpr T* tryGet() {
            return mAlive ? &mInstance : nullptr;
        }

        constexpr const T* tryGet() const {
            return mAlive ? &mInstance : nullptr;
        }
    };

} // namespace hk::util
