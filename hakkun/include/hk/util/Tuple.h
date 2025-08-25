#pragma once

namespace hk {

    /**
     * @brief Object to hold multiple values of different types.
     *
     * @tparam Types
     */
    template <typename... Types>
    struct Tuple;

    template <typename A>
    struct Tuple<A> {
        A a;
    };

    template <typename A, typename B>
    struct Tuple<A, B> {
        A a;
        B b;
    };

    template <typename A, typename B, typename C>
    struct Tuple<A, B, C> {
        A a;
        B b;
        C c;
    };

    template <typename A, typename B, typename C, typename D>
    struct Tuple<A, B, C, D> {
        A a;
        B b;
        C c;
        D d;
    };

    template <typename A, typename B, typename C, typename D, typename E>
    struct Tuple<A, B, C, D, E> {
        A a;
        B b;
        C c;
        D d;
        E e;
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F>
    struct Tuple<A, B, C, D, E, F> {
        A a;
        B b;
        C c;
        D d;
        E e;
        F f;
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
    struct Tuple<A, B, C, D, E, F, G> {
        A a;
        B b;
        C c;
        D d;
        E e;
        F f;
        G g;
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
    struct Tuple<A, B, C, D, E, F, G, H> {
        A a;
        B b;
        C c;
        D d;
        E e;
        F f;
        G g;
        H h;
    };

    template <typename... Types>
    struct OutTuple;

    template <typename A>
    struct OutTuple<A&> {
        A& a;

        constexpr OutTuple& operator=(const Tuple<A>&& v) {
            a = v.a;
            return *this;
        }
    };

    template <typename A, typename B>
    struct OutTuple<A&, B&> {
        A& a;
        B& b;

        constexpr OutTuple& operator=(const Tuple<A, B>&& v) {
            a = v.a;
            b = v.b;
            return *this;
        }
    };

    template <typename A, typename B, typename C>
    struct OutTuple<A&, B&, C&> {
        A& a;
        B& b;
        C& c;

        constexpr OutTuple& operator=(const Tuple<A, B, C>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            return *this;
        }
    };

    template <typename A, typename B, typename C, typename D>
    struct OutTuple<A&, B&, C&, D&> {
        A& a;
        B& b;
        C& c;
        D& d;

        constexpr OutTuple& operator=(const Tuple<A, B, C, D>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            d = v.d;
            return *this;
        }
    };

    template <typename A, typename B, typename C, typename D, typename E>
    struct OutTuple<A&, B&, C&, D&, E&> {
        A& a;
        B& b;
        C& c;
        D& d;
        E& e;

        constexpr OutTuple& operator=(const Tuple<A, B, C, D, E>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            d = v.d;
            e = v.e;
            return *this;
        }
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F>
    struct OutTuple<A&, B&, C&, D&, E&, F&> {
        A& a;
        B& b;
        C& c;
        D& d;
        E& e;
        F& f;

        constexpr OutTuple& operator=(const Tuple<A, B, C, D, E, F>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            d = v.d;
            e = v.e;
            f = v.f;
            return *this;
        }
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G>
    struct OutTuple<A&, B&, C&, D&, E&, F&, G&> {
        A& a;
        B& b;
        C& c;
        D& d;
        E& e;
        F& f;
        G& g;

        constexpr OutTuple& operator=(const Tuple<A, B, C, D, E, F, G>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            d = v.d;
            e = v.e;
            f = v.f;
            g = v.g;
            return *this;
        }
    };

    template <typename A, typename B, typename C, typename D, typename E, typename F, typename G, typename H>
    struct OutTuple<A&, B&, C&, D&, E&, F&, G&, H&> {
        A& a;
        B& b;
        C& c;
        D& d;
        E& e;
        F& f;
        G& g;
        H& h;

        constexpr OutTuple& operator=(const Tuple<A, B, C, D, E, F, G, H>&& v) {
            a = v.a;
            b = v.b;
            c = v.c;
            d = v.d;
            e = v.e;
            f = v.f;
            g = v.g;
            h = v.h;
            return *this;
        }
    };

} // namespace hk

/**
 * @brief Ties out variables to assignment for a Tuple
 *
 *  Example:
 *  ```cpp
 *  int a, b;
 *  tie(a, b) = Tuple<int, int>(4, 8);
 *  ```
 *
 * @tparam Args
 * @param args Out variables
 * @return hk::OutTuple<Args...>
 */
template <typename... Args>
constexpr hk::OutTuple<Args...> tie(Args&&... args) {
    return { args... };
}
