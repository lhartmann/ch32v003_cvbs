#! /usr/bin/env python3
import sys
from PIL import Image

im = Image.open("zx81_font.png")
pixels = im.load()

def get_byte_for(code,  line):
    x0 = code%16 * 8
    y0 = code//16 *8 + line
#    print(f'Reading {code}.{line} from {x0},{y0}')
    val = 0
    for dx in range(8):
        val = val*2 + (1 if pixels[x0+dx, y0] else 0)
    return val

out = "static const uint8_t zx81_font[] = {\n"
out += "\t6, // shift, 2**6 gliphs\n"
for line in range(8):
    for code in range(64):
        out += f'\t{get_byte_for(code, line)},\n'
out += "};\n"

with open("zx81.h","w") as f:
    f.write(out)
