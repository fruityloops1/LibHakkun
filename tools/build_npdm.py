#!/usr/bin/env python3

# Copyright (c) 2025, SenobiNX
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import argparse
import json
import sys
import typing
import enum
import io
import struct

def abort(msg: str):
    print(f"error: {msg}", file=sys.stderr)
    sys.exit(1)

def abort_unless(cond: bool, msg: str):
    if not cond:
        abort(msg)

def round_up(x: int, alignment: int) -> int:
    # TODO: abort if Not power of 2
    return (x + alignment - 1) & ~(alignment - 1)

class ByteOrder(enum.Enum):
    little = "<"
    big = ">"

# BinaryWriter

class BinaryWriter:
    def __init__(self, size: int = 0, byte_order: ByteOrder = ByteOrder.little):
        self.stream = bytearray(size)
        self.byte_order = byte_order
        self._position = 0

    @property
    def position(self) -> int:
        return self._position

    def save(self, filename: str):
        with open(filename, "wb") as f:
            f.write(self.stream)
    
    def write_sub(self, other: typing.Self):
        self.write_bytes(other.stream)

    def seek(self, offset: int, *, relative: bool = False):
        if relative:
            self._position += offset
        else:
            self._position = offset
        
        self._fill_bytes(0)
    
    def seek_rel(self, offset: int):
        self.seek(offset, relative=True)

    def align(self, alignment: int):
        self.seek(round_up(self.position, alignment))

    def _fill_bytes(self, offset: int, relative: bool = True):
        bytes_to_add = offset - len(self.stream)
        if relative:
            bytes_to_add += self.position

        if bytes_to_add > 0:
            self.stream += bytearray(bytes_to_add)

    def write(self, raw: bytes) -> int:
        start = self._position
        self._fill_bytes(len(raw))
        for byte in raw:
            self.stream[self._position] = byte
            self._position += 1
        return self._position - start

    def _write(self, fmt: str, value) -> int:
        endianness = self.byte_order.value
        raw = struct.pack(endianness + fmt, value)
        return self.write(raw)

    def write_bool(self, value: bool) -> int:
        return self._write("?", value)

    def write_s8(self, value: int) -> int:
        return self._write("b", value)

    def write_u8(self, value: int) -> int:
        return self._write("B", value)

    def write_s16(self, value: int) -> int:
        return self._write("h", value)

    def write_u16(self, value: int) -> int:
        return self._write("H", value)

    def write_u24(self, value: int) -> int:
        if self.byte_order == ByteOrder.little:
            return self.write(struct.pack("<I", value)[:3])
        else:
            return self.write(struct.pack(">I", value)[1:])

    def write_s32(self, value: int) -> int:
        return self._write("i", value)

    def write_u32(self, value: int) -> int:
        return self._write("I", value)

    def write_s64(self, value: int) -> int:
        return self._write("q", value)

    def write_u64(self, value: int) -> int:
        return self._write("Q", value)

    def write_f32(self, value: float) -> int:
        return self._write("f", value)

    def write_f64(self, value: float) -> int:
        return self._write("d", value)

    def write_bytes(self, value: bytes) -> int:
        return self.write(value)
    
    def write_string(self, value: str, *, max_len: int = -1) -> int:
        if max_len > 0:
            value = value[:max_len]
            return self.write("{:\0<{max_len}}".format(value, max_len=max_len).encode("ascii"))
        else:
            return self.write(value.encode("ascii"))


def write_u8(fp: io.BufferedWriter, value: int):
    fp.write(struct.pack("<B", value))

def write_u16(fp: io.BufferedWriter, value: int):
    fp.write(struct.pack("<H", value))

def write_u32(fp: io.BufferedWriter, value: int):
    fp.write(struct.pack("<I", value))

def write_u64(fp: io.BufferedWriter, value: int):
    fp.write(struct.pack("<Q", value))

def write_bytes(fp: io.BufferedWriter, value: bytes):
    fp.write(value)

def write_string(fp: io.BufferedWriter, value: str, *, max_len: int = -1):
    if max_len > 0:
        value = value[:max_len]
        fp.write("{:\0<{max_len}}".format(value, max_len=max_len).encode("ascii"))
    else:
        fp.write(value.encode("ascii"))

def align(fp: io.BufferedWriter, alignment: int):
    fp.seek(round_up(fp.tell(), alignment), io.SEEK_CUR)

