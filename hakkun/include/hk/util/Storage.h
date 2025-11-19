#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"
#include <new> // IWYU pragma: keep
#include <utility>

namespace hk::util {

    /**
     * @brief Object of which the lifetime is manually managed by the user of the class
     *
     * @tparam T Object
     */
    template <typename T>
    class Storage {
        alignas(alignof(T)) u8 mStorage[sizeof(T)] { 0 };
        bool mAlive = false;

        void destroyImpl() {
            getUnsafe()->~T();
            mAlive = false;
        }

        template <typename... Args>
        void createImpl(Args&&... args) {
            new (getUnsafe()) T(std::forward<Args>(args)...);
            mAlive = true;
        }

    public:
        T* getUnsafe() { return cast<T*>(mStorage); }

        bool isAlive() const { return mAlive; }

        bool tryDestroy() {
            if (!mAlive)
                return false;
            destroy();
            return true;
        }

        void destroy() {
            HK_ASSERT(mAlive);
            destroyImpl();
        }

        template <typename... Args>
        bool tryCreate(Args&&... args) {
            if (mAlive)
                return false;
            createImpl(std::forward<Args>(args)...);
            return true;
        }

        template <typename... Args>
        void create(Args&&... args) {
            HK_ASSERT(!mAlive);
            createImpl(std::forward<Args>(args)...);
        }

        T take() {
            HK_ASSERT(mAlive);
            T value = move(*getUnsafe());
            destroyImpl();
            return move(value);
        }

        T* get() {
            HK_ASSERT(mAlive);
            return getUnsafe();
        }

        T* tryGet() {
            if (!mAlive)
                return nullptr;
            return getUnsafe();
        }
    };

} // namespace hk::util
