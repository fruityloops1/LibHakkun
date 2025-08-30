#pragma once

#include "hk/ValueOrResult.h"
#include "hk/types.h"
#include "hk/util/Tuple.h"

#include "hk/svc/cpu.h"

namespace hk::svc {

    Result QueryMemory(MemoryInfo* outMemoryInfo, u32* outPageInfo, ptr address);
    Result MapMemory(ptr dest, ptr source, size size);
    Result UnmapMemory(ptr addr, ptr source, size size);
    hk_noreturn void ExitProcess();
    Result CreateThread(Handle* outHandle, ThreadFunc func, ptr arg, ptr stackTop, s32 priority, s32 coreId);
    Result StartThread(Handle threadHandle);
    hk_noreturn void ExitThread();
    // values 0, -1, and -2 will yield the thread.
    // see https://switchbrew.org/wiki/SVC#SleepThread
    void SleepThread(s64 nanoseconds);
    Result CreateTransferMemory(Handle* outHandle, ptr address, size size, MemoryPermission perm);
    Result CloseHandle(Handle handle);
    Result SignalEvent(Handle handle);
    Result ClearEvent(Handle handle);
    Result WaitSynchronization(s32* outIdx, const Handle* handles, s32 numHandles, s64 timeout);
    Result CancelSynchronization(Handle handle);
    Result ResetSignal(Handle handle);
    Result ArbitrateLock(Handle threadHandle, ptr addr, u32 tag);
    Result ArbitrateUnlock(ptr addr);
    Result ConnectToNamedPort(Handle* outHandle, const char* name);
    Result SendSyncRequestLight(Handle sessionHandle, u8 data[28]);
    Result SendSyncRequest(Handle sessionHandle);
    hk_noreturn Result Break(BreakReason reason, void* arg, size argSize);
    Result OutputDebugString(const char* str, size_t len);
    hk_noreturn void ReturnFromException(Result result);
    Result AcceptSession(Handle* outSessionHandle, Handle portHandle);
    Result ReplyAndReceiveLight(Handle sessionHandle, u8 data[28]);
    Result ReplyAndReceive(u32* outIndex, const Handle* handles, u32 handleCount, Handle replyHandle, u64 timeout);
    Result CreateEvent(Handle* outWriteHandle, Handle* outReadHandle);
    Result ManageNamedPort(Handle* outHandle, const char* name, s32 maxSessions);
    Result GetInfo(u64* out, InfoType type, Handle handle, u64 subType);
    Result WaitForAddress(const void* address, ArbitrationType type, u32 value, u64 timeout);
    Result SignalToAddress(void* address, SignalType type, u32 value, u64 timeout);
    Result InvalidateProcessDataCache(Handle process, ptr addr, size size);
    Result FlushProcessDataCache(Handle process, ptr addr, size size);
    Result DebugActiveProcess(Handle* outHandle, u64 processId);
    Result BreakDebugProcess(Handle debugHandle);
    Result TerminateDebugProcess(Handle debugHandle);
    Result GetDebugEvent(DebugEventInfo* outInfo, Handle debugHandle);
    Result ContinueDebugEvent(Handle debugHandle, u32 flags, const u64* threadIds, s32 numThreadIds);
    Result GetProcessList(s32* outNumProcesses, u64* outProcessIds, s32 maxProcesses);
    Result QueryDebugProcessMemory(MemoryInfo* outMemoryInfo, u32* outPageInfo, Handle debugHandle, ptr address);
    Result ReadDebugProcessMemory(void* buffer, Handle debugHandle, ptr address, size size);
    Result WriteDebugProcessMemory(Handle debugHandle, const void* buffer, ptr address, size size);
    Result GetSystemInfo(u64* outInfo, SystemInfoType infoType, Handle handle, PhysicalMemorySystemInfo infoSubType);
    Result MapProcessMemory(ptr dest, Handle process, u64 source, size size);

    // ValueOrResult wrappers

    inline hk_alwaysinline ValueOrResult<Tuple<MemoryInfo, u32>> QueryMemory(ptr address) {
        MemoryInfo info;
        u32 pageInfo;

        HK_TRY(QueryMemory(&info, &pageInfo, address));

        return Tuple<MemoryInfo, u32> { info, pageInfo };
    }

    inline hk_alwaysinline ValueOrResult<Handle> CreateThread(void (*func)(ptr arg), ptr arg, ptr stackTop, s32 priority, s32 coreId) {
        Handle outHandle;

        HK_TRY(CreateThread(&outHandle, func, arg, stackTop, priority, coreId));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<Handle> CreateTransferMemory(ptr address, size size, MemoryPermission perm) {
        Handle outHandle;

        HK_TRY(CreateTransferMemory(&outHandle, address, size, perm));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<Handle> ConnectToNamedPort(const char* name) {
        Handle outHandle;

        HK_TRY(ConnectToNamedPort(&outHandle, name));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<s32> WaitSynchronization(const Handle* handles, s32 numHandles, s64 timeout) {
        s32 outIdx;

        HK_TRY(WaitSynchronization(&outIdx, handles, numHandles, timeout));

        return outIdx;
    }

    inline hk_alwaysinline ValueOrResult<Tuple<Handle, Handle>> CreateEvent() {
        Handle writeHandle;
        Handle readHandle;

        HK_TRY(CreateEvent(&writeHandle, &readHandle));

        return Tuple<Handle, Handle>(writeHandle, readHandle);
    }

    inline hk_alwaysinline ValueOrResult<u64> GetInfo(InfoType type, Handle handle, u64 subType) {
        u64 value;

        HK_TRY(GetInfo(&value, type, handle, subType));

        return value;
    }

    inline hk_alwaysinline ValueOrResult<Handle> DebugActiveProcess(u64 processId) {
        Handle outHandle;

        HK_TRY(DebugActiveProcess(&outHandle, processId));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<u64> GetSystemInfo(SystemInfoType infoType, Handle handle, PhysicalMemorySystemInfo infoSubType) {
        u64 value;

        HK_TRY(GetSystemInfo(&value, infoType, handle, infoSubType));

        return value;
    }

    // Misc.

    hk_noreturn Result BreakWithMessage(BreakReason reason, void* arg, size argSize, void* headerSym, void* msgSym);

    inline hk_alwaysinline Result getProcessHandleMesosphere(Handle* out) {
        u64 value = 0;
        Result rc = GetInfo(&value, InfoType_MesosphereCurrentProcess, 0, 0);
        if (rc.succeeded())
            *out = Handle(value);
        return rc;
    }

    inline hk_alwaysinline void clearCache(ptr addr, ptrdiff size) {
        auto* tls = getTLS();
        tls->cacheMaintanenceFlag = true;
#ifdef __aarch64__
        __builtin___clear_cache((char*)addr, (char*)addr + size);
#else // ILP32 userland cannot flush by itself
        Handle process;
        Result rc = getProcessHandleMesosphere(&process);
#ifndef HK_RELEASE
        HK_ABORT_UNLESS_R(rc);
#endif
        FlushProcessDataCache(process, addr, size);
        InvalidateProcessDataCache(process, addr, size);
#endif
        tls->cacheMaintanenceFlag = false;
    }

} // namespace hk::svc
