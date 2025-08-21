#pragma once

#include "RoModule.h"

namespace nn::ro::detail {

    // TODO: Replace with proper IntrusiveList implementation
    struct RoModuleList {
        RoModule* front;
        RoModule* back;

        RoModuleList()
            : front((RoModule*)this)
            , back((RoModule*)this) {
        }

        class Iterator;

        Iterator begin() { return Iterator(this->back, false); }

        Iterator end() { return Iterator((RoModule*)this, false); }

        Iterator rbegin() { return Iterator(this->front, true); }

        Iterator rend() { return Iterator((RoModule*)this, true); }

        class Iterator {
        public:
            Iterator(RoModule* module, bool reverted)
                : mCurrentModule(module)
                , mReverted(reverted) { }

            Iterator& operator=(RoModule* module) {
                mCurrentModule = module;
                return *this;
            }

            Iterator& operator++() {
                if (mCurrentModule) {
                    mCurrentModule = mReverted ? mCurrentModule->next
                                               : mCurrentModule->prev;
                }
                return *this;
            }

            bool operator!=(const Iterator& iterator) {
                return mCurrentModule != iterator.mCurrentModule;
            }

            RoModule* operator*() { return mCurrentModule; }

        private:
            RoModule* mCurrentModule;
            bool mReverted;
        };

        bool empty() const {
            return front == (RoModule*)this;
        }

        void pushFront(RoModule* module) {
            if (!module)
                return;

            module->prev = (RoModule*)this;
            module->next = front;

            front->prev = module;

            front = module;
        }

        void pushBack(RoModule* module) {
            if (!module)
                return;

            module->next = (RoModule*)this;
            module->prev = back;

            back->next = module;

            back = module;
        }

        void popFront() {
            if (empty())
                return;

            RoModule* oldFront = front;
            front = front->next;
            front->prev = (RoModule*)this;

            oldFront->prev = oldFront;
            oldFront->next = oldFront;
        }

        void popBack() {
            if (empty())
                return;

            RoModule* oldBack = back;
            back = back->prev;
            back->next = (RoModule*)this;

            oldBack->prev = oldBack;
            oldBack->next = oldBack;
        }

        void insertBefore(RoModule* pos, RoModule* module) {
            if (!pos || !module)
                return;

            module->next = pos;
            module->prev = pos->prev;

            pos->prev->next = module;
            pos->prev = module;

            if (pos == front)
                front = module;
        }

        void insertAfter(RoModule* pos, RoModule* module) {
            if (!pos || !module)
                return;

            module->prev = pos;
            module->next = pos->next;

            pos->next->prev = module;
            pos->next = module;

            if (pos == back)
                back = module;
        }

        void remove(RoModule* module) {
            if (!module || module == (RoModule*)this)
                return;

            if (module == front)
                front = (module->next == (RoModule*)this) ? (RoModule*)this : module->next;

            if (module == back)
                back = (module->prev == (RoModule*)this) ? (RoModule*)this : module->prev;

            module->prev->next = module->next;
            module->next->prev = module->prev;

            module->prev = module;
            module->next = module;
        }

        void clear() {
            while (!empty())
                popFront();
        }
    };

#ifdef __aarch64__
    static_assert(sizeof(RoModuleList) == 0x10, "RoModuleList isn't valid");
#elif __arm__
    static_assert(sizeof(RoModuleList) == 0x8, "RoModuleList isn't valid");
#endif

} // namespace nn::ro::detail
