#!/usr/bin/env python3

import sys
import struct
import hashlib
import lz4.block

from nxo import NroHeader
from elftools.elf.elffile import ELFFile
from elftools.common.exceptions import ELFError, ELFParseError

def fatal(msg):
    print(msg)
    sys.exit(1)

def main(argc, argv):
    # Check arguments
    if argc < 3:
        fatal('Usage: %s <input elf> <output nro> [flags]' % argv[0])

    is_homebrew  = '-h' in argv[3:]

    # Load ELF
    try:
        elf = ELFFile(open(argv[1], 'rb'))
    except IOError:
        fatal('Opening ELF for reading failed!')
    except (ELFError, ELFParseError):
        fatal('Reading ELF failed!')

    # Check architecture
    if elf.get_machine_arch() != 'ARM' and elf.get_machine_arch() != 'AArch64':
        fatal('Only ARM and AArch64 ELFs are supported! Input arch: %s' % elf.get_machine_arch())

    # Check segments
    pt_load_segments = []
    
    for section in elf.iter_segments():
        if section.header.p_type == 'PT_LOAD':
            pt_load_segments.append(section)

    if len(pt_load_segments) != 3:
        fatal('Invalid ELF: Expected 3 loadable segments! Got %d' % len(pt_load_segments))

    text_segment = pt_load_segments[0]
    rodata_segment = pt_load_segments[1]
    data_segment = pt_load_segments[2]

    # Open output file
    try:
        out = open(argv[2], 'wb')
    except IOError:
        fatal('Opening NRO for writing failed!')

    # Create NRO
    header = NroHeader()

    # Module is alyways a single zeroed byte directly after the header
    elf.stream.seek(text_segment.header.p_offset)
    header.load(elf.stream.read(header.size))
    out.seek(header.size, 0)

    # text segment
    header.text_segment.offset = out.tell() - header.size
    header.text_segment.segment_size = (text_segment.header.p_filesz + header.size + 0xFFF) & ~0xFFF

    elf.stream.seek(text_segment.header.p_offset + header.size)
    text_segment_data = elf.stream.read(text_segment.header.p_filesz - header.size)

    out.write(text_segment_data)
    out.write(b'\0' * (header.text_segment.segment_size - text_segment.header.p_filesz))

    # rodata segment
    header.rodata_segment.offset = out.tell()
    header.rodata_segment.segment_size = (rodata_segment.header.p_filesz + 0xFFF) & ~0xFFF

    elf.stream.seek(rodata_segment.header.p_offset)
    rodata_segment_data = elf.stream.read(rodata_segment.header.p_filesz)
  
    out.write(rodata_segment_data)
    out.write(b'\0' * (header.rodata_segment.segment_size - rodata_segment.header.p_filesz))

    # data segment
    header.data_segment.offset = out.tell()
    header.data_segment.segment_size = (data_segment.header.p_filesz + 0xFFF) & ~0xFFF

    elf.stream.seek(data_segment.header.p_offset)
    data_segment_data = elf.stream.read(data_segment.header.p_filesz)

    out.write(data_segment_data)
    out.write(b'\0' * (header.data_segment.segment_size - data_segment.header.p_filesz))

    # bss size
    header.bss_size = data_segment.header.p_memsz - data_segment.header.p_filesz
    header.file_size = out.tell()

    if is_homebrew:
        out.write(struct.pack('<II2Q2Q2Q',
                              0x54455341, 0,
                              0, 0,
                              0, 0,
                              0, 0
                              ))

    # Write Header
    out.seek(0, 0)
    out.write(header.save())
    out.close()

    sys.exit(0)

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
