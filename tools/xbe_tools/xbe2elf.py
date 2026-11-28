"""Convert an Xbox XBE into an ELF32 (i386) that Ghidra loads natively, at the real guest addresses.
usage (repo root):
  python -I tools/xbe_tools/xbe2elf.py <in.xbe> <out.elf> [--funcs x86_func_mapping.cpp] [--names names.txt]
                                       [--exports tools/XboxRecomp2/src/xbox-recomp2-xbox/src/xbe/kernel/exports.rs]
The ELF holds the game's code, so write it to HeroesRecompLib/private/ (gitignored). Never commit it.

What goes in:
  - one PT_LOAD segment + one named section per XBE section (va, size, R/W/X from the XBE flags)
  - the XBE headers as a read-only "xbe_header" segment at the base address
  - symbols: function starts from XboxRecomp2 (sub_XXXX), kernel thunk slots (__imp_<Name>),
    the entry point (xbe_entry), and optional extra names ("0xADDR name" per line, e.g. from match_shadow.py)
"""
import argparse, re, struct

ap = argparse.ArgumentParser()
ap.add_argument("xbe")
ap.add_argument("out")
ap.add_argument("--funcs")
ap.add_argument("--names")
ap.add_argument("--exports", default="tools/XboxRecomp2/src/xbox-recomp2-xbox/src/xbe/kernel/exports.rs")
a = ap.parse_args()

d = open(a.xbe, "rb").read()
u32 = lambda o: struct.unpack_from("<I", d, o)[0]
assert d[:4] == b"XBEH", "not an XBE"
base, hdr_size = u32(0x104), u32(0x108)


def unxor(v, retail, debug):
    r = v ^ retail
    return r if base <= r < 0x80000000 else v ^ debug


entry = unxor(u32(0x128), 0xA8FC57AB, 0x94859D4B)
thunk = unxor(u32(0x158), 0x5B6D40B6, 0xEFB1F152)

secs = []  # (name, va, vsize, raw_off, raw_size, flags)
n, sa = u32(0x11C), u32(0x120) - base
for i in range(n):
    flags, va, vsz, raw, rsz, name_addr = struct.unpack_from("<6I", d, sa + i * 56)
    name = d[name_addr - base:].split(b"\0")[0].decode("ascii", "replace")
    secs.append((name, va, vsz, raw, rsz, flags))
secs.insert(0, ("xbe_header", base, hdr_size, 0, hdr_size, 0))  # read-only, not executable


def va_to_off(va):
    for _, v, vs, r, rs, _ in secs:
        if v <= va < v + rs:
            return r + va - v
    return None


def sec_index(va):
    for i, (_, v, vs, r, rs, _) in enumerate(secs):
        if v <= va < v + max(vs, rs):
            return i + 1  # section 0 is the null section
    return 0


# ---- symbols
syms = {}  # va -> (name, is_func)
if a.funcs:
    for m in re.finditer(r"\{\s*0x([0-9A-Fa-f]+)\s*,", open(a.funcs, encoding="utf-8").read()):
        va = int(m.group(1), 16)
        syms[va] = ("sub_%X" % va, True)
names = dict((int(o, 16), s) for o, s in re.findall(r'\((0x[0-9A-Fa-f]+),\s*"([^"]+)"\)', open(a.exports).read()))
o = va_to_off(thunk)
i = 0
while o is not None and u32(o + 4 * i) != 0:
    v = u32(o + 4 * i)
    ordinal = v & 0x7FFFFFFF if v & 0x80000000 else None
    syms[thunk + 4 * i] = ("__imp_" + names.get(ordinal, "ordinal_%d" % ordinal if ordinal else "unknown"), False)
    i += 1
syms[entry] = ("xbe_entry", True)
if a.names:
    for line in open(a.names, encoding="utf-8"):
        p = line.split()
        if len(p) >= 2 and p[0].lower().startswith("0x"):
            syms[int(p[0], 16)] = (p[1], True)

# ---- layout: ELF header, program headers, section data, string tables, symtab, section headers
EH, PH, SH, SYM = 52, 32, 40, 16
nseg = len(secs)
data_off = EH + PH * nseg
blobs, off = [], data_off
seg = []
for name, va, vsz, raw, rsz, fl in secs:
    off = (off + 0xFFF) & ~0xFFF if rsz else off
    seg.append(off)
    blobs.append((off, d[raw:raw + rsz]))
    off += rsz

shstr = b"\0"
def add_shstr(s):
    global shstr
    i = len(shstr); shstr += s.encode() + b"\0"; return i

strtab = b"\0"
symtab = b"\0" * SYM
for va, (nm, is_func) in sorted(syms.items()):
    idx = sec_index(va)
    if not idx:
        continue
    so = len(strtab)
    strtab += nm.encode() + b"\0"
    info = (1 << 4) | (2 if is_func else 1)  # STB_GLOBAL, STT_FUNC / STT_OBJECT
    symtab += struct.pack("<IIIBBH", so, va, 0, info, 0, idx)

sec_names = [add_shstr(s[0]) for s in secs]
i_symtab, i_strtab, i_shstr = add_shstr(".symtab"), add_shstr(".strtab"), add_shstr(".shstrtab")
off = (off + 3) & ~3
symtab_off = off; off += len(symtab)
strtab_off = off; off += len(strtab)
shstr_off = off; off += len(shstr)
off = (off + 3) & ~3
sh_off = off
nsec = 1 + len(secs) + 3

out = bytearray(sh_off + SH * nsec)
struct.pack_into("<4sBBBB8sHHIIIIIHHHHHH", out, 0, b"\x7fELF", 1, 1, 1, 0, b"\0" * 8,
                 2, 3, 1, entry, EH, sh_off, 0, EH, PH, nseg, SH, nsec, nsec - 1)
for i, (name, va, vsz, raw, rsz, fl) in enumerate(secs):
    pf = 4 | (2 if fl & 0x1 else 0) | (1 if fl & 0x4 else 0)  # R | W(writable) | X(executable)
    struct.pack_into("<8I", out, EH + PH * i, 1, seg[i], va, va, rsz, max(vsz, rsz), pf, 0x1000)
for o2, b in blobs:
    out[o2:o2 + len(b)] = b
out[symtab_off:symtab_off + len(symtab)] = symtab
out[strtab_off:strtab_off + len(strtab)] = strtab
out[shstr_off:shstr_off + len(shstr)] = shstr

def sh(i, name, typ, flags, addr, off_, size, link=0, info=0, align=1, ent=0):
    struct.pack_into("<10I", out, sh_off + SH * i, name, typ, flags, addr, off_, size, link, info, align, ent)

for i, (name, va, vsz, raw, rsz, fl) in enumerate(secs):
    shf = 0x2 | (0x1 if fl & 0x1 else 0) | (0x4 if fl & 0x4 else 0)  # ALLOC | WRITE | EXECINSTR
    typ = 1 if rsz else 8  # PROGBITS / NOBITS
    sh(i + 1, sec_names[i], typ, shf, va, seg[i], max(vsz, rsz) if typ == 8 else rsz, align=4)
j = 1 + len(secs)
sh(j, i_symtab, 2, 0, 0, symtab_off, len(symtab), link=j + 1, info=1, align=4, ent=SYM)
sh(j + 1, i_strtab, 3, 0, 0, strtab_off, len(strtab))
sh(j + 2, i_shstr, 3, 0, 0, shstr_off, len(shstr))
open(a.out, "wb").write(out)
print("%s: %d sections, %d symbols, entry 0x%X -> %s" % (a.xbe, len(secs), len(symtab) // SYM - 1, entry, a.out))
