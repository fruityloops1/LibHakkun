/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "hk/diag/diag.h"
#include "hk/types.h"

#include "vapours/util/util_parent_of_member.hpp"
#include <iterator>

#define NON_COPYABLE(cls)     \
    cls(const cls&) = delete; \
    cls& operator=(const cls&) = delete

#define NON_MOVEABLE(cls) \
    cls(cls&&) = delete;  \
    cls& operator=(cls&&) = delete

namespace ams::util {

    /* Forward declare implementation class for Node. */
    namespace impl {

        class IntrusiveListImpl;

    }

    class IntrusiveListNode {
        NON_COPYABLE(IntrusiveListNode);

    private:
        friend class impl::IntrusiveListImpl;

        IntrusiveListNode* m_prev;
        IntrusiveListNode* m_next;

    public:
        constexpr hk_alwaysinline IntrusiveListNode()
            : m_prev(this)
            , m_next(this) { /* ... */ }

        constexpr hk_alwaysinline bool IsLinked() const {
            return m_next != this;
        }

    private:
        constexpr hk_alwaysinline void LinkPrev(IntrusiveListNode* node) {
            /* We can't link an already linked node. */
            HK_ASSERT(!node->IsLinked());
            this->SplicePrev(node, node);
        }

        constexpr hk_alwaysinline void SplicePrev(IntrusiveListNode* first, IntrusiveListNode* last) {
            /* Splice a range into the list. */
            auto last_prev = last->m_prev;
            first->m_prev = m_prev;
            last_prev->m_next = this;
            m_prev->m_next = first;
            m_prev = last_prev;
        }

        constexpr hk_alwaysinline void LinkNext(IntrusiveListNode* node) {
            /* We can't link an already linked node. */
            HK_ASSERT(!node->IsLinked());
            return this->SpliceNext(node, node);
        }

        constexpr hk_alwaysinline void SpliceNext(IntrusiveListNode* first, IntrusiveListNode* last) {
            /* Splice a range into the list. */
            auto last_prev = last->m_prev;
            first->m_prev = this;
            last_prev->m_next = m_next;
            m_next->m_prev = last_prev;
            m_next = first;
        }

        constexpr hk_alwaysinline void Unlink() {
            this->Unlink(m_next);
        }

        constexpr hk_alwaysinline void Unlink(IntrusiveListNode* last) {
            /* Unlink a node from a next node. */
            auto last_prev = last->m_prev;
            m_prev->m_next = last;
            last->m_prev = m_prev;
            last_prev->m_next = this;
            m_prev = last_prev;
        }

        constexpr hk_alwaysinline IntrusiveListNode* GetPrev() {
            return m_prev;
        }

        constexpr const hk_alwaysinline IntrusiveListNode* GetPrev() const {
            return m_prev;
        }

        constexpr hk_alwaysinline IntrusiveListNode* GetNext() {
            return m_next;
        }

        constexpr const hk_alwaysinline IntrusiveListNode* GetNext() const {
            return m_next;
        }
    };
    /* DEPRECATED: static_assert(std::is_literal_type<IntrusiveListNode>::value); */

    namespace impl {

        class IntrusiveListImpl {
            NON_COPYABLE(IntrusiveListImpl);

        private:
            IntrusiveListNode m_root_node;

        public:
            template <bool Const>
            class Iterator;

            using value_type = IntrusiveListNode;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;
            using iterator = Iterator<false>;
            using const_iterator = Iterator<true>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            template <bool Const>
            class Iterator {
            public:
                using iterator_category = std::bidirectional_iterator_tag;
                using value_type = typename IntrusiveListImpl::value_type;
                using difference_type = typename IntrusiveListImpl::difference_type;
                using pointer = typename std::conditional<Const, IntrusiveListImpl::const_pointer, IntrusiveListImpl::pointer>::type;
                using reference = typename std::conditional<Const, IntrusiveListImpl::const_reference, IntrusiveListImpl::reference>::type;

