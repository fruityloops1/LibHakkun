.global __module_start__
.global __mod0
.hidden hk_rtld_module

.section ".text.modulestart","a"
__module_start__:
    b __module_entry__
    .word __mod0 - __module_start__

.section ".rodata.mod0","a"
.align 2
__mod0:
    .ascii "MOD0"
    .word  _DYNAMIC               - __mod0
    .word  __bss_start__          - __mod0
    .word  __bss_end__            - __mod0
    .word  __eh_frame_hdr_start   - __mod0
    .word  __eh_frame_hdr_end     - __mod0
    .word  hkRtldModule           - __mod0
