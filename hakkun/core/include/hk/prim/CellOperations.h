#pragma once

#include "hk/prim/traits/Type.h"
#include "hk/types.h"
#include "hk/util/Algorithm.h"
#include "hk/util/Allocator.h"
#include "hk/util/TypeName.h"
#include <memory>

namespace hk {

    namespace detail {

        template <typename T, typename Storage>
            requires(!util::ctIsConst<T> and util::ctIsDestructible<T>)
        struct CellOperationsBase : protected Storage {
            using Type = T;

            constexpr const T& operator*() const {
                checkAlive();
                return *getStorageConst();
            }

            constexpr const T* operator->() const {
                checkAlive();
                return getStorageConst();
            }

            constexpr T& operator*() {
                checkAlive();
                return *getStorage();
            }

            constexpr T* operator->() {
                checkAlive();
                return getStorage();
            }

            constexpr operator const T*() const { return isAlive() ? getStorageConst() : nullptr; }
            constexpr operator T*() { return isAlive() ? getStorage() : nullptr; }
            constexpr const T* data() const { return isAlive() ? getStorageConst() : nullptr; }
            constexpr T* data() { return isAlive() ? getStorage() : nullptr; }

            constexpr operator const T&() const {
                checkAlive();
                return *getStorageConst();
            }

            constexpr operator T&() {
                checkAlive();
                return *getStorage();
            }

            constexpr const T& get() const {
                checkAlive();
                return *getStorageConst();
            }

            constexpr T& get() {
                checkAlive();
                return *getStorage();
            }

            template <typename... Args>
            constexpr void construct(Args&&... args) {
                if constexpr (isAliveMutable())
                    destroy();

                construct_at(getStorage(), forward<Args>(args)...);
                if constexpr (isAliveMutable())
                    setAlive(true);
            }

            constexpr bool destroy() {
                if (!isAlive())
                    return false;

                util::destroy(getStorage());
                if constexpr (isAliveMutable())
                    setAlive(false);
                return true;
            }

            constexpr operator bool() const { return isAlive(); }
            constexpr bool hasValue() const { return isAlive(); }

        protected:
            using Storage::getStorage;
            using Storage::getStorageConst;
            using Storage::isAlive;
            using Storage::isAliveMutable;
            using Storage::setAlive;

            constexpr void checkAlive() const {
                HK_ABORT_UNLESS(isAlive(), "%s<%s>: No value", util::getTypeName<Storage>(), util::getTypeName<T>());
            }
        };

        enum class CellConstructDefault { };
        enum class CellConstructEmpty { };

        struct CellConstructTypeBase { };
        template <typename T>
        struct CellConstructType : CellConstructTypeBase { };

    } // namespace detail

    constexpr detail::CellConstructDefault CellDefault = detail::CellConstructDefault();
    constexpr detail::CellConstructEmpty CellEmpty = detail::CellConstructEmpty();
    template <typename T>
    constexpr detail::CellConstructType CellType = detail::CellConstructType<T>();

    template <typename T, typename Storage>
    struct CellOperations : detail::CellOperationsBase<T, Storage> {
        using Super = detail::CellOperationsBase<T, Storage>;

        constexpr CellOperations()
            : CellOperations(CellEmpty) { }

        constexpr CellOperations(detail::CellConstructEmpty) { setAlive(false); }

        using Super::construct;
        template <typename = void>
            requires util::ctIsDefaultConstructible<T>
        constexpr CellOperations(detail::CellConstructDefault) { construct(T()); }

        template <typename... Args>
            requires util::ctIsConstructible<Args...>
        constexpr CellOperations(Args&&... args) { construct(forward<Args>(args)...); }

        template <typename = void>
            requires util::ctIsCopyConstructible<T>
        constexpr CellOperations(const CellOperations& other) {
            construct(*other.getStorage());
        }

        template <typename = void>
            requires util::ctIsCopyConstructible<T>
        CellOperations& operator=(const CellOperations& other) {
            construct(*other.getStorage());
            return *this;
        }

