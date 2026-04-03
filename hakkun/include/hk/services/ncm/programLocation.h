#pragma once

#include "hk/types.h"

namespace hk::ncm {
    enum class StorageId : u8 {
        None = 0,
        Host = 1,
        GameCard = 2,
        BuiltInSystem = 3,
        BuiltInUser = 4,
        SdCard = 5,
        Any = 6,
    };

    struct ProgramLocation {
        constexpr ProgramLocation(u64 programId, StorageId storageId)
            : programId(programId)
            , storageId(storageId) { }

        constexpr ProgramLocation(u64 programId)
            : programId(programId)
            , storageId(StorageId::None) { }

        u64 programId;
        StorageId storageId;

        constexpr ProgramLocation withStorage(StorageId storageId) const {
            return ProgramLocation(programId, storageId);
        }
    };

    static constexpr ProgramLocation fs = ProgramLocation(0x0100000000000000);
    static constexpr ProgramLocation loader = ProgramLocation(0x0100000000000001);
    static constexpr ProgramLocation ncm = ProgramLocation(0x0100000000000002);
    static constexpr ProgramLocation pm = ProgramLocation(0x0100000000000003);
    static constexpr ProgramLocation sm = ProgramLocation(0x0100000000000004);
    static constexpr ProgramLocation boot = ProgramLocation(0x0100000000000005);
    static constexpr ProgramLocation usb = ProgramLocation(0x0100000000000006);
    static constexpr ProgramLocation tma = ProgramLocation(0x0100000000000007);
    static constexpr ProgramLocation boot2 = ProgramLocation(0x0100000000000008);
    static constexpr ProgramLocation settings = ProgramLocation(0x0100000000000009);
    static constexpr ProgramLocation bus = ProgramLocation(0x010000000000000A);
    static constexpr ProgramLocation bluetooth = ProgramLocation(0x010000000000000B);
    static constexpr ProgramLocation bcat = ProgramLocation(0x010000000000000C);
    static constexpr ProgramLocation dmnt = ProgramLocation(0x010000000000000D);
    static constexpr ProgramLocation friends = ProgramLocation(0x010000000000000E);
    static constexpr ProgramLocation nifm = ProgramLocation(0x010000000000000F);
    static constexpr ProgramLocation ptm = ProgramLocation(0x0100000000000010);
    static constexpr ProgramLocation shell = ProgramLocation(0x0100000000000011);
    static constexpr ProgramLocation bsdSockets = ProgramLocation(0x0100000000000012);
    static constexpr ProgramLocation hid = ProgramLocation(0x0100000000000013);
    static constexpr ProgramLocation audio = ProgramLocation(0x0100000000000014);
    static constexpr ProgramLocation logManager = ProgramLocation(0x0100000000000015);
    static constexpr ProgramLocation wlan = ProgramLocation(0x0100000000000016);
    static constexpr ProgramLocation cs = ProgramLocation(0x0100000000000017);
    static constexpr ProgramLocation ldn = ProgramLocation(0x0100000000000018);
    static constexpr ProgramLocation nvServices = ProgramLocation(0x0100000000000019);
    static constexpr ProgramLocation pcv = ProgramLocation(0x010000000000001A);
    static constexpr ProgramLocation ppc = ProgramLocation(0x010000000000001B);
    static constexpr ProgramLocation nvnFlinger = ProgramLocation(0x010000000000001C);
    static constexpr ProgramLocation pcie = ProgramLocation(0x010000000000001D);
    static constexpr ProgramLocation account = ProgramLocation(0x010000000000001E);
    static constexpr ProgramLocation ns = ProgramLocation(0x010000000000001F);
    static constexpr ProgramLocation nfc = ProgramLocation(0x0100000000000020);
    static constexpr ProgramLocation psc = ProgramLocation(0x0100000000000021);
    static constexpr ProgramLocation capSrv = ProgramLocation(0x0100000000000022);
    static constexpr ProgramLocation am = ProgramLocation(0x0100000000000023);
    static constexpr ProgramLocation ssl = ProgramLocation(0x0100000000000024);
    static constexpr ProgramLocation nim = ProgramLocation(0x0100000000000025);
    static constexpr ProgramLocation cec = ProgramLocation(0x0100000000000026);
    static constexpr ProgramLocation tspm = ProgramLocation(0x0100000000000027);
    static constexpr ProgramLocation spl = ProgramLocation(0x0100000000000028);
    static constexpr ProgramLocation lbl = ProgramLocation(0x0100000000000029);
    static constexpr ProgramLocation btm = ProgramLocation(0x010000000000002A);
    static constexpr ProgramLocation erpt = ProgramLocation(0x010000000000002B);
    static constexpr ProgramLocation time = ProgramLocation(0x010000000000002C);
    static constexpr ProgramLocation vi = ProgramLocation(0x010000000000002D);
    static constexpr ProgramLocation pctl = ProgramLocation(0x010000000000002E);
    static constexpr ProgramLocation npns = ProgramLocation(0x010000000000002F);
    static constexpr ProgramLocation eupld = ProgramLocation(0x0100000000000030);
    static constexpr ProgramLocation arp = ProgramLocation(0x0100000000000031);
    static constexpr ProgramLocation glue = ProgramLocation(0x0100000000000031);
    static constexpr ProgramLocation eclct = ProgramLocation(0x0100000000000032);
    static constexpr ProgramLocation es = ProgramLocation(0x0100000000000033);
    static constexpr ProgramLocation fatal = ProgramLocation(0x0100000000000034);
    static constexpr ProgramLocation grc = ProgramLocation(0x0100000000000035);
    static constexpr ProgramLocation creport = ProgramLocation(0x0100000000000036);
    static constexpr ProgramLocation ro = ProgramLocation(0x0100000000000037);
    static constexpr ProgramLocation profiler = ProgramLocation(0x0100000000000038);
    static constexpr ProgramLocation sdb = ProgramLocation(0x0100000000000039);
    static constexpr ProgramLocation migration = ProgramLocation(0x010000000000003A);
    static constexpr ProgramLocation jit = ProgramLocation(0x010000000000003B);
    static constexpr ProgramLocation jpegDec = ProgramLocation(0x010000000000003C);
    static constexpr ProgramLocation safeMode = ProgramLocation(0x010000000000003D);
    static constexpr ProgramLocation olsc = ProgramLocation(0x010000000000003E);
    static constexpr ProgramLocation dt = ProgramLocation(0x010000000000003F);
    static constexpr ProgramLocation nd = ProgramLocation(0x0100000000000040);
    static constexpr ProgramLocation ngct = ProgramLocation(0x0100000000000041);
    static constexpr ProgramLocation pgl = ProgramLocation(0x0100000000000042);
    static constexpr ProgramLocation omm = ProgramLocation(0x0100000000000045);
    static constexpr ProgramLocation eth = ProgramLocation(0x0100000000000046);
    static constexpr ProgramLocation ngc = ProgramLocation(0x0100000000000050);
    static constexpr ProgramLocation end = ProgramLocation(0x01000000000007FF);
    static constexpr ProgramLocation browserCoreDll = ProgramLocation(0x010000000000085D);
    static constexpr ProgramLocation manu = ProgramLocation(0x010000000000B14A);
    static constexpr ProgramLocation htc = ProgramLocation(0x010000000000B240);
    static constexpr ProgramLocation dmntGen2 = ProgramLocation(0x010000000000D609);
    static constexpr ProgramLocation devServer = ProgramLocation(0x010000000000D623);

} // namespace hk::ncm
