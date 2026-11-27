"""Copy one root-level file out of a plain Xbox XISO (XDVDFS). Read-only on the ISO.
usage: python -I tools/xbe_tools/xiso_get.py <iso> default.xbe HeroesRecompLib/private/default.xbe
Only reads root-level files of a plain XISO (XDVDFS header at 0x10000). Use Windows paths (D:/...).
"""
import struct, sys

SECTOR = 2048
iso_path, want, out_path = sys.argv[1], sys.argv[2].lower(), sys.argv[3]

with open(iso_path, "rb") as f:
    f.seek(32 * SECTOR)
    vd = f.read(28)
    assert vd[:20] == b"MICROSOFT*XBOX*MEDIA", "not an XDVDFS image"
    root_sector, root_size = struct.unpack_from("<II", vd, 20)
    f.seek(root_sector * SECTOR)
    root = f.read(root_size)

    # Directory = binary tree of entries; offsets are in 4-byte units.
    def find(off):
        while off != 0xFFFF and off * 4 < len(root):
            left, right, sector, size, attr, nlen = struct.unpack_from("<HHIIBB", root, off * 4)
            if left == 0xFFFF and right == 0xFFFF and sector == 0xFFFFFFFF:
                return None
            name = root[off * 4 + 14: off * 4 + 14 + nlen].decode("ascii", "replace")
            c = (want > name.lower()) - (want < name.lower())
            if c == 0:
                return sector, size
            off = left if c < 0 else right
        return None

    hit = find(0)
    if hit is None:
        sys.exit(f"{want} not found in root")
    sector, size = hit
    f.seek(sector * SECTOR)
    data = f.read(size)

with open(out_path, "wb") as o:
    o.write(data)
print(f"{want}: {size} bytes -> {out_path}")
