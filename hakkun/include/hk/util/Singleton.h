#pragma once

#include "hk/util/Storage.h"

/**
 * @brief Declaration of Singleton for a class.
 *
 */
#define HK_SINGLETON(CLASS)                                                                               \
    static ::hk::util::Storage<CLASS> sSingletonStorage;                                                  \
                                                                                                          \
public:                                                                                                   \
    static CLASS* instance() { return sSingletonStorage.tryGet(); }                                       \
    template <typename... Args>                                                                           \
    static void createInstance(Args&&... args) { sSingletonStorage.create(std::forward<Args>(args)...); } \
    static void deleteInstance() { sSingletonStorage.destroy(); }

/**
 * @brief Definition (implementation) of Singleton for a class
 *
 */
#define HK_SINGLETON_IMPL(CLASS) \
    ::hk::util::Storage<CLASS> CLASS::sSingletonStorage;
