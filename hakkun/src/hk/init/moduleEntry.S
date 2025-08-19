.global __module_start__

.section ".text.modulestart","a"
__module_start__:
    b module_entry_stub
    .word __mod0 - __module_start__
module_entry_stub:
    b __module_entry__
