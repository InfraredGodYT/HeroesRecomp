"""List every place the host (outside patches/) is bound to a Shadow guest address.
usage: python -I tools/xbe_tools/shadow_bindings.py [--md]
Run from the repo root. Kinds:
  override  X86_OVERRIDE_FUNC / GUEST_FUNCTION_HOOK* / GUEST_FUNCTION_STUB on sub_X  (compile error until ported)
  import    override of a kernel import Heroes doesn't import                         (compile error)
  call      any other use of sub_X (direct call, GuestToHostFunction, pointer)       (link error, or SILENT if Heroes has sub_X)
  memory    X86_LOAD/STORE at a fixed address, GuestVariable{0x...}                   (SILENT: reads/writes Heroes memory)
Heroes function list: HeroesRecompLib/private/heroes_funcs.txt (from x86_func_mapping.cpp) marks silent collisions.
"""
import os, re, sys

ROOT = "HeroesRecomp"
SKIP = ("HeroesRecomp/patches/",)
heroes = set()
if os.path.exists("HeroesRecompLib/private/heroes_funcs.txt"):
    heroes = set(open("HeroesRecompLib/private/heroes_funcs.txt").read().split())

# Kernel imports Heroes has: the generated header defines X86_OVERRIDE_ALLOWED___imp__<Name> for each.
SHARED = "out/build/heroes-local/HeroesRecompLib/x86/x86_recomp_shared.h"
imports = None
if os.path.exists(SHARED):
    imports = set(re.findall(r"X86_OVERRIDE_ALLOWED___imp__(\w+)", open(SHARED, encoding="utf-8", errors="replace").read()))

ovr = re.compile(r"\b(X86_OVERRIDE_FUNC|GUEST_FUNCTION_HOOK\w*|GUEST_FUNCTION_STUB)\(\s*(__imp__\w+|sub_[0-9A-F]+)\s*(?:,\s*(\w+))?")
sub = re.compile(r"\b(?:__imp__)?(sub_[0-9A-F]{4,})\b")
mem = re.compile(r"X86_(LOAD|STORE)_\w+\(\s*(0x[0-9A-Fa-f]{4,})|GuestVariable<[^>]*>\s+(\w+)\s*\{\s*(0x[0-9A-Fa-f]+)")

rows = []
for dp, _, fs in os.walk(ROOT):
    for f in fs:
        p = os.path.join(dp, f).replace("\\", "/")
        if not p.endswith((".cpp", ".h", ".inl")) or p.startswith(SKIP):
            continue
        lines = open(p, encoding="utf-8", errors="replace").read().splitlines()
        for i, line in enumerate(lines, 1):
            code = line.split("//")[0]
            comment = line.split("//", 1)[1].strip() if "//" in line else ""
            seen = set()
            for m in ovr.finditer(code):
                target, name = m.group(2), m.group(3) or comment
                if target.startswith("__imp__"):
                    if imports is None or target[7:] not in imports:
                        rows.append((p, i, "import", target[7:], "not imported by Heroes", ""))
                    continue
                seen.add(target)
                if not name:  # name from a comment just above the override, if any
                    prev = lines[i - 2].strip() if i > 1 else ""
                    name = prev[2:].strip() if prev.startswith("//") else ""
                rows.append((p, i, "override", target, name, "COLLIDES" if target in heroes else ""))
            for m in sub.finditer(code):
                s = m.group(1)
                if s in seen or (("X86_FUNC_IMPL" in code) and "__imp__" + s in code):
                    continue
                seen.add(s)
                rows.append((p, i, "call", s, comment, "SILENT: Heroes has " + s if s in heroes else ""))
            for m in mem.finditer(code):
                addr = m.group(2) or m.group(4)
                what = (m.group(1) or "var " + m.group(3)).lower()
                rows.append((p, i, "memory", addr, what + (" | " + comment if comment else ""), "SILENT"))

rows.sort()
if "--md" in sys.argv:
    print("| File | Line | Kind | Shadow address | Name / hint | Risk |\n|---|---|---|---|---|---|")
    for r in rows:
        print("| `%s` | %d | %s | `%s` | %s | %s |" % (r[0][len(ROOT) + 1:], r[1], r[2], r[3], r[4].replace("|", "/")[:70], r[5]))
else:
    for r in rows:
        print("%-42s %5d  %-8s %-14s %-40s %s" % (r[0][len(ROOT) + 1:], r[1], r[2], r[3], r[4][:40], r[5]))
from collections import Counter
print("\ntotals:", dict(Counter(r[2] for r in rows)), "| silent:", sum(1 for r in rows if r[5]), file=sys.stderr)
