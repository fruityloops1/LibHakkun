.global __module_start__

.section ".text.modulestart","a"
#if HK_TARGET == HK_TARGET_MODULE_DLL
.size __module_start__, 0x80 // space for NRO header
__module_start__:
    b __module_entry__
    .word __mod0 - __module_start__
    .fill 0x8, 1, 0
    .ascii "NRO0"
    .fill 0x6C, 1, 0
#else // HK_TARGET_MODULE, HK_TARGET_MODULE_STANDALONE
__module_start__:
    b module_entry_stub
    .word __mod0 - __module_start__
module_entry_stub:
    b __module_entry__
#endif