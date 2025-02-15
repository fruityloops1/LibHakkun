#pragma once

#include "hk/types.h"
#include <new>

namespace hk::util {

    template <typename T>
    class Storage {
        u8 mStorage[sizeof(T)] { 0 };
        bool mAlive = false;

        T* getUnsafe() { return cast<T*>(mStorage); }

        void destroyImpl() {
            getUnsafe()->~T();
            mAlive = false;
        }

        template <typename... Args>
        void createImpl(Args... args) {
            new (getUnsafe()) T(args...);
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
        bool tryCreate(Args... args) {
            if (mAlive)
                return false;
            createImpl(args...);
            return true;
        }

        template <typename... Args>
        void create(Args... args) {
            // assert(!mAlive);
            createImpl(args...);
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