# JSON

def json_read_value(json: dict, keys: str, default: typing.Any) -> (str, typing.Any):
    if isinstance(keys, str):
        keys = (keys,)
    
    for key in keys:
        if key in json:
            return (key, json[key])
    
    if default is not None:
        return (key[0], default)
    abort(f"couldn't find key `{key[0]}`")

def json_read_dict(json: dict, key: str, default: dict = None) -> dict:
    key, data = json_read_value(json, key, default)
    abort_unless(isinstance(data, dict), f"`{key}` must be a dict")
    return data

def json_read_list(json: dict, key: str, default: list = None) -> list:
    key, data = json_read_value(json, key, default)
    abort_unless(isinstance(data, list), f"`{key}` must be a list")
    return data

def json_read_bool(json: dict, key: str, default: bool = None) -> bool:
    key, data = json_read_value(json, key, default)
    abort_unless(isinstance(data, bool), f"`{key}` must be a boolean")
    return data

def json_read_str(json: dict, key: str, max_len: int = -1, default: str = None) -> str:
    key, data = json_read_value(json, key, default)
    abort_unless(isinstance(data, str), f"`{key}` must be a string")
    abort_unless(max_len <= 0 or len(data) <= max_len, f"string `{key}` must be less than {max_len} in length")
    return data

def json_read_int(json: dict, key: str, min_val: int, max_val: int, default: int = None) -> int:
    key, data = json_read_value(json, key, default)
    if isinstance(data, int):
        val = data
    elif isinstance(data, str):
        val = int(data, 16)
    else:
        abort(isinstance(data, (int, str)), f"`{key}` must be an integer")

    abort_unless(min_val <= val <= max_val, f"`{key}` must be between {min_val:#x} and {max_val:#x}")
    return val

def json_read_u64(json: dict, key: str, default: int = None) -> int:
    return json_read_int(json, key, 0, (1 << 64) - 1, default)

def json_read_u32(json: dict, key: str, default: int = None) -> int:
    return json_read_int(json, key, 0, (1 << 32) - 1, default)

def json_read_u16(json: dict, key: str, default: int = None) -> int:
    return json_read_int(json, key, 0, (1 << 16) - 1, default)

def json_read_u8(json: dict, key: str, default: int = None) -> int:
    return json_read_int(json, key, 0, (1 << 8) - 1, default)

# NPDM

