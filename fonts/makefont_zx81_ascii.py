#! /usr/bin/env python3
import sys
from PIL import Image

im = Image.open("zx81_ascii_font.png")
pixels = im.load()

cnt = {}

def on_pixel(val):
    if val not in cnt:
        cnt[val] = 0
    cnt[val] += 1

def get_byte_for(code,  line):
    x0 = code  % 16 * 9 + 1
    y0 = code // 16 * 9 + 1 + line
    val = 0
    for dx in range(8):
        px = pixels[x0+dx, y0]
        on_pixel(px)
        val = val*2 + (1 if px!=215 else 0)
    return val

out = "static const uint8_t zx81_ascii_font[] = {\n"
out += "\t7, // shift, 2**6 gliphs\n"
for line in range(8):
    for code in range(128):
        out += f'\t{get_byte_for(code, line)},\n'
out += "};\n"

with open("zx81_ascii.h","w") as f:
    f.write(out)

print(cnt)
