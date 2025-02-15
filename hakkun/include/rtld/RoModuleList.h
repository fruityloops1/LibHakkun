#pragma once

#include "RoModule.h"

namespace nn::ro::detail {

    struct RoModuleList {
        RoModule* front;
        RoModule* back;

        class Iterator;

        Iterator begin() { return Iterator(this->back, false); }

        Iterator end() { return Iterator((RoModule*)this, false); }

        Iterator rbegin() { return Iterator(this->front, true); }

        Iterator rend() { return Iterator((RoModule*)this, true); }

        class Iterator {
        public:
            Iterator(RoModule* pModule, bool reverted)
                : m_pCurrentModule(pModule)
                , m_Reverted(reverted) { }

            Iterator& operator=(RoModule* pModule) {
                m_pCurrentModule = pModule;
                return *this;
            }

            Iterator& operator++() {
                if (m_pCurrentModule) {
                    m_pCurrentModule = m_Reverted ? m_pCurrentModule->next
                                                  : m_pCurrentModule->prev;
                }
                return *this;
            }

            bool operator!=(const Iterator& iterator) {
                return m_pCurrentModule != iterator.m_pCurrentModule;
            }

            RoModule* operator*() { return m_pCurrentModule; }

        private:
            RoModule* m_pCurrentModule;
            bool m_Reverted;
        };
    };

#ifdef __aarch64__
    static_assert(sizeof(RoModuleList) == 0x10, "RoModuleList isn't valid");
#elif __arm__
    static_assert(sizeof(RoModuleList) == 0x8, "RoModuleList isn't valid");
#endif

} // namespace nn::ro::detail
