set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_VERSION "NX/Clang")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")

set(CMAKE_ASM_COMPILER "clang")
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXECUTABLE_SUFFIX ".nss")
set(ARCH_FLAGS "--target=aarch64-none-elf -march=armv8-a -mtune=cortex-a57 -fPIC -nodefaultlibs")

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
    ${LIBSTD_PATH}/libclang_rt.builtins-aarch64.a
    ${LIBSTD_PATH}/libm.a
    ${LIBSTD_PATH}/libunwind.a
)

set(DEFAULTINCLUDES
    ${LIBSTD_PATH}/llvm-project/build/include/c++/v1
    ${LIBSTD_PATH}/llvm-project/libunwind/include
    ${LIBSTD_PATH}/musl/include
    ${LIBSTD_PATH}/musl/obj/include
    ${LIBSTD_PATH}/musl/arch/generic
    ${LIBSTD_PATH}/musl/arch/aarch64
)

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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} ${DEFAULTINCLUDES_F} -fno-exceptions -fno-rtti")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ARCH_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DEFAULTLIBS_F}")

add_definitions(${DEFAULTDEFINES})

