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

SVC_BEGIN _ZN2hk3svc11CloseHandleENS_6HandleE
    svc #0x16
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc13ArbitrateLockENS_6HandleEjj
	svc 0x1A
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc15ArbitrateUnlockEj
	svc 0x1B
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc20SendSyncRequestLightEjPj
    mov r8, r1
    ldr r1, [r8, #0]
	ldr r2, [r8, #4]
    ldr r3, [r8, #8]
	ldr r4, [r8, #12]
    ldr r5, [r8, #16]
	ldr r6, [r8, #20]
    ldr r7, [r8, #24]
    str r8, [sp, #-4]!
    svc 0x20
    ldr r8, [sp], #4
    add sp, sp, #4
    str r1, [r8, #0]
	str r2, [r8, #4]
    str r3, [r8, #8]
	str r4, [r8, #12]
    str r5, [r8, #16]
	str r6, [r8, #20]
    str r7, [r8, #24]
    bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc15SendSyncRequestENS_6HandleE
	svc 0x21
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc29SendSyncRequestWithUserBufferEjjNS_6HandleE
	svc 0x22
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc5BreakENS0_11BreakReasonEPvj
	svc 0x26
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc16BreakWithMessageENS0_11BreakReasonEPvjS2_S2_
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

SVC_BEGIN _ZN2hk3svc7GetInfoEPyNS0_8InfoTypeENS_6HandleEy
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

SVC_BEGIN _ZN2hk3svc20ReplyAndReceiveLightEjPh
    mov r8, r1
    ldr r1, [r8, #0]
	ldr r2, [r8, #4]
    ldr r3, [r8, #8]
	ldr r4, [r8, #12]
    ldr r5, [r8, #16]
	ldr r6, [r8, #20]
    ldr r7, [r8, #24]
    str r8, [sp, #-4]!
    svc 0x42
    ldr r8, [sp], #4
    str r1, [r8, #0]
	str r2, [r8, #4]
    str r3, [r8, #8]
	str r4, [r8, #12]
    str r5, [r8, #16]
	str r6, [r8, #20]
    str r7, [r8, #24]
    bx lr
SVC_END

// the abi for the following svcs is completely Wrong
SVC_BEGIN _ZN2hk3svc26InvalidateProcessDataCacheENS_6HandleEjj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    mov r4, #0x0
    svc 0x5d
	ldmia sp!, {r3, r4, pc}
SVC_END

SVC_BEGIN _ZN2hk3svc21FlushProcessDataCacheENS_6HandleEjj
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
	str r0, [sp, #-4]!
	svc 0x71
    ldr r2, [sp]
    str r1, [r2]
    add sp, sp, #4
	bx lr
SVC_END

SVC_BEGIN _ZN2hk3svc16MapProcessMemoryEjNS_6HandleEyj
    stmdb sp!, {r3, r4, lr}
    mov r3, #0x0
    ldr r4, [sp, #12]
	svc 0x74
	ldmia sp!, {r3, r4, pc}
SVC_END
