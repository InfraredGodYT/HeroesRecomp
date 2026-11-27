"""Print facts about an Xbox XBE: header, certificate, sections, XDK libraries, kernel imports.
usage: python -I tools/xbe_tools/xbe_info.py HeroesRecompLib/private/default.xbe tools/XboxRecomp2/src/xbox-recomp2-xbox/src/xbe/kernel/exports.rs HeroesRecomp
Read-only. Kernel names come from XboxRecomp2's exports.rs; "host" = names registered with
GUEST_FUNCTION_*(__imp__Name in the host sources.
"""
import os, re, struct, sys

xbe_path, exports_rs, host_dir = sys.argv[1:4]
d = open(xbe_path, "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]
assert d[:4] == b"XBEH"
base = u32(0x104)

n_sec, sec_addr = u32(0x11C), u32(0x120)
sections = []
for i in range(n_sec):
    o = sec_addr - base + i * 56
    flags, va, vsz, raw, rsz, name_addr = struct.unpack_from("<6I", d, o)
    name = d[name_addr - base:].split(b"\0")[0].decode()
    sections.append((name, va, vsz, raw, rsz, flags))

def va_to_off(va):
    if base <= va < base + u32(0x108):
        return va - base
    for _, sva, vsz, raw, rsz, _ in sections:
        if sva <= va < sva + rsz:
            return raw + (va - sva)
    raise ValueError(hex(va))

def unxor(v, retail, debug):
    r = v ^ retail
    return (r, "retail") if r < 0x80000000 and r >= base else (v ^ debug, "debug")

entry, kind = unxor(u32(0x128), 0xA8FC57AB, 0x94859D4B)
thunk, _ = unxor(u32(0x158), 0x5B6D40B6, 0xEFB1F152)

cert = u32(0x118) - base
title_id = u32(cert + 8)
title = d[cert + 0xC: cert + 0xC + 0x50].decode("utf-16-le").split("\0")[0]
region, disc, version = u32(cert + 0xA0), u32(cert + 0xA8), u32(cert + 0xAC)

print(f"title: {title!r}  title_id: 0x{title_id:08X}  region: 0x{region:X}  disc: {disc}  version: {version}")
print(f"kind: {kind}  base: 0x{base:X}  entry: 0x{entry:X}  kernel thunk: 0x{thunk:X}")
print(f"image size: 0x{u32(0x10C):X}  stack commit: 0x{u32(0x130):X}  heap reserve: 0x{u32(0x134):X}  init flags: 0x{u32(0x124):X}")
print("\nsections (name, va, vsize, raw, rawsize, flags):")
for s in sections:
    print(f"  {s[0]:<10} va 0x{s[1]:08X}  vsize 0x{s[2]:08X}  raw 0x{s[3]:08X}  rsize 0x{s[4]:08X}  flags 0x{s[5]:X}")

print("\nXDK libraries (name major.minor.build, flags):")
n_lib, lib_addr = u32(0x160), u32(0x164)
for i in range(n_lib):
    o = lib_addr - base + i * 16
    name = d[o:o + 8].split(b"\0")[0].decode()
    major, minor, build, fl = struct.unpack_from("<4H", d, o + 8)
    qfe, approved, debug = fl & 0x1FFF, (fl >> 13) & 3, fl >> 15
    print(f"  {name:<9} {major}.{minor}.{build}  qfe {qfe}  approved {approved}  debug {debug}")

names = dict((int(a, 16), b) for a, b in re.findall(r'\((0x[0-9A-Fa-f]+),\s*"([^"]+)"\)', open(exports_rs).read()))
host = set()
for root, _, files in os.walk(host_dir):
    for f in files:
        if f.endswith((".cpp", ".h", ".inl")):
            host |= set(re.findall(r"GUEST_FUNCTION_[A-Z_]+\(__imp__([A-Za-z0-9_]+)", open(os.path.join(root, f), encoding="utf-8", errors="replace").read()))

imports, o = [], va_to_off(thunk)
while (v := u32(o)) != 0:
    imports.append(v & 0x7FFFFFFF if v & 0x80000000 else None)
    o += 4
print(f"\nnon-kernel import directory: 0x{u32(0x15C):X}")
print(f"kernel imports: {len(imports)} ({sum(1 for i in imports if i is not None and names.get(i) in host)} implemented by host)")
missing = [i for i in imports if names.get(i) not in host]
print(f"NOT implemented by host ({len(missing)}):")
for i in missing:
    print(f"  {i:3d} (0x{i:03X}) {names.get(i, '?')}")
print("implemented:", ", ".join(sorted(names.get(i, "?") for i in imports if names.get(i) in host)))
