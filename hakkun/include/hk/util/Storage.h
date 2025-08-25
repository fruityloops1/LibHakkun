#pragma once

#include "hk/types.h"
#include <new>
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

        T* getUnsafe() { return cast<T*>(mStorage); }

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
        bool isAlive() const { return mAlive; }

        bool tryDestroy() {
            if (!mAlive)
                return false;
            destroy();
            return true;
        }

        void destroy() {
            // assert(mAlive)
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
            // assert(!mAlive);
            createImpl(std::forward<Args>(args)...);
        }

        T* get() {
            // assert(mAlive);
            return getUnsafe();
        }

        T* tryGet() {
            if (!mAlive)
                return nullptr;
            return getUnsafe();
        }
    };

} // namespace hk::util