        template <typename = void>
            requires util::ctIsMoveConstructible<T>
        constexpr CellOperations(CellOperations&& other) {
            other.setAlive(false);
            construct(forward<T>(*other.getStorage()));
            util::destroy(other.getStorage());
        }

        template <typename = void>
            requires util::ctIsMoveConstructible<T>
        CellOperations& operator=(CellOperations&& other) {
            other.setAlive(false);
            construct(forward<T>(*other.getStorage()));
            util::destroy(other.getStorage());
            return *this;
        }

        using Super::destroy;
        constexpr ~CellOperations() { destroy(); }

    protected:
        using Super::getStorage;
        using Super::isAlive;
        using Super::setAlive;
    };

    template <typename T, typename Storage, typename Allocator = util::DefaultAllocator>
    struct CellOperationsOnHeap : detail::CellOperationsBase<T, Storage> {
        using Super = detail::CellOperationsBase<T, Storage>;

        constexpr CellOperationsOnHeap()
            : CellOperationsOnHeap(CellEmpty) { }
        constexpr CellOperationsOnHeap(nullptr_t)
            : CellOperationsOnHeap(CellEmpty) { }

        constexpr CellOperationsOnHeap(detail::CellConstructEmpty) { setStorage(nullptr); }

        template <typename = void>
            requires util::ctIsDefaultConstructible<T>
        constexpr CellOperationsOnHeap(detail::CellConstructDefault) {
            setStorage(allocate());
            Super::construct(T());
        }

        template <typename Derived, typename... Args>
            requires(util::ctIsBaseOf<T, Derived> and util::ctIsConstructible<Derived, Args...>)
        constexpr CellOperationsOnHeap(detail::CellConstructType<Derived>, Args&&... args) {
            Derived* instance = allocateDerived<Derived>();
            construct_at(instance, forward<Args>(args)...);
            setStorage(instance);
        }

        template <typename... Args>
            requires(util::ctIsConstructible<T, Args...> and !util::ctContainsDerived<detail::CellConstructTypeBase, Args...>)
        constexpr CellOperationsOnHeap(Args&&... args) {
            setStorage(allocate());
            Super::construct(forward<Args>(args)...);
        }

        NON_COPYABLE(CellOperationsOnHeap);

        template <typename = void>
        constexpr CellOperationsOnHeap(CellOperationsOnHeap&& other) {
            setStorage(other.getStorage());
            other.setStorage(nullptr);
        }

        template <typename = void>
        CellOperationsOnHeap& operator=(CellOperationsOnHeap&& other) {
            destroy();

            setStorage(other.getStorage());
            other.setStorage(nullptr);
            return *this;
        }

        template <typename... Args>
        constexpr void construct(Args&&... args) {
            if (isAlive())
                util::destroy(getStorage());
            else
                setStorage(allocate());

            construct_at(getStorage(), forward<Args>(args)...);
        }

        constexpr void destroy() {
            if (isAlive()) {
                Super::destroy();
                free(getStorage());
                setStorage(nullptr);
            }
        }

        constexpr ~CellOperationsOnHeap() { destroy(); }

    protected:
        using Storage::setStorage;
        using Super::getStorage;
        using Super::isAlive;
        using Super::setAlive;

    private:
        using Super::construct;
        using Super::destroy;

        constexpr static T* allocate() {
            if consteval {
                return std::allocator<T>().allocate(1);
            } else {
                return cast<T*>(Allocator::allocate(sizeof(T), alignof(T)));
            }
        }

        template <typename Derived>
            requires util::ctIsBaseOf<T, Derived>
        constexpr static Derived* allocateDerived() {
            if consteval {
                return std::allocator<Derived>().allocate(1);
            } else {
                return cast<Derived*>(Allocator::allocate(sizeof(Derived), alignof(Derived)));
            }
        }

        constexpr static void free(T* ptr) {
            if consteval {
                std::allocator<T>().deallocate(ptr, 1);
            } else {
                Allocator::free(ptr);
            }
        }
    };

} // namespace hk
