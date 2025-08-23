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


SVC_BEGIN _ZN2hk3svc9MapMemoryEmmm
	svc 0x4
	ret
SVC_END

SVC_BEGIN svcUnmapMemory
	svc 0x5
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11QueryMemoryEPNS0_10MemoryInfoEPjm
	str x1, [sp, #-16]!
	svc 0x6
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc13ArbitrateLockEjmj
	svc 0x1A
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc15ArbitrateUnlockEm
	svc 0x1B
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc15SendSyncRequestEj
    svc 0x21
    ret
SVC_END

SVC_BEGIN _ZN2hk3svc5BreakENS0_11BreakReasonEPvm
	svc 0x26
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc18hkBreakWithMessageENS0_11BreakReasonEPvmS2_S2_
	svc 0x26
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc17OutputDebugStringEPKcm
	svc 0x27
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc7GetInfoEPmNS0_8InfoTypeEjm
	str x0, [sp, #-16]!
	svc 0x29
	ldr x2, [sp], #16
	str x1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc14GetProcessListEPiPmi
    str x0, [sp, #-16]!
	svc 0x65
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc26SetProcessMemoryPermissionEjmmNS0_16MemoryPermissionE
	svc 0x73
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc16MapProcessMemoryEmjmm
	svc 0x74
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc18UnmapProcessMemoryEmjmm
	svc 0x75
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc20MapProcessCodeMemoryEjmmm
	svc 0x77
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc22UnmapProcessCodeMemoryEjmmm
	svc 0x78
	ret
SVC_END
