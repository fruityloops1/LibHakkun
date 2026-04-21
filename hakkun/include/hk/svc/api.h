#pragma once

#include "hk/ValueOrResult.h"
#include "hk/types.h"
#include "hk/util/Tuple.h"

#include "hk/container/Span.h"
#include "hk/svc/cpu.h"

namespace hk::svc {

    ResultNN QueryMemory(MemoryInfo* outMemoryInfo, u32* outPageInfo, ptr address);
    ResultNN MapMemory(ptr dest, ptr source, size size);
    ResultNN UnmapMemory(ptr addr, ptr source, size size);
    hk_noreturn void ExitProcess();
    ResultNN CreateThread(Handle* outHandle, ThreadFunc func, ptr arg, ptr stackTop, s32 priority, s32 coreId);
    ResultNN StartThread(Handle threadHandle);
    hk_noreturn void ExitThread();
    // values 0, -1, and -2 will yield the thread.
    // see https://switchbrew.org/wiki/SVC#SleepThread
    void SleepThread(s64 nanoseconds);
    ResultNN SetThreadCoreMask(Handle handle, s32 preferredCore, u32 mask);
    ResultNN CreateTransferMemory(Handle* outHandle, ptr address, size size, MemoryPermission perm);
    ResultNN CloseHandle(Handle handle);
    ResultNN SignalEvent(Handle handle);
    ResultNN ClearEvent(Handle handle);
    ResultNN WaitSynchronization(s32* outIdx, const Handle* handles, s32 numHandles, s64 timeout);
    ResultNN CancelSynchronization(Handle handle);
    ResultNN ResetSignal(Handle handle);
    ResultNN ArbitrateLock(Handle threadHandle, ptr addr, u32 tag);
    ResultNN ArbitrateUnlock(ptr addr);
    ResultNN ConnectToNamedPort(Handle* outHandle, const char* name);
    ResultNN SendSyncRequestLight(Handle sessionHandle, u8 data[28]);
    ResultNN SendSyncRequest(Handle sessionHandle);
    ResultNN SendSyncRequestWithUserBuffer(ptr buffer, size bufferSize, Handle sessionHandle);
    hk_noreturn ResultNN Break(BreakReason reason, void* arg, size argSize);
    ResultNN OutputDebugString(const char* str, size_t len);
    hk_noreturn void ReturnFromException(Result result);
    ResultNN CreateSession(Handle* outServerHandle, Handle* outClientHandle, bool isLight, ptr name);
    ResultNN AcceptSession(Handle* outSessionHandle, Handle portHandle);
    ResultNN ReplyAndReceiveLight(Handle sessionHandle, u8 data[28]);
    ResultNN ReplyAndReceive(u32* outIndex, const Handle* handles, u32 handleCount, Handle replyHandle, u64 timeout);
    ResultNN CreateEvent(Handle* outWriteHandle, Handle* outReadHandle);
    ResultNN ManageNamedPort(Handle* outHandle, const char* name, s32 maxSessions);
    ResultNN GetInfo(u64* out, InfoType type, Handle handle, u64 subType);
    ResultNN WaitForAddress(const void* address, ArbitrationType type, u32 value, u64 timeout);
    ResultNN SignalToAddress(void* address, SignalType type, u32 value, u64 timeout);
    ResultNN InvalidateProcessDataCache(Handle process, ptr addr, size size);
    ResultNN FlushProcessDataCache(Handle process, ptr addr, size size);
    ResultNN DebugActiveProcess(Handle* outHandle, u64 processId);
    ResultNN BreakDebugProcess(Handle debugHandle);
    ResultNN TerminateDebugProcess(Handle debugHandle);
    ResultNN GetDebugEvent(DebugEventInfo* outInfo, Handle debugHandle);
    ResultNN ContinueDebugEvent(Handle debugHandle, u32 flags, const u64* threadIds, s32 numThreadIds);
    ResultNN GetProcessList(s32* outNumProcesses, u64* outProcessIds, s32 maxProcesses);
    ResultNN QueryDebugProcessMemory(MemoryInfo* outMemoryInfo, u32* outPageInfo, Handle debugHandle, u64 address);
    ResultNN ReadDebugProcessMemory(void* buffer, Handle debugHandle, u64 address, size size);
    ResultNN WriteDebugProcessMemory(Handle debugHandle, const void* buffer, u64 address, size size);
    ResultNN GetSystemInfo(u64* outInfo, SystemInfoType infoType, Handle handle, PhysicalMemorySystemInfo infoSubType);
    ResultNN MapProcessMemory(ptr dest, Handle process, u64 source, size size);
    ResultNN CallSecureMonitor(SecmonArgs args);

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

    inline hk_alwaysinline ResultNN SendSyncRequestWithUserBuffer(Span<u8> userBuffer, Handle sessionHandle) {
        return SendSyncRequestWithUserBuffer(ptr(userBuffer.data()), userBuffer.size_bytes(), sessionHandle);
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

    struct CreatedSession {
        Handle server;
        Handle client;
    };

    inline hk_alwaysinline ValueOrResult<CreatedSession> CreateSession(bool isLight, ptr name) {
        Handle outServerHandle, outClientHandle;

        HK_TRY(CreateSession(&outServerHandle, &outClientHandle, isLight, name));

        return CreatedSession {
            .server = outServerHandle,
            .client = outClientHandle,
        };
    }

    inline hk_alwaysinline ValueOrResult<Handle> AcceptSession(Handle portHandle) {
        Handle outHandle;

        HK_TRY(AcceptSession(&outHandle, portHandle));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<Handle> DebugActiveProcess(u64 processId) {
        Handle outHandle;

        HK_TRY(DebugActiveProcess(&outHandle, processId));

        return outHandle;
    }

    inline hk_alwaysinline ValueOrResult<DebugEventInfo> GetDebugEvent(Handle debugHandle) {
        DebugEventInfo info;

        HK_TRY(GetDebugEvent(&info, debugHandle));

        return info;
    }

    inline hk_alwaysinline ValueOrResult<Tuple<MemoryInfo, u32>> QueryDebugProcessMemory(Handle debugHandle, ptr address) {
        MemoryInfo info;
        u32 pageInfo;

        HK_TRY(QueryDebugProcessMemory(&info, &pageInfo, debugHandle, address));

        return Tuple<MemoryInfo, u32> { info, pageInfo };
    }

    inline hk_alwaysinline ValueOrResult<u64> GetSystemInfo(SystemInfoType infoType, Handle handle, PhysicalMemorySystemInfo infoSubType) {
        u64 value;

        HK_TRY(GetSystemInfo(&value, infoType, handle, infoSubType));

        return value;
    }

    // Misc.

    hk_noreturn ResultNN BreakWithMessage(BreakReason reason, void* arg, size argSize, void* headerSym, void* msgSym);

    inline hk_alwaysinline ValueOrResult<Handle> getProcessHandleMesosphere() {
        u64 value = 0;
        HK_TRY(GetInfo(&value, InfoType_MesosphereCurrentProcess, 0, 0));
        return Handle(value);
    }

    inline hk_alwaysinline void clearCache(ptr addr, ptrdiff size) {
        auto* tls = getTLS();
        tls->cacheMaintanenceFlag = true;
#ifdef __aarch64__
        __builtin___clear_cache((char*)addr, (char*)addr + size);
#else // ILP32 userland cannot flush by itself
        Handle process = HK_UNWRAP(getProcessHandleMesosphere());
        FlushProcessDataCache(process, addr, size);
        InvalidateProcessDataCache(process, addr, size);
#endif
        tls->cacheMaintanenceFlag = false;
    }

} // namespace hk::svc
