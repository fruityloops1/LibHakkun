.global __module_entry__
.global rtldEntry

#ifdef __aarch64__
__module_entry__:
    cbz x0, rtldEntry
    mov w0, #0xf801
    b _ZN2hk3svc19ReturnFromExceptionENS_6ResultE
#else
__module_entry__:
    cmp r0, #0x0
    beq rtldEntry
    mov r0, #0xf801
    b _ZN2hk3svc19ReturnFromExceptionENS_6ResultE
#endif