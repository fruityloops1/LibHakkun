import sys
import subprocess
import os
import math
import struct
import requests

if len(sys.argv) != 5:
    print("compile_shader.py <uam-bin/glslc> <vertex-source> <fragment-source> <bin-file>")
    print("pass 'glslc' into uam-bin arg to use glslc API")
    exit(1)

uam_bin = sys.argv[1]
vertex_path = sys.argv[2]
frag_path = sys.argv[3]
out_file = sys.argv[4]

def readBinaryFile(path: str):
    with open(path, 'rb') as f:
        return f.read()
def alignUp(value, alignment):
    return (value + (alignment - 1)) & ~(alignment - 1)

glslc_api = "https://glslc.littun.co"

def compileGlslc(srcFile: str, stage: str, part: str):
    url = f"{glslc_api}/{stage}/{part}"
    print(url)
    res = requests.post(url, data=open(srcFile, 'rb').read())
    if (res.status_code == 200):
        return res.content
    else:
        print(res.content.decode('utf-8'))
        exit(1)

if uam_bin == 'glslc':
    vertCtrl = compileGlslc(vertex_path, 'compileVert', 'control')
    fragCtrl = compileGlslc(frag_path, 'compileFrag', 'control')
    vertCode = compileGlslc(vertex_path, 'compileVert', 'code')
    fragCode = compileGlslc(frag_path, 'compileFrag', 'code')
else:
    vertexCtrlPath = ".vertex_ctrl.bin"
    fragCtrlPath = ".frag_ctrl.bin"
    vertexCodePath = ".vertex_code.bin"
    fragCodePath = ".frag_code.bin"

    subprocess.check_call([uam_bin, '--glslcbinds', f'--nvnctrl={vertexCtrlPath}', f'--nvngpu={vertexCodePath}', vertex_path, '--stage', 'vert'])
    subprocess.check_call([uam_bin, '--glslcbinds', f'--nvnctrl={fragCtrlPath}', f'--nvngpu={fragCodePath}', frag_path, '--stage', 'frag'])

    vertCtrl = readBinaryFile(vertexCtrlPath)
    fragCtrl = readBinaryFile(fragCtrlPath)
    vertCode = readBinaryFile(vertexCodePath)
    fragCode = readBinaryFile(fragCodePath)

fragCtrlOffs = 0x10
vertCtrlOffs = alignUp(fragCtrlOffs + len(fragCtrl), 0x100)
fragCodeOffs = alignUp(vertCtrlOffs + len(vertCtrl), 0x100)
vertCodeOffs = alignUp(fragCodeOffs + len(fragCode), 0x100)

with open(out_file, 'wb') as f:
    f.write(struct.pack('IIII', fragCtrlOffs,vertCtrlOffs, fragCodeOffs, vertCodeOffs))
    f.write(fragCtrl)

    while f.tell() != vertCtrlOffs:
        f.write(b'\x00')

    f.write(vertCtrl)

    while f.tell() != fragCodeOffs:
        f.write(b'\x00')
    
    f.write(fragCode)

    while f.tell() != vertCodeOffs:
        f.write(b'\x00')
    
    f.write(vertCode)

if uam_bin != 'glslc':
    os.remove(vertexCtrlPath)
    os.remove(fragCtrlPath)
    os.remove(vertexCodePath)
    os.remove(fragCodePath)
