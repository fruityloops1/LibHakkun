#pragma once

#include "hk/prim/traits/Type.h"
#include "hk/util/Algorithm.h"
#include "hk/util/TypeName.h"

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

            constexpr operator const T*() const { return isAlive() ? getStorage() : nullptr; }
            constexpr operator T*() { return isAlive() ? getStorage() : nullptr; }

            constexpr operator const T&() const {
                checkAlive();
                return *getStorage();
            }

            constexpr operator T&() {
                checkAlive();
                return *getStorage();
            }

            template <typename... Args>
            constexpr void construct(Args&&... args) {
                if (isAlive())
                    destroy();

                construct_at(getStorage(), forward<Args>(args)...);
                setAlive(true);
            }

            constexpr bool destroy() {
                if (!isAlive())
                    return false;

                util::destroy(getStorage());
                setAlive(false);
                return true;
            }

            constexpr operator bool() const { return isAlive(); }
            constexpr bool hasValue() const { return isAlive(); }

        protected:
            using Storage::getStorage;
            using Storage::getStorageConst;
            using Storage::isAlive;
            using Storage::setAlive;

            constexpr void checkAlive() const {
                HK_ABORT_UNLESS(isAlive(), "%s<%s>: No value", util::getTypeName<Storage>(), util::getTypeName<T>());
            }
        };

        enum class CellConstructDefault { };
        enum class CellConstructEmpty { };

    } // namespace detail

    constexpr detail::CellConstructDefault CellDefault = detail::CellConstructDefault();
    constexpr detail::CellConstructEmpty CellEmpty = detail::CellConstructEmpty();

    template <typename T, typename Storage>
    struct CellOperations : detail::CellOperationsBase<T, Storage> {
        using Super = detail::CellOperationsBase<T, Storage>;

        constexpr CellOperations()
            : CellOperations(CellEmpty) { }

        constexpr CellOperations(detail::CellConstructEmpty) { setAlive(false); }

        using Super::construct;
        template <typename = void>
            requires util::ctIsTriviallyDefaultConstructible<T>
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

} // namespace hk
