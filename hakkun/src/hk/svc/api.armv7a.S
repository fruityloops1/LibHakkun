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

SVC_BEGIN _ZN2hk3svc11ExitProcessEv
	svc #0x7
SVC_END

SVC_BEGIN _ZN2hk3svc12CreateThreadEPjPFvmEmmii
    str r0, [sp, #-4]!
    svc #0x8
    ldr r2, [sp]
    str r1, [r2]
    add sp, sp, #4
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc10ExitThreadEv
	svc #0xA
SVC_END

SVC_BEGIN _ZN2hk3svc11SleepThreadEl
	svc #0xB
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc20CreateTransferMemoryEPjjjNS0_16MemoryPermissionE
    str r0, [sp, #-4]!
    svc #0x15
    ldr r2, [sp]
    str r1, [r2]
    add sp, sp, #4
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc11CloseHandleEj
    svc #0x16
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

SVC_BEGIN _ZN2hk3svc20SendSyncRequestLightEjPj
    mov r8, r1
    ldrd r1, r2, [r8, #0]
    ldrd r3, r4, [r8, #8]
    ldrd r5, r6, [r8, #16]
    ldr r7, [r8, #24]
    str r8, [sp, #-4]!
    svc 0x20
    ldr r8, [sp], #4
    add sp, sp, #4
    strd r1, r2, [r8, #0]
    strd r3, r4, [r8, #8]
    strd r5, r6, [r8, #16]
    str r7, [r8, #24]
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

SVC_BEGIN _ZN2hk3svc19ReturnFromExceptionENS_6ResultE
	svc 0x28
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

SVC_BEGIN _ZN2hk3svc20ReplyAndReceiveLightEjPj
    mov r8, r1
    ldrd r1, r2, [r8, #0]
    ldrd r3, r4, [r8, #8]
    ldrd r5, r6, [r8, #16]
    ldr r7, [r8, #24]
    str r8, [sp, #-4]!
    svc 0x20
    ldr r8, [sp], #4
    strd r1, r2, [r8, #0]
    strd r3, r4, [r8, #8]
    strd r5, r6, [r8, #16]
    str r7, [r8, #24]
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

SVC_BEGIN _ZN2hk3svc13GetSystemInfoEPvNS0_14SystemInfoTypeEjNS0_24PhysicalMemorySystemInfoE
    str r1, [sp, #-4]!
    svc #0x6F
    ldr r2, [sp]
    str r1, [r2]
    add sp, sp, #4
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc15ManageNamedPortEPjPKci
	str x0, [sp, #-16]!
	svc 0x74
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc16MapProcessMemoryEjjyj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    ldr r4, [sp, #12]
	svc 0x74
	ldmia sp!, {r3, r4, pc}
SVC_END
