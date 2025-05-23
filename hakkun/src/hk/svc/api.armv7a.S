.macro SVC_BEGIN name
	.section .text.\name, "ax", %progbits
	.global \name
	.type \name, %function
	.hidden \name
	.align 2
	.cfi_startproc
\name:
.endm

.macro SVC_END
	.cfi_endproc
.endm

SVC_BEGIN _ZN2hk3svc11QueryMemoryEPNS0_10MemoryInfoEPjj
    str r1, [sp, #-4]!
    svc #0x6
    ldr r2, [sp]
    str r1, [r2]
    add sp, sp, #4
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc13ArbitrateLockEjjj
	svc 0x1A
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc15ArbitrateUnlockEj
	svc 0x1B
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc5BreakENS0_11BreakReasonEPvj
	svc 0x26
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc18hkBreakWithMessageENS0_11BreakReasonEPvjS2_S2_
    ldr r4, [sp, #0x0]
    mov r5, #0x42 // Prevent showing message twice
	svc 0x26
SVC_END

SVC_BEGIN _ZN2hk3svc17OutputDebugStringEPKcj
	svc 0x27
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc7GetInfoEPyNS0_8InfoTypeEjy
    str r0, [sp, #-4]!
    ldr r0, [sp, #4]
    ldr r3, [sp, #8]
    svc #0x29
    ldr r3, [sp]
    str r1, [r3]
    str r2, [r3, #4]
    add sp, sp, #4
    bx lr
SVC_END

// the abi for the following svcs is completely Wrong
SVC_BEGIN _ZN2hk3svc26InvalidateProcessDataCacheEjjj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    mov r4, #0x0
    svc 0x5d
	ldmia sp!, {r3, r4, pc}
SVC_END

SVC_BEGIN _ZN2hk3svc21FlushProcessDataCacheEjjj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    mov r4, #0x0
    svc 0x5f
	ldmia sp!, {r3, r4, pc}
SVC_END

SVC_BEGIN _ZN2hk3svc16MapProcessMemoryEjjyj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    ldr r4, [sp, #12]
	svc 0x74
	ldmia sp!, {r3, r4, pc}
SVC_END
