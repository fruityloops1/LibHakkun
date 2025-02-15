OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
ENTRY(__module_start__)

PHDRS
{
    text PT_LOAD FLAGS(5);
    rodata PT_LOAD FLAGS(4);
    data PT_LOAD FLAGS(6);
    dynamic PT_DYNAMIC;
    tdata PT_TLS FLAGS(6);
}

SECTIONS
{
    . = 0x0;
    __module_start__ = .;

    .text : {
        __text_start__ = .;
        KEEP (*(.text.modulestart))
        KEEP (*(.text.crt0))
        *(.text .text.*)
        *(.plt .plt.*)
        __text_end__ = .;
    } :text

    . = ALIGN(0x1000);

    .rodata : {
        KEEP (*(.rodata.modulename))
        KEEP (*(.rodata.mod0))
        *(.rodata .rodata.*)
    } :rodata

    .hash     : { *(.hash) } :rodata
    .gnu.hash : { *(.gnu.hash) } :rodata
    .dynsym   : { *(.dynsym .dynsym.*) } :rodata
    .dynstr   : { *(.dynstr .dynstr.*) } :rodata

    .gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) } :rodata
    .eh_frame_hdr : {
        __eh_frame_hdr_start = .;
        *(.eh_frame_hdr)
        __eh_frame_hdr_end = .;

        __eh_frame_start = .;
        *(.eh_frame)
        __eh_frame_end = .;
    } :rodata
    .eh_frame : { KEEP (*(.eh_frame)) } :rodata

    .note.gnu.build-id : { *(.note.gnu.build-id) } :rodata

    . = ALIGN(0x1000);

    .data : {
        *(.data .data.*)
    } :data

    .data.rela.ro : {
        *(.data.rela.ro.local*)
        *(.data.rela.ro .data.rela.ro.*)
    } :data

    .data.rel.ro : {
        *(.data.rel.ro.local*)
        *(.data.rel.ro .data.rel.ro.*)
    } :data

    .got : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) } :data

    .preinit_array ALIGN(8) : {
        __preinit_array_start__ = .;
        KEEP(*(.preinit_array))
        __preinit_array_end__ = .;
    } :data

    .init_array ALIGN(8) : {
        __init_array_start__ = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end__ = .;
    } :data

    .fini_array ALIGN(8) : {
        __fini_array_start__ = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
        KEEP(*(.fini_array))
        __fini_array_end__ = .;
    } :data

    .saildb : {
        *(.saildb)
    } :data

    .tdata : {
        __tdata_align_abs__ = ABSOLUTE(.);
        *(.tdata .tdata.*)
    } :tdata

    . = ALIGN(0x1000);

    .tbss : {
        __tbss_align_abs__ = ABSOLUTE(.);
        *(.tbss .tbss.*)
        *(.tcommon)
    } :tdata

    .bss : {
        __bss_start__ = .;
        KEEP (*(.bss.rtldmodule))
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end__ = .;
        . = ALIGN(0x1000);
    } :data

    __module_end__ = ABSOLUTE(.);
}
