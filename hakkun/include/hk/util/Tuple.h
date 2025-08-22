#pragma once

namespace hk {

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

} // namespace hk
