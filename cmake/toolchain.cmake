message(STATUS "Current working directory: ${CMAKE_BINARY_DIR}")

string(FIND ${CMAKE_SOURCE_DIR} "TryCompile" TRYCOMPILE_IDX)
if (TRYCOMPILE_IDX EQUAL -1) # There is no Better way to eliminate this Parasite
    include(config/config.cmake)
endif()

if (IS_32_BIT)
    set(TARGET_TRIPLE "armv7-none-eabi")
    set(MARCH "armv7-a")
    set(CMAKE_SYSTEM_PROCESSOR armv7-a)
else()
    set(TARGET_TRIPLE "aarch64-none-elf")
    set(MARCH "armv8-a")
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION "NX/Clang")
set(CMAKE_ASM_COMPILER "clang")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXECUTABLE_SUFFIX ".nss")
set(ARCH_FLAGS "--target=${TARGET_TRIPLE} -march=${MARCH} -mtune=cortex-a57 -fPIC -nodefaultlibs")

set(DEFAULTDEFINES
    -D_POSIX_C_SOURCE=200809L
    -D_LIBUNWIND_IS_BAREMETAL
    -D_LIBCPP_HAS_THREAD_API_PTHREAD
    -D_GNU_SOURCE
    -DNNSDK
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F} -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DEFAULTLIBS_F}")

add_definitions(${DEFAULTDEFINES})

