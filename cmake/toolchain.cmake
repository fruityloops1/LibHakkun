message(STATUS "Current working directory: ${CMAKE_BINARY_DIR}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostartfiles -Wno-unused-command-line-argument")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -nostartfiles -Wno-unused-command-line-argument")

if (CMAKE_SOURCE_DIR MATCHES "TryCompile") # There is no Better way to eliminate this Parasite
    return()
endif()

include(config/config.cmake)

if (IS_OUNCE)
    set(CMAKE_SYSTEM_NAME Ounce)
    set(PLATFORM_MACRO OUNCE)

    set(TARGET_TRIPLE "aarch64-none-elf")
    set(CPU "cortex-a78c")
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(ARCH_NAME "aarch64")
    set(TARGET_FLAGS "-mbranch-protection=pac-ret+b-key")

    if (IS_32_BIT)
        message(FATAL_ERROR "ILP32 not supported")
    endif()
else()
    set(CMAKE_SYSTEM_NAME NX)
    set(PLATFORM_MACRO NX)

    if (IS_32_BIT)
        set(TARGET_TRIPLE "armv7-none-eabi")
        set(CPU "cortex-a57")
        set(CMAKE_SYSTEM_PROCESSOR armv7-a)
        set(ARCH_NAME "aarch32")
    else()
        set(TARGET_TRIPLE "aarch64-none-elf")
        set(CPU "cortex-a57")
        set(CMAKE_SYSTEM_PROCESSOR aarch64)
        set(ARCH_NAME "aarch64")
    endif()
endif()

set(CMAKE_SYSTEM_VERSION "${CMAKE_SYSTEM_NAME}/Clang")
set(CMAKE_ASM_COMPILER "clang")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXECUTABLE_SUFFIX ".nss")
set(ARCH_FLAGS "--target=${TARGET_TRIPLE} -mcpu=${CPU} -fPIC -nodefaultlibs ${TARGET_FLAGS}")

set(DEFAULTDEFINES
    -D_POSIX_C_SOURCE=200809L
    -D_LIBUNWIND_IS_BAREMETAL
    -D_LIBCPP_HAS_THREAD_API_PTHREAD
    -D_GNU_SOURCE
    -DNNSDK
    -D${PLATFORM_MACRO}
)

set(LIBSTD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/std")

set(DEFAULTLIBS
    ${LIBSTD_PATH}/libc.a
    ${LIBSTD_PATH}/libc++.a
    ${LIBSTD_PATH}/libc++abi.a
    ${LIBSTD_PATH}/libm.a
    ${LIBSTD_PATH}/libunwind.a
)

if (IS_32_BIT)
    list(APPEND DEFAULTLIBS
        ${LIBSTD_PATH}/libclang_rt.builtins-arm.a
    )
else()
    list(APPEND DEFAULTLIBS
        ${LIBSTD_PATH}/libclang_rt.builtins-aarch64.a
    )
endif()

set(DEFAULTINCLUDES
    ${LIBSTD_PATH}/llvm-project/build/include/c++/v1
    ${LIBSTD_PATH}/llvm-project/libunwind/include
    ${LIBSTD_PATH}/musl/include
    ${LIBSTD_PATH}/musl/obj/include
    ${LIBSTD_PATH}/musl/arch/generic
)

if (IS_32_BIT)
    list(APPEND DEFAULTINCLUDES
        ${LIBSTD_PATH}/musl/arch/arm
    )
else()
    list(APPEND DEFAULTINCLUDES
        ${LIBSTD_PATH}/musl/arch/aarch64
    )
endif()

set(STDLIB_FOUND TRUE)
foreach (item IN LISTS DEFAULTLIBS)
    if (NOT EXISTS ${item})
        set(STDLIB_FOUND FALSE)
    endif()
endforeach()
foreach (item IN LISTS DEFAULTINCLUDES)
    if (NOT EXISTS ${item})
        set(STDLIB_FOUND FALSE)
    endif()
endforeach()

if (NOT STDLIB_FOUND)
    message(WARNING "Standard library not found! Running setup_libcxx_prepackaged.py")
    execute_process(COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/sys/tools/setup_libcxx_prepackaged.py ${ARCH_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE result
    )
endif()

set(DEFAULTINCLUDES_F "")
foreach(item IN LISTS DEFAULTINCLUDES)
    set(DEFAULTINCLUDES_F "${DEFAULTINCLUDES_F} -isystem ${item}")
endforeach()
set(DEFAULTLIBS_F "")
foreach(item IN LISTS DEFAULTLIBS)
    set(DEFAULTLIBS_F "${DEFAULTLIBS_F} ${item}")
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL Release)
    list(APPEND DEFAULTDEFINES -DHK_RELEASE)
endif()
if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    list(APPEND DEFAULTDEFINES -DHK_RELEASE -DHK_RELEASE_DEBINFO)
endif()

if (NOT EXCEPTION_FLAGS)
    message(WARNING "EXCEPTION_FLAGS not set, using -fno-exceptions -fno-rtti")
    set(EXCEPTION_FLAGS "-fno-exceptions -fno-rtti")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F} ${EXCEPTION_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DEFAULTLIBS_F}")

add_definitions(${DEFAULTDEFINES})