def write_kc(contents: dict) -> BinaryWriter:
    writer = BinaryWriter()
    kernel_caps = json_read_list(contents, "kernel_capabilities")
    for cap_idx, cap in enumerate(kernel_caps):
        abort_unless(isinstance(cap, dict), "kernel capabilities must be dicts")
        abort_unless(cap_idx < 32, "too many kernel capabilities (max = 32)")
        
        type_ = json_read_str(cap, "type")
        if type_ == "kernel_flags":
            value = json_read_dict(cap, "value")
            cap = (1 << 3) - 1
            cap |= json_read_int(value, "highest_thread_priority", 0, 63) << 4
            cap |= json_read_int(value, "lowest_thread_priority", 0, 63) << 10
            cap |= json_read_u8(value, "lowest_cpu_id") << 16
            cap |= json_read_u8(value, "highest_cpu_id") << 24
            writer.write_u32(cap)
        elif type_ == "syscalls":
            value = json_read_dict(cap, "value")
            groups = [0] * 8
            for name, data in value.items():
                if isinstance(data, int):
                    val = data
                elif isinstance(data, str):
                    val = int(data, 16)
                else:
                    abort(isinstance(data, (int, str)), f"syscalls must be integers")

                abort_unless(0 <= val <= 0xbf, "syscall values must be between 0 and 0xbf")
                groups[val // 24] |= 1 << (val % 24)
            
            for idx, group in enumerate(groups):
                if group:
                    cap = (1 << 4) - 1
                    cap |= group << 5
                    cap |= idx << 29
                    writer.write_u32(cap)
        elif type_ == "map":
            value = json_read_dict(cap, "value")

            cap = (1 << 6) - 1
            cap |= json_read_int(value, "address", 0, (1 << 24) - 1) << 7
            cap |= (1 << 31) if json_read_bool(value, "is_ro") else 0
            writer.write_u32(cap)

            cap = (1 << 6) - 1
            cap |= json_read_int(value, "size", 0, (1 << 20) - 1) << 7
            cap |= (1 << 31) if json_read_bool(value, "is_io") else 0
            writer.write_u32(cap)
        elif type_ == "map_page":
            cap = (1 << 7) - 1
            cap |= json_read_int(cap, "value", 0, (1 << 24) - 1) << 8
            writer.write_u32(cap)
        elif type_ == "map_region":
            value = json_read_list(cap, "value")
            abort_unless(len(value) <= 3, "`map_region` can have a maximum of 3 regions")
            cap = (1 << 10) - 1
            for i, region in enumerate(value):
                abort_unless(isinstance(region, dict), "`map_region` entries must be dicts")
                cap |= json_read_int(region, "region_type", 0, 3) << (11 + 7 * i)
                cap |= (1 << (17 + 7 * i)) if json_read_bool(region, "is_ro") else 0
            
            writer.write_u32(cap)
        elif type_ == "irq_pair":
            value = json_read_list(cap, "value")
            abort_unless(len(value) == 2, "`irq_pair` must contain 2 elements")
            cap = (1 << 11) - 1
            for i, irq in enumerate(value):
                if irq is None:
                    irq_value = 0x3ff
                else:
                    if isinstance(data, int):
                        irq_value = data
                    elif isinstance(data, str):
                        irq_value = int(data, 16)
                    else:
                        abort(isinstance(data, (int, str)), f"`irq_pair` values must be a integers")

                    abort_unless(0 <= val <= (1 << 10) - 1, f"`irq_pair` values must be between {0:#x} and {(1 << 10) - 1:#x}")

                cap |= irq_value << (11 + i * 10)

            writer.write_u32(cap)
        elif type_ == "application_type":
            value = json_read_int(cap, "value", 0, 2, 0)
            cap = (1 << 13) - 1
            cap |= value << 14
            writer.write_u32(cap)
        elif type_ == "min_kernel_version":
            value = json_read_u16(cap, "value")
            cap = (1 << 14) - 1
            cap |= value << 15
            writer.write_u32(cap)
        elif type_ == "handle_table_size":
            value = json_read_int(cap, "value", 0, (1 << 10) - 1)
            cap = (1 << 15) - 1
            cap |= value << 16
            writer.write_u32(cap)
        elif type_ == "debug_flags":
            value = json_read_dict(cap, "value")
            allow_debug = json_read_bool(value, "allow_debug", False)
            force_debug = json_read_bool(value, "force_debug", False)
            force_debug_prod = json_read_bool(value, "force_debug_prod", False)
            abort_unless(
                allow_debug + force_debug + force_debug_prod <= 1,
                "only one of `allow_debug`, `force_debug`, or `force_debug_prod` can be set"
            )

            cap = (1 << 16) - 1
            cap |= (1 << 17) if allow_debug else 0
            cap |= (1 << 18) if force_debug_prod else 0
            cap |= (1 << 19) if force_debug else 0
            writer.write_u32(cap)
        else:
            abort(f"unrecognised kernel capability type `{type_}`")
    
    return writer

def write_sac(contents: dict) -> BinaryWriter:
    writer = BinaryWriter()
    service_host = json_read_list(contents, "service_host")
    service_access = json_read_list(contents, "service_access")
    for service in service_host:
        abort_unless(isinstance(service, str), "services must be strings")
        abort_unless(1 <= len(service) <= 8, "services must be between 1 and 8 chars long")

        writer.write_u8(0x80 | (len(service) - 1))
        writer.write_string(service)
    
    for service in service_access:
        abort_unless(isinstance(service, str), "services must be strings")
        abort_unless(1 <= len(service) <= 8, "services must be between 1 and 8 chars long")
        
        writer.write_u8(len(service) - 1)
        writer.write_string(service)
    
    return writer


def write_acid(contents: dict, sac_writer: BinaryWriter, kc_writer: BinaryWriter) -> BinaryWriter:
    writer = BinaryWriter()

    is_ounce = json_read_str(contents, "platform", -1, "nx").lower() == "ounce"

    rsa_size = 0x180 if is_ounce else 0x100

    writer.write_bytes(b"\x00"*rsa_size) # RSA signature
    writer.write_bytes(b"\x00"*rsa_size) # RSA public key

    writer.write_string("ACID")
    writer.seek_rel(4) # skip size for now
    writer.seek_rel(4) # TODO: skipped version and unknown 0x209 thingy
    
    acid_flags = 0
    acid_flags |= 0b00000001 if json_read_bool(contents, "is_retail") else 0
    acid_flags |= 0b00000010 if json_read_bool(contents, "unqualified_approval", False) else 0
    acid_flags |= json_read_int(contents, "pool_partition", 0, 3) << 2
    writer.write_u32(acid_flags)
    writer.write_u64(json_read_u64(contents, ("program_id_range_min", "title_id_range_min")))
    writer.write_u64(json_read_u64(contents, ("program_id_range_max", "title_id_range_max")))
    writer.seek_rel(0x20)
    
    # ACID - Filesystem Access Control

    fs_access = json_read_dict(contents, "filesystem_access")
    fs_permissions = json_read_u64(fs_access, "permissions")

    fac_offset = writer.position
    writer.write_u8(1) # version
    writer.write_u8(0) # content owner ID count
    writer.write_u8(0) # save data owner ID count
    writer.seek(fac_offset + 4)
    writer.write_u64(fs_permissions)
    writer.write_u64(0) # content owner ID min
    writer.write_u64(0) # content owner ID max
    writer.write_u64(0) # save data owner ID min
    writer.write_u64(0) # save data owner ID max
    fac_size = writer.position - fac_offset

    # ACID - Service Access Control

    writer.align(0x10)
    sac_offset = writer.position
    sac_size = len(sac_writer.stream)
    writer.seek(sac_offset)
    writer.write_sub(sac_writer)

    # ACID - Kernel Capabilities

    writer.align(0x10)
    kc_offset = writer.position
    kc_size = len(kc_writer.stream)
    writer.seek(kc_offset)
    writer.write_sub(kc_writer)

    acid_size = writer.position

    writer.seek(0x304 if is_ounce else 0x204)
    writer.write_u32(acid_size - 0x100)
    writer.seek_rel(0x18)
    writer.write_u32(fac_offset)
    writer.write_u32(fac_size)
    writer.write_u32(sac_offset)
    writer.write_u32(sac_size)
    writer.write_u32(kc_offset)
    writer.write_u32(kc_size)

    return writer


def write_aci(contents: dict, sac_writer: BinaryWriter, kc_writer: BinaryWriter) -> BinaryWriter:
    writer = BinaryWriter()

    fs_access = json_read_dict(contents, "filesystem_access")
    content_owner_ids = json_read_list(fs_access, "content_owner_ids", [])
    save_data_owner_ids = json_read_list(fs_access, "save_data_owner_ids", [])
    fs_permissions = json_read_u64(fs_access, "permissions")

    writer.write_string("ACI0")
    writer.seek_rel(0xc) # reserved
    writer.write_u64(json_read_u64(contents, ("program_id", "title_id")))
    writer.seek_rel(0x8) # reserved
    writer.seek_rel(0x20) # skip over offsets and sizes for now

    # ACI - Filesystem Access Header

    fah_offset = writer.position
    writer.write_u32(1) # version
    writer.write_u64(fs_permissions)
    writer.seek_rel(0x10) # skip over coi/sdoi offsets + sizes for now
    
    coi_offset = writer.position
    if len(content_owner_ids):
        writer.write_u32(len(content_owner_ids))
    for coi in content_owner_ids:
        if isinstance(coi, int):
            val = coi
        elif isinstance(coi, str):
            val = int(coi, 16)
        else:
            abort(isinstance(coi, (int, str)), f"`content_owner_ids` entries must be integers")
        abort_unless(0 <= val <= (1 << 64) - 1, f"`content_owner_ids` entries must be between 0 and {(1 << 64) - 1:#x}")

        writer.write_u64(val)
    coi_size = writer.position - coi_offset
 
    sdoi_offset = writer.position
    if len(save_data_owner_ids):
        writer.write_u32(len(save_data_owner_ids))
    sdoi_accessibilities = []
    sdoi_ids = []
    for sdoi in save_data_owner_ids:
        abort_unless(isinstance(sdoi, dict), "`save_data_owner_ids` entries must be dicts")
        sdoi_accessibilities.append(json_read_int(sdoi, "accessibility", 1, 3))
        sdoi_ids.append(json_read_u64(sdoi, "id"))
    
    for accessibility in sdoi_accessibilities:
        writer.write_u8(accessibility)
    writer.align(4)
    for id_ in sdoi_ids:
        writer.write_u64(id_)
    sdoi_size = writer.position - sdoi_offset

    fah_size = writer.position - fah_offset
    writer.seek(fah_offset + 0xc)
    writer.write_u32(coi_offset - fah_offset) # content owner IDs offset
    writer.write_u32(coi_size) # content owner IDs size
    writer.write_u32(sdoi_offset - fah_offset) # save data owner IDs offset
    writer.write_u32(sdoi_size) # save data owner IDs size

    # ACI - Service Access Control

    writer.seek(fah_offset + fah_size)
    writer.align(0x10)
    sac_offset = writer.position
    writer.write_sub(sac_writer)

    # ACI - Kernel Capabilities

    writer.align(0x10)
    kc_offset = writer.position
    writer.write_sub(kc_writer)

    aci_size = writer.position

    writer.seek(0x20)
    writer.write_u32(fah_offset)
    writer.write_u32(fah_size)
    writer.write_u32(sac_offset)
    writer.write_u32(len(sac_writer.stream))
    writer.write_u32(kc_offset)
    writer.write_u32(len(kc_writer.stream))

    return writer


def write_meta(contents: dict) -> BinaryWriter:
    writer = BinaryWriter()

    writer.write_string("META")
    writer.write_u32(json_read_u32(contents, "signature_key_generation", 0))
    writer.seek_rel(0x4) # reserved

    cap = 0
    cap |= 0b00000001 if json_read_bool(contents, "is_64_bit") else 0
    cap |= json_read_int(contents, "address_space_type", 0, 3) << 1
    cap |= 0b00010000 if json_read_bool(contents, "optimize_memory_allocation", False) else 0
    cap |= 0b00100000 if json_read_bool(contents, "disable_device_address_space_merge", False) else 0
    cap |= 0b01000000 if json_read_bool(contents, "enable_alias_region_extra_size", False) else 0
    cap |= 0b10000000 if json_read_bool(contents, "prevent_code_reads", False) else 0
    writer.write_u8(cap)
    writer.seek(0xe)
    writer.write_u8(json_read_int(contents, "main_thread_priority", 0, 0x3f))
    writer.write_u8(json_read_u8(contents, "default_cpu_id"))
    writer.write_u32(json_read_int(contents, "remote_memory_size", 0, 0x1fe00000, 0))
    writer.write_u32(json_read_int(contents, "system_resource_size", 0, 0x1fe00000, 0))
    writer.write_u32(json_read_u32(contents, "version", 0))

    main_thread_stack_size = json_read_u32(contents, "main_thread_stack_size")
    abort_unless(main_thread_stack_size & 0xfff == 0, "`main_thread_stack_size` must be aligned to 0x1000")
    writer.write_u32(main_thread_stack_size)

    name = json_read_str(contents, "name", max_len=0x10)
    writer.write_string(name, max_len=0x10)
    writer.write_bytes(b"\0"*16) # product code
    writer.seek_rel(0x30) # reserved
    writer.seek_rel(0x10) # skip ACI/ACID offsets + sizes for now

    return writer


def main():
    parser = argparse.ArgumentParser(description="generate NPDM file from JSON")
    parser.add_argument("infile")
    parser.add_argument("outfile", nargs="?")
    # parser.add_argument("-q", "--quiet", action="store_true")

    args = parser.parse_args()

    with open(args.infile) as f:
        contents = json.load(f)
    
    writer = BinaryWriter()

    sac_writer = write_sac(contents)
    kc_writer = write_kc(contents)

    # META section

    meta_writer = write_meta(contents)
    meta_size = len(meta_writer.stream)
    writer.write_sub(meta_writer)

    # ACID section

    writer.seek(meta_size)
    writer.align(0x10)
    acid_offset = writer.position
    acid_writer = write_acid(contents, sac_writer, kc_writer)
    acid_size = len(acid_writer.stream)
    writer.write_sub(acid_writer)

    # ACI section

    writer.align(0x10)
    aci_offset = writer.position
    aci_writer = write_aci(contents, sac_writer, kc_writer)
    aci_size = len(aci_writer.stream)
    writer.write_sub(aci_writer)

    # write ACI/ACID offsets + size into META

    writer.seek(0x70)
    writer.write_u32(aci_offset)
    writer.write_u32(aci_size)
    writer.write_u32(acid_offset)
    writer.write_u32(acid_size)

    writer.save(args.outfile)

if __name__ == "__main__":
    main()
