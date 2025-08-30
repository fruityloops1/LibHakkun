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

SVC_BEGIN _ZN2hk3svc11QueryMemoryEPNS0_10MemoryInfoEPjm
	str x1, [sp, #-16]!
	svc 0x6
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11ExitProcessEv
	svc #0x7
SVC_END

SVC_BEGIN _ZN2hk3svc12CreateThreadEPjPFvmEmmii
	str x0, [sp, #-16]!
	svc #0x8
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11StartThreadEj
	svc 0x9
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc10ExitThreadEv
	svc #0xA
SVC_END

SVC_BEGIN _ZN2hk3svc11SleepThreadEl
	svc #0xB
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11SignalEventEj
	svc 0x11
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc20CreateTransferMemoryEPjmmNS0_16MemoryPermissionE
	str x0, [sp, #-16]!
	svc #0x15
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11CloseHandleEj
	svc #0x16
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc19WaitSynchronizationEPiPKjil
	str x0, [sp, #-16]!
	svc 0x18
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

SVC_BEGIN _ZN2hk3svc18ConnectToNamedPortEPjPKc
	str x0, [sp, #-16]!
	svc 0x1F
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc20SendSyncRequestLightEjPh
  mov x8, x1
  ldp w1, w2, [x8, #0]
  ldp w3, w4, [x8, #8]
  ldp w5, w6, [x8, #16]
  ldr w7, [x8, #24]
  str x8, [sp, #-16]!
  svc 0x20
  ldr x8, [sp], #16
  stp w1, w2, [x8, #0]
  stp w3, w4, [x8, #8]
  stp w5, w6, [x8, #16]
  str w7, [x8, #24]
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

SVC_BEGIN _ZN2hk3svc16BreakWithMessageENS0_11BreakReasonEPvmS2_S2_
	svc 0x26
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc17OutputDebugStringEPKcm
	svc 0x27
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc19ReturnFromExceptionENS_6ResultE
	svc 0x28
SVC_END

SVC_BEGIN _ZN2hk3svc7GetInfoEPmNS0_8InfoTypeEjm
	str x0, [sp, #-16]!
	svc 0x29
	ldr x2, [sp], #16
	str x1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc14WaitForAddressEPKvNS0_15ArbitrationTypeEjm
	svc 0x34
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc15SignalToAddressEPvNS0_10SignalTypeEjm
	svc 0x35
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc13AcceptSessionEPjj
	str x0, [sp, #-16]!
	svc 0x41
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc20ReplyAndReceiveLightEjPh
  mov x8, x1
  ldp w1, w2, [x8, #0]
  ldp w3, w4, [x8, #8]
  ldp w5, w6, [x8, #16]
  ldr w7, [x8, #24]
  str x8, [sp, #-16]!
  svc 0x42
  ldr x8, [sp], #16
  stp w1, w2, [x8, #0]
  stp w3, w4, [x8, #8]
  stp w5, w6, [x8, #16]
  str w7, [x8, #24]
  ret
SVC_END

SVC_BEGIN _ZN2hk3svc15ReplyAndReceiveEPjPKjjjm
	str x0, [sp, #-16]!
	svc 0x43
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc11CreateEventEPjS1_
	stp x0, x1, [sp, #-16]!
	svc 0x45
	ldp x3, x4, [sp], #16
	str w1, [x3]
	str w2, [x4]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc14GetProcessListEPiPmi
	str x0, [sp, #-16]!
	svc 0x65
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc13GetSystemInfoEPvNS0_14SystemInfoTypeEjNS0_24PhysicalMemorySystemInfoE
	str x0, [sp, #-16]!
	svc 0x6F
	ldr x2, [sp], #16
	str x1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc15ManageNamedPortEPjPKci
	str x0, [sp, #-16]!
	svc 0x71
	ldr x2, [sp], #16
	str w1, [x2]
	ret
SVC_END

SVC_BEGIN _ZN2hk3svc16MapProcessMemoryEmjmm
	svc 0x74
	ret
SVC_END
