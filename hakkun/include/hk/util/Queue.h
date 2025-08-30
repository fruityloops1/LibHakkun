#pragma once

#include "hk/os/Event.h"
#include "hk/os/Mutex.h"
#include "hk/util/Allocator.h"

namespace hk::util {

    template <typename T, AllocatorType Allocator = MallocAllocator>
    class Queue : protected util::Allocator<Allocator> {
        struct Node {
            Node(const T& value)
                : value(value) {
            }
            Node(T&& value)
                : value(std::forward<T>(value)) {
            }

            T value;
            Node* up = nullptr;
        };

        os::Event mSignal;
        os::Mutex mMutex;
        Node* mBottom = nullptr;
        Node* mTop = nullptr;

    public:
        ~Queue() {
            auto lock = mMutex.lockScoped();

            while (mBottom != nullptr) {
                Node* oldNode = mBottom;
                mBottom = mBottom->up;
                delete oldNode;
            }
        }

        void add(const T& value) {
            Node* node = new Node(value);

            {
                auto lock = mMutex.lockScoped();

                if (mBottom == nullptr) {
                    mBottom = mTop = node;
                } else {
                    mTop->up = node;
                    mTop = node;
                }
            }

            mSignal.signal();
        }

        void add(T&& value) {
            Node* node = new Node(std::forward<T>(value));

            {
                auto lock = mMutex.lockScoped();

                if (mBottom == nullptr) {
                    mBottom = mTop = node;
                } else {
                    mTop->up = node;
                    mTop = node;
                }
            }

            mSignal.signal();
        }

        T take() {
            while (mBottom == nullptr)
                HK_ABORT_UNLESS_R(mSignal.wait());

            auto lock = mMutex.lockScoped();

            T value = T(std::move(mBottom->value));

            Node* oldNode = mBottom;
            mBottom = mBottom->up;
            delete oldNode;

            if (mBottom == nullptr)
                mTop = nullptr;

            return std::move(value);
        }

        bool empty() const { return !(mBottom or mTop); }
    };

} // namespace hk::util
