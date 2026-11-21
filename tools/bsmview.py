#!/usr/bin/env python3
"""Read a .bsm back through tinymarshal and rasterise it (debug/validation aid)."""
import sys
from PIL import Image, ImageDraw
import tinymarshal, svg2bsm

def load(path):
    data = open(path, 'rb').read()
    b = svg2bsm.BSMBuilder(path)
    b, rest = tinymarshal.unpack(data, b)
    assert not rest, "%d trailing bytes" % len(rest)
    return b

def render(b, size=512):
    pts = [(v[0], v[1]) for m in b.meshes for v in m.vertices]
    if not pts: return Image.new('RGB', (size, size), 'white')
    xs, zs = [p[0] for p in pts], [p[1] for p in pts]
    x0, x1, z0, z1 = min(xs), max(xs), min(zs), max(zs)
    sc = (size - 8) / max(x1 - x0, z1 - z0, 1e-9)
    img = Image.new('RGB', (size, size), 'white'); d = ImageDraw.Draw(img)
    P = lambda v: (4 + (v[0] - x0) * sc, 4 + (v[1] - z0) * sc)
    for m in b.meshes:
        for (i, j, k) in m.faces:
            vs = [m.vertices[i], m.vertices[j], m.vertices[k]]
            c = m.colors[i] if m.colors else 0
            rgb = (m.color[0], m.color[1], m.color[2])
            d.polygon([P(v) for v in vs], fill=tuple(int(max(0, min(1, x)) * 255) for x in rgb))
    return img

if __name__ == '__main__':
    b = load(sys.argv[1])
    print("size", b.size, "viewbox", b.viewbox, "meshes", len(b.meshes),
          "verts", sum(len(m.vertices) for m in b.meshes),
          "faces", sum(len(m.faces) for m in b.meshes), "shapes", len(b.shapes), "groups", len(b.groups))
    render(b).save(sys.argv[2])
