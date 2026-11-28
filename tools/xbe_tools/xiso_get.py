"""Copy files out of a plain Xbox XISO (XDVDFS). Read-only on the ISO.
usage:
  python -I tools/xbe_tools/xiso_get.py <iso> default.xbe HeroesRecompLib/private/default.xbe   # one root-level file
  python -I tools/xbe_tools/xiso_get.py <iso> --all HeroesRecompLib/private/disc                  # whole disc
Plain XISO only (XDVDFS header at 0x10000). Use Windows paths (D:/...). The output is game data:
keep it in HeroesRecompLib/private/ (gitignored) and never commit it.
"""
import os, struct, sys

SECTOR = 2048
ATTR_DIRECTORY = 0x10
iso_path, want, out_path = sys.argv[1], sys.argv[2], sys.argv[3]


def read_dir(f, sector, size):
    """All entries of one directory table: (name, sector, size, attr). The table is a binary tree
    (offsets in 4-byte units); walking it fully gives every entry, in any order."""
    if size == 0:
        return []
    f.seek(sector * SECTOR)
    t = f.read(size)
    out, todo, seen = [], [0], set()
    while todo:
        off = todo.pop()
        if off == 0xFFFF or off in seen or off * 4 + 14 > len(t):
            continue
        seen.add(off)
        left, right, sec, sz, attr, nlen = struct.unpack_from("<HHIIBB", t, off * 4)
        if left == 0xFFFF and right == 0xFFFF and sec == 0xFFFFFFFF:
            continue
        name = t[off * 4 + 14: off * 4 + 14 + nlen].decode("ascii", "replace")
        out.append((name, sec, sz, attr))
        todo += [left, right]
    return out


def copy(f, sector, size, dst):
    f.seek(sector * SECTOR)
    with open(dst, "wb") as o:
        left = size
        while left:
            chunk = f.read(min(left, 1 << 22))
            if not chunk:
                raise IOError("unexpected end of ISO in " + dst)
            o.write(chunk)
            left -= len(chunk)


with open(iso_path, "rb") as f:
    f.seek(32 * SECTOR)
    vd = f.read(28)
    assert vd[:20] == b"MICROSOFT*XBOX*MEDIA", "not an XDVDFS image"
    root_sector, root_size = struct.unpack_from("<II", vd, 20)

    if want == "--all":
        files = total = 0
        stack = [(root_sector, root_size, out_path)]
        while stack:
            sec, sz, dst = stack.pop()
            os.makedirs(dst, exist_ok=True)
            for name, s, n, attr in read_dir(f, sec, sz):
                p = os.path.join(dst, name)
                if attr & ATTR_DIRECTORY:
                    stack.append((s, n, p))
                else:
                    copy(f, s, n, p)
                    files += 1
                    total += n
        print(f"{files} files, {total / 2**20:.1f} MiB -> {out_path}")
    else:
        for name, s, n, attr in read_dir(f, root_sector, root_size):
            if name.lower() == want.lower() and not attr & ATTR_DIRECTORY:
                copy(f, s, n, out_path)
                print(f"{name}: {n} bytes -> {out_path}")
                break
        else:
            sys.exit(f"{want} not found in root")
