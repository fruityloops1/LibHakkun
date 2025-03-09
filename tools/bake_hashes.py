import struct
from elftools.elf.elffile import ELFFile
import mmh3

def bake_hashes(filename):
    with open(filename, 'rb') as f:
        elf_data = bytearray(f.read())
        f.seek(0)
        elffile = ELFFile(f)
        
        dynsym = elffile.get_section_by_name('.dynsym')
        dynstr = elffile.get_section_by_name('.dynstr')
        
        if not dynsym or not dynstr:
            raise Exception("Sections not found")
        
        dynstr_offset = dynstr.header.sh_offset
        dynstr_data = bytearray(dynstr.data())
        
        for symbol in dynsym.iter_symbols():
            if symbol.entry.st_value != 0:
                continue
            
            name_offset = symbol.entry.st_name
            if name_offset == 0:
                continue
                
            name = symbol.name
            if not name:
                continue
                
            hash_value = mmh3.hash(name.encode(), seed=0)
            
            hash_bytes = struct.pack('<I', hash_value & 0xFFFFFFFF)
            
            abs_pos = dynstr_offset + name_offset
            
            str_length = len(name)
            
            for i in range(str_length):
                elf_data[abs_pos + i] = 0
                
            for i in range(len(hash_bytes)):
                elf_data[abs_pos + i] = hash_bytes[i]
    
    output_filename = filename + '.baked'
    with open(output_filename, 'wb') as f:
        f.write(elf_data)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("bake_hashes.py <nss_file>")
        sys.exit(1)
        
    bake_hashes(sys.argv[1])