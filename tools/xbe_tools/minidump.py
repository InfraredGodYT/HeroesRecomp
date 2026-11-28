"""Read a Windows minidump (.dmp) of HeroesRecomp.exe: exception, faulting thread, and a heuristic backtrace.
usage: python -I tools/xbe_tools/minidump.py <file.dmp> <HeroesRecomp.exe> [--symbolizer <llvm-symbolizer.exe>]
Addresses inside HeroesRecomp.exe are turned into function/file:line with llvm-symbolizer (reads the .pdb).
The backtrace scans the crashing thread's stack for values that point into a module's code, so it can show
a few false frames; real frames are usually the ones that form a sensible call chain.
"""
import argparse, struct, subprocess, sys

ap = argparse.ArgumentParser()
ap.add_argument("dmp")
ap.add_argument("exe")
ap.add_argument("--symbolizer", default=r"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\Llvm\x64\bin\llvm-symbolizer.exe")
ap.add_argument("--depth", type=int, default=40)
a = ap.parse_args()

d = open(a.dmp, "rb").read()
assert d[:4] == b"MDMP", "not a minidump"
nstreams, dir_rva = struct.unpack_from("<II", d, 8)
streams = {}
for i in range(nstreams):
    typ, size, rva = struct.unpack_from("<III", d, dir_rva + 12 * i)
    streams.setdefault(typ, (rva, size))


def mdstr(rva):
    n = struct.unpack_from("<I", d, rva)[0]
    return d[rva + 4:rva + 4 + n].decode("utf-16-le", "replace")


# modules
mods = []
rva, _ = streams[4]
for i in range(struct.unpack_from("<I", d, rva)[0]):
    o = rva + 4 + 108 * i
    base, size = struct.unpack_from("<QI", d, o)
    name = mdstr(struct.unpack_from("<I", d, o + 20)[0])
    mods.append((base, size, name))


def mod_of(addr):
    for base, size, name in mods:
        if base <= addr < base + size:
            return base, name
    return None, None


# memory (for reading the stack): MemoryListStream (5) and Memory64ListStream (9)
ranges = []
if 5 in streams:
    rva, _ = streams[5]
    for i in range(struct.unpack_from("<I", d, rva)[0]):
        start, size, mrva = struct.unpack_from("<QII", d, rva + 4 + 16 * i)
        ranges.append((start, size, mrva))
if 9 in streams:
    rva, _ = streams[9]
    n, base_rva = struct.unpack_from("<QQ", d, rva)
    cur = base_rva
    for i in range(n):
        start, size = struct.unpack_from("<QQ", d, rva + 16 + 16 * i)
        ranges.append((start, size, cur))
        cur += size


def read(addr, n):
    for start, size, mrva in ranges:
        if start <= addr and addr + n <= start + size:
            return d[mrva + addr - start: mrva + addr - start + n]
    return None


# exception
rva, _ = streams[6]
tid = struct.unpack_from("<I", d, rva)[0]
code, flags, _, addr, nparams = struct.unpack_from("<IIQQI", d, rva + 8)
info = struct.unpack_from("<15Q", d, rva + 8 + 32)[:nparams]
ctx_size, ctx_rva = struct.unpack_from("<II", d, rva + 8 + 152)
ctx = d[ctx_rva:ctx_rva + ctx_size]
names = {0xC0000005: "ACCESS_VIOLATION", 0x80000003: "BREAKPOINT", 0xC000001D: "ILLEGAL_INSTRUCTION",
         0xC0000094: "INT_DIVIDE_BY_ZERO", 0xC00000FD: "STACK_OVERFLOW", 0xC0000409: "STACK_BUFFER_OVERRUN (fail fast / abort)",
         0xE06D7363: "C++ EXCEPTION (throw)", 0x40000015: "FATAL_APP_EXIT (abort)"}
print("exception: 0x%08X %s in thread %d at 0x%X" % (code, names.get(code, ""), tid, addr))
if code == 0xC0000005 and len(info) >= 2:
    print("  access violation %s address 0x%X" % (["reading", "writing", "?", "?", "?", "?", "?", "?", "executing"][info[0]] if info[0] < 9 else info[0], info[1]))

regs = {}
for nm, off in (("rax", 0x78), ("rcx", 0x80), ("rdx", 0x88), ("rbx", 0x90), ("rsp", 0x98), ("rbp", 0xA0),
                ("rsi", 0xA8), ("rdi", 0xB0), ("r8", 0xB8), ("r9", 0xC0), ("r10", 0xC8), ("r11", 0xD0),
                ("r12", 0xD8), ("r13", 0xE0), ("r14", 0xE8), ("r15", 0xF0), ("rip", 0xF8)):
    regs[nm] = struct.unpack_from("<Q", ctx, off)[0]
print("  " + "  ".join("%s=%X" % kv for kv in regs.items()))

exe_base = next((b for b, s, n in mods if n.lower().endswith(a.exe.lower().replace("/", "\\").split("\\")[-1].lower())), None)
pe = open(a.exe, "rb").read(0x400)
pref = struct.unpack_from("<Q", pe, struct.unpack_from("<I", pe, 0x3C)[0] + 24 + 24)[0]


def symbolize(addrs):
    """addrs inside the exe -> list of strings."""
    if not addrs:
        return []
    inp = "\n".join("0x%X" % (x - exe_base + pref) for x in addrs) + "\n"
    out = subprocess.run([a.symbolizer, "--obj=" + a.exe, "--demangle", "--inlining=false"], input=inp,
                         capture_output=True, text=True).stdout.strip().split("\n\n")
    return [" @ ".join(x.strip().splitlines()[:2]) for x in out]


frames = [regs["rip"]]
sp = regs["rsp"]
stack = b""
for k in range(0, 0x20000, 0x1000):
    chunk = read(sp + k, 0x1000)
    if chunk is None:
        chunk = read(sp + k, (0x1000 - ((sp + k) & 0xFFF)))
        if chunk is None:
            break
    stack += chunk
for i in range(0, len(stack) - 7, 8):
    v = struct.unpack_from("<Q", stack, i)[0]
    b, n = mod_of(v)
    if b is not None and n.lower().endswith(".exe") or (b is not None and len(frames) < a.depth and ("ucrtbase" in n.lower() or "kernel" in n.lower() or "ntdll" in n.lower() or "vcruntime" in n.lower())):
        frames.append(v)
    if len(frames) >= a.depth:
        break

exe_frames = [f for f in frames if exe_base and mod_of(f)[0] == exe_base]
syms = dict(zip(exe_frames, symbolize(exe_frames)))
print("\nbacktrace (crash frame first, then stack scan):")
for i, f in enumerate(frames):
    b, n = mod_of(f)
    where = syms.get(f) if f in syms else ("%s+0x%X" % (n.split("\\")[-1], f - b) if b else "?")
    print("  #%-2d 0x%016X  %s" % (i, f, where))
