# this script Sucks ass and only works (properly) on certain fonts because its Simple and it needs to stay Simple

from PIL import Image, ImageFont, ImageDraw
import sys
import math
import struct
import subprocess

if len(sys.argv) != 5:
    print("generate_font.py <otf-file> <charset> <fontsize> <hkf-file>")
    exit(1)

fontPath = sys.argv[1]
charset = sys.argv[2]
fontSize = int(sys.argv[3])
outFile = sys.argv[4]

font = ImageFont.truetype(fontPath, fontSize)

glyphWidth = 0
glyphHeight = 0

for char in charset:
    size, _ = font.font.getsize(char)
    if size[0] > glyphWidth:
        glyphWidth = size[0]
    if size[1] > glyphHeight:
        glyphHeight = size[1]

texWidth = int(glyphWidth * 32)
texHeight = int(math.ceil(len(charset) / 32)) * glyphHeight

print(str(texWidth) + " " + str(texHeight))

image = Image.new("RGBA", (int(texWidth), int(texHeight)), (255, 255, 255, 0))
draw = ImageDraw.Draw(image)


i = 0
for char in charset:
    x = i % 32
    y = int(i / 32)

    draw.text((x * glyphWidth, y * glyphHeight), char, (255,255,255), font=font, anchor='la')
    i = i + 1


image.save("font.png")

subprocess.run(['astcenc', '-cl', 'font.png', 'font.astc', '6x6', '-exhaustive'])

astcTexture = open('font.astc', 'rb').read()

with open(outFile, "wb") as f:
    f.write(struct.pack('qiiq', len(charset), texWidth, texHeight, len(astcTexture)))
    for char in charset:
        f.write(struct.pack('H', ord(char)))
    f.write(struct.pack('H', 0))
    f.write(astcTexture)
