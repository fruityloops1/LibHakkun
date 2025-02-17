# Hakkun

Code modification framework for 64-bit RTLD-based userspace Nintendo Switch programs. This is the library repository to be cloned as submodule into a project. An example project can be found [here](https://github.com/fruityloops1/Hakkun-Example)

## Features
* Clang/LLVM toolchain, linking musl libc + LLVM libc++
* Compiled into module to be loaded by RTLD
* Compatible with programs that statically link RTLD into their main executable
* Runtime Replace, Trampoline, B/BL hooking, RW access to code memory
* Sophisticated symbol sourcing system with proper error reporting and compatibility across multiple program target versions
* Abort/Assert util that prints text visible in Atmosphère crash reports
* Symbols imported dynamically can be stripped from final binary
* Framework and user code clearly separated with a submodule to prevent messy codebases and forks

## Setup
#### Prerequisites
* CMake + GNUMake or Ninja
* cURL
* Clang, LLVM, LLD 18 or later
* Python 3.10, `pyelftools` and `mmh` packages
* [switch-tools](https://github.com/switchbrew/switch-tools) bin path in `SWITCHTOOLS` env variable, or devkitPro distribution of switch-tools
#### Compile stdlibs and sail

You can either use a prepackaged stdlib, or compile one yourself:
##### Using prepackaged stdlib
Run `tools/setup_libcxx_prepackaged.py` from your repository's root to download a pre-packaged stdlib. (~20MiB)
##### Compiling stdlib yourself
After setting up a repository in a similar fashion to the [example repository](https://github.com/fruityloops1/Hakkun-Example), run `tools/setup_libcxx.py` from your repository's root to download and compile musl libc and LLVM libc++. (~1.7GiB)
##### Sail
Run `tools/setup_sail.py` from your repository's root to compile sail, the tool used to parse .sym files for symbol sourcing.

#### Build
With a CMakeLists.txt setup similar to the [example repository](https://github.com/fruityloops1/Hakkun-Example), the project can be built by running CMake from the build folder and calling the build system:
* `mkdir build`
* `cd build`
* `cmake .. -DCMAKE_BUILD_TYPE=Debug` (or Release)
* Run `make` or `ninja` depending on which you used in the above command

## Deploy
Hakkun provides 2 ways of deploying the built files into a usable structure, besides manually copying from the build folder:
#### SD
By default, module files are output into the `sd` folder in the build folder in Atmosphère's LayeredFS structure:
```
sd
└── atmosphere
    └── contents
        └── 0100000000010000
            └── exefs
                ├── main.npdm
                └── subsdk4
```
#### FTP
If following environment variables are set, Hakkun will automatically attempt to upload files to an FTP server after linking:
* `HAKKUN_FTP_IP`: IP (e.g. 192.168.188.151, my switch)
* `HAKKUN_FTP_PORT`: Port (optional, 5000 by default)
* `HAKKUN_FTP_USER`: User (optional)
* `HAKKUN_FTP_PASS`: Password (optional)

## Configuration
#### config.cmake
Hakkun provides various options that you can configure from `config/config.cmake` within your repository:
* `LINKFLAGS`: Flags passed to compiler linking command
* `LLDFLAGS`: Flags passed to linker
* `OPTIMIZE_OPTIONS_DEBUG`: Optimization options for Debug mode
* `OPTIMIZE_OPTIONS_RELEASE`: Optimization options for Release mode
* `WARN_OPTIONS`: Warn options
* `DEFINITIONS`: Preprocessor definitions
* `INCLUDES`: Include directories
* `ASM_OPTIONS`, `C_OPTIONS`, `CXX_OPTIONS`: Various compiler options
* `TARGET_IS_STATIC`: Whether or not target program has statically linked rtld/sdk. Usually sysmodules or applets do this, Applications do not. Enabling this will also add a dummy RTLD module, to work around an unfortunate decision in `loader`
* `MODULE_NAME`: Name of your output RTLD module
* `TITLE_ID`: Title ID of the target program
* `MODULE_BINARY`: ExeFS slot for your output module (can be sdk, or subsdk0-subsdk9)
* `SDK_PAST_1900`: Enable if RTLD version of target program is from SDK 19.0.0 or later, usually the case with titles updated in or later than late 2024
* `TRAMPOLINE_POOL_SIZE`: Maximum amount of trampoline hooks
* `BAKE_SYMBOLS`: Whether or not to 'bake' symbols provided by sail. Baking will replace all string references to symbols to hashes, reducing binary size at the expense of harder debugging
#### Sail
Sail reads 2 configuration files:
##### ModuleList.sym
List of modules available to sail, bound to their indices. For example:
```
rtld = 0
smo = 1
sdk = 4
```
##### VersionList.sym
List of versions for modules bound to NSO build IDs, if versioning is desired, for example:
```
@smo
100 = 3ca12dfaaf9c82da064d1698df79cda1
101 = 50ade4b5eb6e45efb170a6b230d3b0ba
110 = 948dbbfc2fa0c60e2c30316e4c961aba
120 = f5dccddb37e97724ebdbcccdbeb965ff
130 = b424be150a8e7d78701cbe7a439d9ebf
@sdk
100 = ae34e75d02925f4417b24499ad80c39412fc76db
101 = ae34e75d02925f4417b24499ad80c39412fc76db
110 = ae34e75d02925f4417b24499ad80c39412fc76db
120 = ae34e75d02925f4417b24499ad80c39412fc76db
130 = fcf93a41b08c5193a1410d262602dc621617c56f
```
#### npdm.json
`config/npdm.json` configures the NPDM file required to grant certain SVC permissions needed.
The following SVC permissions are used and need to be allowed for Hakkun to function:
```
"svcQueryMemory": "0x06",
"svcBreak": "0x26",
"svcOutputDebugString": "0x27",
"svcGetInfo": "0x29",
"svcMapProcessMemory": "0x74",
```

## Symbol Language
Hakkun provides sail, a tool that allows parsing of a symbol language contained within .sym files in the sym/ directory of your repository, for the purpose of sourcing symbols across different versions of programs. After configuring ModuleList.sym and VersionList.sym, the following ways of adding symbols can be used:
```cpp
// immediate symbols
@game:100

coolsymbol = 0x1234
coolsymbol_2 = 0x1238

@game:110 // different version

coolsymbol = 0x1334
coolsymbol_2 = 0x1338

// dynamic symbols:
@sdk:100,110 // multiple versions

_ZN2nn3abc7acquireEv = "_ZN2nn3abc7acquireEv"
_ZN2nn3abc7acquireEv // compact
_ZN2nn3abc8OneThingEv = "_ZN2nn3abc12AnotherThingEv" // different

// data search blocks

// version doesn't matter for data search
some_cool_sead_thingy = 0123456789ABCDEF @ game + 0x20 // < some instructions
some_other_sead_thingy = 0123456789ABCDEF @ game - 0x20

// data may change
some_other_sead_thingy2 = FEDCBA9876543210 @ game<110 - 0x20 // before 910
some_other_sead_thingy3 = 0123456789ABCDEF @ game>110 - 0x20 // at and after 910

// optimization
some_shit = 0123456789ABCDEF @ game:text + 0x20 // only search .text section
some_balls = 01234567 @ game:rodata // no addend

// addend

a_close_symbol = some_balls + 0x1000
another_close_symbol = a_close_symbol - 0x124

// ReadADRPGlobal
$some_adrp_ldr_instruction = 0123456789ABCDEF @ game - 0x20
some_global_variable = readAdrpGlobal($some_adrp_ldr_instruction)
some_global_variable = readAdrpGlobal($some_adrp_ldr_instruction, 0x2) // 0x2 indicates the offset of the LDR instruction from the ADRP instruction
```
Symbols can be accessed directly through linking, or through hk::util::lookupSymbol.

## Hooking
Hakkun provides multiple utils for hooking:
```cpp
using namespace hk;

HkTrampoline<int, void*> myHook = hook::trampoline([](void* something) -> int {
    int value = myHook.orig(something); // call original function
    // do something ...
    return value;
});

static void test() { }

extern "C" void hkMain() {
    // tramopline/replace
    myHook.installAtSym<"SomeFunction">(); // install to symbol provided by sail
    myHook.uninstall(); // hooks can be uninstalled
    myHook.installAtOffset(ro::getMainModule(), 0x1234); // by offset (not recommended)
    myHook.uninstall();
    myHook.installAtPtr(/* rare usecase */);

    // replace hooks are also a thing, use HkReplace instead of HkTrampoline

    // b/bl branching to function
    hook::writeBranchLinkAtSym<"SomeFunction2">(test);
    hook::writeBranchAtPtr(1234, test);

    // writing to module
    ro::getMainModule()->writeRo<u32>(/* offset */ 0x12345678, /* u32 value */ 0xfefefefe);
}
```
Please note trampoline hooks do not relocate instructions at the moment, which should not be a problem as long as you hook at instructions that do not need to be relocated (avoid branches, adrp, etc.)

## Credits:
* tetraxile for some help and testing
* shadowninja108
* marysaka for [oss-rtld](https://github.com/marysaka/oss-rtld)