            private:
                pointer m_node;

            public:
                constexpr hk_alwaysinline explicit Iterator(pointer n)
                    : m_node(n) { /* ... */ }

                constexpr hk_alwaysinline bool operator==(const Iterator& rhs) const {
                    return m_node == rhs.m_node;
                }

                constexpr hk_alwaysinline pointer operator->() const {
                    return m_node;
                }

                constexpr hk_alwaysinline reference operator*() const {
                    return *m_node;
                }

                constexpr hk_alwaysinline Iterator& operator++() {
                    m_node = m_node->m_next;
                    return *this;
                }

                constexpr hk_alwaysinline Iterator& operator--() {
                    m_node = m_node->m_prev;
                    return *this;
                }

                constexpr hk_alwaysinline Iterator operator++(int) {
                    const Iterator it { *this };
                    ++(*this);
                    return it;
                }

                constexpr hk_alwaysinline Iterator operator--(int) {
                    const Iterator it { *this };
                    --(*this);
                    return it;
                }

                constexpr hk_alwaysinline operator Iterator<true>() const {
                    return Iterator<true>(m_node);
                }

                constexpr hk_alwaysinline Iterator<false> GetNonConstIterator() const {
                    return Iterator<false>(const_cast<IntrusiveListImpl::pointer>(m_node));
                }
            };

        public:
            constexpr hk_alwaysinline IntrusiveListImpl()
                : m_root_node() { /* ... */ }

            /* Iterator accessors. */
            constexpr hk_alwaysinline iterator begin() {
                return iterator(m_root_node.GetNext());
            }

            constexpr hk_alwaysinline const_iterator begin() const {
                return const_iterator(m_root_node.GetNext());
            }

            constexpr hk_alwaysinline iterator end() {
                return iterator(std::addressof(m_root_node));
            }

            constexpr hk_alwaysinline const_iterator end() const {
                return const_iterator(std::addressof(m_root_node));
            }

            constexpr hk_alwaysinline iterator iterator_to(reference v) {
                /* Only allow iterator_to for values in lists. */
                HK_ASSERT(v.IsLinked());
                return iterator(std::addressof(v));
            }

            constexpr hk_alwaysinline const_iterator iterator_to(const_reference v) const {
                /* Only allow iterator_to for values in lists. */
                HK_ASSERT(v.IsLinked());
                return const_iterator(std::addressof(v));
            }

            /* Content management. */
            constexpr hk_alwaysinline bool empty() const {
                return !m_root_node.IsLinked();
            }

            constexpr hk_alwaysinline size_type size() const {
                return static_cast<size_type>(std::distance(this->begin(), this->end()));
            }

            constexpr hk_alwaysinline reference back() {
                return *m_root_node.GetPrev();
            }

            constexpr hk_alwaysinline const_reference back() const {
                return *m_root_node.GetPrev();
            }

            constexpr hk_alwaysinline reference front() {
                return *m_root_node.GetNext();
            }

            constexpr hk_alwaysinline const_reference front() const {
                return *m_root_node.GetNext();
            }

            constexpr hk_alwaysinline void push_back(reference node) {
                m_root_node.LinkPrev(std::addressof(node));
            }

            constexpr hk_alwaysinline void push_front(reference node) {
                m_root_node.LinkNext(std::addressof(node));
            }

            constexpr hk_alwaysinline void pop_back() {
                m_root_node.GetPrev()->Unlink();
            }

            constexpr hk_alwaysinline void pop_front() {
                m_root_node.GetNext()->Unlink();
            }

            constexpr hk_alwaysinline iterator insert(const_iterator pos, reference node) {
                pos.GetNonConstIterator()->LinkPrev(std::addressof(node));
                return iterator(std::addressof(node));
            }

            constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveListImpl& o) {
                splice_impl(pos, o.begin(), o.end());
            }

            constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveListImpl& o, const_iterator first) {
                const_iterator last(first);
                std::advance(last, 1);
                splice_impl(pos, first, last);
            }

            constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveListImpl& o, const_iterator first, const_iterator last) {
                splice_impl(pos, first, last);
            }

            constexpr hk_alwaysinline iterator erase(const_iterator pos) {
                if (pos == this->end()) {
                    return this->end();
                }
                iterator it(pos.GetNonConstIterator());
                (it++)->Unlink();
                return it;
            }

            constexpr hk_alwaysinline void clear() {
                while (!this->empty()) {
                    this->pop_front();
                }
            }

        private:
            constexpr hk_alwaysinline void splice_impl(const_iterator _pos, const_iterator _first, const_iterator _last) {
                if (_first == _last) {
                    return;
                }
                iterator pos(_pos.GetNonConstIterator());
                iterator first(_first.GetNonConstIterator());
                iterator last(_last.GetNonConstIterator());
                first->Unlink(std::addressof(*last));
                pos->SplicePrev(std::addressof(*first), std::addressof(*first));
            }
        };

    }

    template <class T, class Traits>
    class IntrusiveList {
        NON_COPYABLE(IntrusiveList);

    private:
        impl::IntrusiveListImpl m_impl;

    public:
        template <bool Const>
        class Iterator;

        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using reference = value_type&;
        using const_reference = const value_type&;
        using iterator = Iterator<false>;
        using const_iterator = Iterator<true>;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        template <bool Const>
        class Iterator {
        public:
            friend class ams::util::IntrusiveList<T, Traits>;

            using ImplIterator = typename std::conditional<Const, ams::util::impl::IntrusiveListImpl::const_iterator, ams::util::impl::IntrusiveListImpl::iterator>::type;

            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename IntrusiveList::value_type;
            using difference_type = typename IntrusiveList::difference_type;
            using pointer = typename std::conditional<Const, IntrusiveList::const_pointer, IntrusiveList::pointer>::type;
            using reference = typename std::conditional<Const, IntrusiveList::const_reference, IntrusiveList::reference>::type;

        private:
            ImplIterator m_iterator;

        private:
            constexpr explicit hk_alwaysinline Iterator(ImplIterator it)
                : m_iterator(it) { /* ... */ }

            constexpr hk_alwaysinline ImplIterator GetImplIterator() const {
                return m_iterator;
            }

        public:
            constexpr hk_alwaysinline bool operator==(const Iterator& rhs) const {
                return m_iterator == rhs.m_iterator;
            }

            constexpr hk_alwaysinline pointer operator->() const {
                return std::addressof(Traits::GetParent(*m_iterator));
            }

            constexpr hk_alwaysinline reference operator*() const {
                return Traits::GetParent(*m_iterator);
            }

            constexpr hk_alwaysinline Iterator& operator++() {
                ++m_iterator;
                return *this;
            }

            constexpr hk_alwaysinline Iterator& operator--() {
                --m_iterator;
                return *this;
            }

            constexpr hk_alwaysinline Iterator operator++(int) {
                const Iterator it { *this };
                ++m_iterator;
                return it;
            }

            constexpr hk_alwaysinline Iterator operator--(int) {
                const Iterator it { *this };
                --m_iterator;
                return it;
            }

            constexpr hk_alwaysinline operator Iterator<true>() const {
                return Iterator<true>(m_iterator);
            }
        };

    private:
        static constexpr hk_alwaysinline IntrusiveListNode& GetNode(reference ref) {
            return Traits::GetNode(ref);
        }

        static constexpr const hk_alwaysinline IntrusiveListNode& GetNode(const_reference ref) {
            return Traits::GetNode(ref);
        }

        static constexpr hk_alwaysinline reference GetParent(IntrusiveListNode& node) {
            return Traits::GetParent(node);
        }

        static constexpr hk_alwaysinline const_reference GetParent(const IntrusiveListNode& node) {
            return Traits::GetParent(node);
        }

    public:
        constexpr hk_alwaysinline IntrusiveList()
            : m_impl() { /* ... */ }

        /* Iterator accessors. */
        constexpr hk_alwaysinline iterator begin() {
            return iterator(m_impl.begin());
        }

        constexpr hk_alwaysinline const_iterator begin() const {
            return const_iterator(m_impl.begin());
        }

        constexpr hk_alwaysinline iterator end() {
            return iterator(m_impl.end());
        }

        constexpr hk_alwaysinline const_iterator end() const {
            return const_iterator(m_impl.end());
        }

        constexpr hk_alwaysinline const_iterator cbegin() const {
            return this->begin();
        }

        constexpr hk_alwaysinline const_iterator cend() const {
            return this->end();
        }

        constexpr hk_alwaysinline reverse_iterator rbegin() {
            return reverse_iterator(this->end());
        }

        constexpr hk_alwaysinline const_reverse_iterator rbegin() const {
            return const_reverse_iterator(this->end());
        }

        constexpr hk_alwaysinline reverse_iterator rend() {
            return reverse_iterator(this->begin());
        }

        constexpr hk_alwaysinline const_reverse_iterator rend() const {
            return const_reverse_iterator(this->begin());
        }

        constexpr hk_alwaysinline const_reverse_iterator crbegin() const {
            return this->rbegin();
        }

        constexpr hk_alwaysinline const_reverse_iterator crend() const {
            return this->rend();
        }

        constexpr hk_alwaysinline iterator iterator_to(reference v) {
            return iterator(m_impl.iterator_to(GetNode(v)));
        }

        constexpr hk_alwaysinline const_iterator iterator_to(const_reference v) const {
            return const_iterator(m_impl.iterator_to(GetNode(v)));
        }

        /* Content management. */
        constexpr hk_alwaysinline bool empty() const {
            return m_impl.empty();
        }

        constexpr hk_alwaysinline size_type size() const {
            return m_impl.size();
        }

        constexpr hk_alwaysinline reference back() {
            HK_ASSERT(!m_impl.empty());
            return GetParent(m_impl.back());
        }

        constexpr hk_alwaysinline const_reference back() const {
            HK_ASSERT(!m_impl.empty());
            return GetParent(m_impl.back());
        }

        constexpr hk_alwaysinline reference front() {
            HK_ASSERT(!m_impl.empty());
            return GetParent(m_impl.front());
        }

        constexpr hk_alwaysinline const_reference front() const {
            HK_ASSERT(!m_impl.empty());
            return GetParent(m_impl.front());
        }

        constexpr hk_alwaysinline void push_back(reference ref) {
            m_impl.push_back(GetNode(ref));
        }

        constexpr hk_alwaysinline void push_front(reference ref) {
            m_impl.push_front(GetNode(ref));
        }

        constexpr hk_alwaysinline void pop_back() {
            HK_ASSERT(!m_impl.empty());
            m_impl.pop_back();
        }

        constexpr hk_alwaysinline void pop_front() {
            HK_ASSERT(!m_impl.empty());
            m_impl.pop_front();
        }

        constexpr hk_alwaysinline iterator insert(const_iterator pos, reference ref) {
            return iterator(m_impl.insert(pos.GetImplIterator(), GetNode(ref)));
        }

        constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveList& o) {
            m_impl.splice(pos.GetImplIterator(), o.m_impl);
        }

        constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveList& o, const_iterator first) {
            m_impl.splice(pos.GetImplIterator(), o.m_impl, first.GetImplIterator());
        }

        constexpr hk_alwaysinline void splice(const_iterator pos, IntrusiveList& o, const_iterator first, const_iterator last) {
            m_impl.splice(pos.GetImplIterator(), o.m_impl, first.GetImplIterator(), last.GetImplIterator());
        }

        constexpr hk_alwaysinline iterator erase(const_iterator pos) {
            return iterator(m_impl.erase(pos.GetImplIterator()));
        }

        constexpr hk_alwaysinline void clear() {
            m_impl.clear();
        }
    };

    template <auto T, class Derived = util::impl::GetParentType<T>>
    class IntrusiveListMemberTraits;

    template <class Parent, IntrusiveListNode Parent::* Member, class Derived>
    class IntrusiveListMemberTraits<Member, Derived> {
    public:
        using ListType = IntrusiveList<Derived, IntrusiveListMemberTraits>;

    private:
        friend class IntrusiveList<Derived, IntrusiveListMemberTraits>;

        static constexpr hk_alwaysinline IntrusiveListNode& GetNode(Derived& parent) {
            return parent.*Member;
        }

        static constexpr const hk_alwaysinline IntrusiveListNode& GetNode(const Derived& parent) {
            return parent.*Member;
        }

        static hk_alwaysinline Derived& GetParent(IntrusiveListNode& node) {
            return util::GetParentReference<Member, Derived>(std::addressof(node));
        }

        static const hk_alwaysinline Derived& GetParent(const IntrusiveListNode& node) {
            return util::GetParentReference<Member, Derived>(std::addressof(node));
        }
    };

    template <auto T, class Derived = util::impl::GetParentType<T>>
    class IntrusiveListMemberTraitsByNonConstexprOffsetOf;

    template <class Parent, IntrusiveListNode Parent::* Member, class Derived>
    class IntrusiveListMemberTraitsByNonConstexprOffsetOf<Member, Derived> {
    public:
        using ListType = IntrusiveList<Derived, IntrusiveListMemberTraitsByNonConstexprOffsetOf>;

    private:
        friend class IntrusiveList<Derived, IntrusiveListMemberTraitsByNonConstexprOffsetOf>;

        static constexpr hk_alwaysinline IntrusiveListNode& GetNode(Derived& parent) {
            return parent.*Member;
        }

        static constexpr const hk_alwaysinline IntrusiveListNode& GetNode(const Derived& parent) {
            return parent.*Member;
        }

        static hk_alwaysinline Derived& GetParent(IntrusiveListNode& node) {
            return *reinterpret_cast<Derived*>(reinterpret_cast<char*>(std::addressof(node)) - GetOffset());
        }

        static const hk_alwaysinline Derived& GetParent(const IntrusiveListNode& node) {
            return *reinterpret_cast<const Derived*>(reinterpret_cast<const char*>(std::addressof(node)) - GetOffset());
        }

        static hk_alwaysinline uintptr_t GetOffset() {
            return reinterpret_cast<uintptr_t>(std::addressof(reinterpret_cast<Derived*>(0)->*Member));
        }
    };

    template <class Derived>
    class IntrusiveListBaseNode : public IntrusiveListNode { };

    template <class Derived>
    class IntrusiveListBaseTraits {
    public:
        using ListType = IntrusiveList<Derived, IntrusiveListBaseTraits>;

    private:
        friend class IntrusiveList<Derived, IntrusiveListBaseTraits>;

        static constexpr hk_alwaysinline IntrusiveListNode& GetNode(Derived& parent) {
            return static_cast<IntrusiveListNode&>(static_cast<IntrusiveListBaseNode<Derived>&>(parent));
        }

        static constexpr const hk_alwaysinline IntrusiveListNode& GetNode(const Derived& parent) {
            return static_cast<const IntrusiveListNode&>(static_cast<const IntrusiveListBaseNode<Derived>&>(parent));
        }

        static constexpr hk_alwaysinline Derived& GetParent(IntrusiveListNode& node) {
            return static_cast<Derived&>(static_cast<IntrusiveListBaseNode<Derived>&>(node));
        }

        static constexpr const hk_alwaysinline Derived& GetParent(const IntrusiveListNode& node) {
            return static_cast<const Derived&>(static_cast<const IntrusiveListBaseNode<Derived>&>(node));
        }
    };

}