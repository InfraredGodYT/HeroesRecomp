"""Find Heroes addresses for the Shadow addresses the host is bound to.
usage (repo root): python -I tools/xbe_tools/match_shadow.py [--md] [--all-targets]

Inputs (all in the gitignored HeroesRecompLib/private/):
  shadow_default.xbe, default.xbe           the two games' executables
  shadow_out/x86/x86_func_mapping.cpp       Shadow function starts (xboxrecomp2 recompile shadow_cfg_local.toml)
  out/x86/x86_func_mapping.cpp              Heroes function starts (xboxrecomp2 recompile heroes_cfg_local.toml)
Targets: shadow_bindings.collect() (overrides, calls, memory) + midasm hooks implemented outside patches/.

Method, most certain first:
  1. exact    The target's bytes, with call/jump distances and in-image addresses masked out, equal
              exactly one Heroes function's (same length). Unique on both sides.
  2. xref     Any exactly matched pair (anchor) of functions has the same instructions at the same offsets,
              so a call target or address found at offset N in Shadow maps to the one at offset N in Heroes.
              Every anchor "votes"; this maps data addresses and changed (XDK 5849 vs 5659) functions too.
  3. fuzzy    Nearest Heroes function by shared 8-byte chunks. Only a hint: always check by hand.
Confidence: HIGH = exact, or xref with >=2 agreeing votes and no disagreement. Everything else needs a manual check.
No disassembler: masking is a byte heuristic. It can over-mask (fine) and, rarely, miss an address (lowers matches).
"""
import os, re, struct, sys
from collections import Counter, defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import shadow_bindings  # noqa: E402

P = "HeroesRecompLib/private/"
MIN_ANCHOR = 24      # bytes; shorter functions are too generic to anchor on
MAX_FUNC = 0x4000    # cap on function extent


class Xbe:
    def __init__(self, path, funcs_path):
        d = open(path, "rb").read()
        self.d = d
        u32 = lambda o: struct.unpack_from("<I", d, o)[0]
        self.base = u32(0x104)
        self.end = self.base + u32(0x10C)
        n, sa = u32(0x11C), u32(0x120) - self.base
        self.secs = []
        for i in range(n):
            flags, va, vsz, raw, rsz, _ = struct.unpack_from("<6I", d, sa + i * 56)
            self.secs.append((va, vsz, raw, rsz, flags))
        txt = open(funcs_path, encoding="utf-8").read()
        self.funcs = sorted(set(int(a, 16) for a in re.findall(r"\{\s*0x([0-9A-Fa-f]+)\s*,", txt)))
        self.funcset = set(self.funcs)
        self.code = [(va, va + rsz) for va, vsz, raw, rsz, fl in self.secs if fl & 0x4]

    def off(self, va):
        for v, vs, r, rs, _ in self.secs:
            if v <= va < v + rs:
                return r + va - v
        return None

    def read(self, va, n):
        o = self.off(va)
        return None if o is None else self.d[o:o + n]

    def in_code(self, va):
        return any(a <= va < b for a, b in self.code)

    def extent(self, i):
        """Bytes of function i: up to the next function start (and within its section)."""
        a = self.funcs[i]
        nxt = self.funcs[i + 1] if i + 1 < len(self.funcs) else a + MAX_FUNC
        o = self.off(a)
        if o is None:
            return b""
        sec_end = next((v + rs for v, vs, r, rs, _ in self.secs if v <= a < v + rs), a)
        return self.d[o:o + max(0, min(nxt, sec_end, a + MAX_FUNC) - a)]

    def containing(self, va):
        lo, hi = 0, len(self.funcs) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if self.funcs[mid] <= va:
                lo = mid
            else:
                hi = mid - 1
        return lo


def mask(x, va, b):
    """Positions (set) whose bytes are addresses: rel32 of call/jmp/jcc, and in-image dwords.
    Returns (masked_bytes, refs) where refs = {offset: ('rel'|'abs', target)}."""
    m = bytearray(b)
    refs = {}
    n = len(b)
    i = 0
    while i < n:
        op = b[i]
        if op in (0xE8, 0xE9) and i + 5 <= n:
            t = (va + i + 5 + struct.unpack_from("<i", b, i + 1)[0]) & 0xFFFFFFFF
            if x.in_code(t):
                refs[i + 1] = ("rel", t)
                m[i + 1:i + 5] = b"\0\0\0\0"
        elif op == 0x0F and i + 6 <= n and 0x80 <= b[i + 1] <= 0x8F:
            t = (va + i + 6 + struct.unpack_from("<i", b, i + 2)[0]) & 0xFFFFFFFF
            if x.in_code(t):
                refs[i + 2] = ("rel", t)
                m[i + 2:i + 6] = b"\0\0\0\0"
        i += 1
    for i in range(n - 3):
        if any(i <= k < i + 4 or k <= i < k + 4 for k in (i - 3, i - 2, i - 1, i) if k in refs):
            continue
        v = struct.unpack_from("<I", b, i)[0]
        if x.base + 0x1000 <= v < x.end:
            refs.setdefault(i, ("abs", v))
            m[i:i + 4] = b"\0\0\0\0"
    return bytes(m), refs


def build(x):
    sig = {}       # func index -> masked bytes
    refs = {}
    for i, a in enumerate(x.funcs):
        b = x.extent(i)
        sig[i], refs[i] = mask(x, a, b)
    return sig, refs


def main():
    sx = Xbe(P + "shadow_default.xbe", P + "shadow_out/x86/x86_func_mapping.cpp")
    hx = Xbe(P + "default.xbe", P + "out/x86/x86_func_mapping.cpp")
    print("functions: shadow %d, heroes %d" % (len(sx.funcs), len(hx.funcs)), file=sys.stderr)
    ssig, sref = build(sx)
    hsig, href = build(hx)

    # 1. exact, unique on both sides
    sby, hby = defaultdict(list), defaultdict(list)
    for i, s in ssig.items():
        sby[s].append(i)
    for i, s in hsig.items():
        hby[s].append(i)
    exact = {}      # shadow func addr -> heroes func addr
    for s, si in sby.items():
        hi = hby.get(s)
        if len(si) == 1 and hi and len(hi) == 1:
            exact[sx.funcs[si[0]]] = hx.funcs[hi[0]]
    sidx = {a: i for i, a in enumerate(sx.funcs)}
    hidx = {a: i for i, a in enumerate(hx.funcs)}
    print("exact unique matches: %d (anchors >= %d bytes: %d)" % (
        len(exact), MIN_ANCHOR, sum(1 for a in exact if len(ssig[sidx[a]]) >= MIN_ANCHOR)), file=sys.stderr)

    # 2. xref votes from matched pairs (same offset, same kind), and
    # 2b. "callees": functions with identical masked bytes that tie (e.g. tiny wrappers) are told apart
    #     by their call targets: s matches h if every call/jump target of s is already known to map to
    #     h's target at the same offset. Repeat until nothing new is found.
    matched = dict(exact)            # shadow func -> heroes func
    method = {a: "exact" for a in exact}
    groups = [(si, hby[s]) for s, si in sby.items() if s in hby and not (len(si) == 1 and len(hby[s]) == 1)]

    def compute_votes():
        v = defaultdict(Counter)
        for a, h in matched.items():
            si, hi = sidx[a], hidx[h]
            if method[a] == "exact" and len(ssig[si]) < MIN_ANCHOR:
                continue
            hr = href[hi]
            for off, (kind, t) in sref[si].items():
                r = hr.get(off)
                if r and r[0] == kind:
                    v[t][r[1]] += 1
        return v

    for rnd in range(10):
        votes = compute_votes()
        known = dict(matched)
        for t, c in votes.items():
            top = c.most_common(1)[0]
            if t not in known and top[1] >= 2 and top[1] == sum(c.values()):
                known[t] = top[0]
        used = set(matched.values())
        new = 0
        for sis, his in groups:
            free_h = [hi for hi in his if hx.funcs[hi] not in used]
            for si in sis:
                a = sx.funcs[si]
                if a in matched:
                    continue
                rels = [(off, t) for off, (k, t) in sref[si].items() if k == "rel"]
                if not rels:
                    continue
                ok = []
                for hi in free_h:
                    hr = href[hi]
                    if all(t in known and hr.get(off, (None, None))[1] == known[t] for off, t in rels):
                        ok.append(hi)
                if len(ok) == 1:
                    matched[a] = hx.funcs[ok[0]]
                    method[a] = "callees"
                    used.add(hx.funcs[ok[0]])
                    free_h.remove(ok[0])
                    new += 1
        print("round %d: +%d matched by callees (total %d)" % (rnd + 1, new, len(matched)), file=sys.stderr)
        if not new:
            break
    votes = compute_votes()
    exact = matched

    # 3. fuzzy index: 8-byte chunks of masked Heroes functions
    def chunks(s):
        return {s[i:i + 8] for i in range(0, max(0, len(s) - 7), 2)}
    index = defaultdict(set)
    hchunks = {}
    for i, s in hsig.items():
        if len(s) >= 16:
            c = chunks(s)
            hchunks[i] = c
            for k in c:
                if len(index[k]) < 40:
                    index[k].add(i)

    def fuzzy(a):
        si = sidx.get(a)
        if si is None:
            return None
        c = chunks(ssig[si])
        if not c:
            return None
        cnt = Counter()
        for k in c:
            for i in index.get(k, ()):
                cnt[i] += 1
        best = []
        for i, _ in cnt.most_common(5):
            j = len(c & hchunks[i]) / len(c | hchunks[i])
            best.append((j, hx.funcs[i]))
        best.sort(reverse=True)
        return best[:2]

    # targets
    targets = []   # (kind, shadow addr, where, hint)
    for f, line, kind, val, hint, risk in shadow_bindings.collect():
        if kind in ("override", "call"):
            targets.append((kind, int(val[4:], 16), "%s:%d" % (f[len("HeroesRecomp/"):], line), hint))
        elif kind == "memory":
            targets.append((kind, int(val, 16), "%s:%d" % (f[len("HeroesRecomp/"):], line), hint))
        elif kind == "const":
            # A named constant: a function start is matched like a call, anything else like data.
            a = int(val, 16)
            targets.append(("call" if a in sx.funcset else "memory", a, "%s:%d" % (f[len("HeroesRecomp/"):], line), hint))
    cfg = open("HeroesRecompLib/config/shadow/recompConfig.toml", encoding="utf-8").read()
    for blk in re.findall(r"\[\[midasm_hook\]\](.*?)(?=\n\[|\Z)", cfg, re.S):
        nm = re.search(r'name\s*=\s*"(\w+)"', blk)
        ad = re.search(r"address\s*=\s*0x([0-9A-Fa-f]+)", blk)
        if not (nm and ad):
            continue
        hits = [l for l in os.popen('git grep -l -w %s -- HeroesRecomp ":!HeroesRecomp/patches"' % nm.group(1)).read().split()]
        if hits:
            targets.append(("midasm", int(ad.group(1), 16), hits[0][len("HeroesRecomp/"):], nm.group(1)))

    seen = set()
    rows = []
    for kind, a, where, hint in targets:
        key = (kind if kind != "call" else "override", a)
        if key in seen and kind != "memory":
            continue
        seen.add(key)
        res, how, conf = None, "", "CHECK"
        if kind == "midasm":
            ci = sx.containing(a)
            fa = sx.funcs[ci]
            if fa in exact:
                res, how, conf = exact[fa] + (a - fa), "inside %s match of sub_%X +0x%X" % (method[fa], fa, a - fa), "HIGH"
            else:
                how = "containing sub_%X not matched exactly" % fa
                fz = fuzzy(fa)
                if fz:
                    how += "; fuzzy sub_%X (%.2f) -> offset unknown" % (fz[0][1], fz[0][0])
        else:
            ex = exact.get(a)
            v = votes.get(a)
            top = v.most_common(2) if v else []
            if ex is not None:
                res, how, conf = ex, method[a], "HIGH"
                if top and top[0][0] != ex:
                    how += "; xref disagrees (%X x%d)" % top[0]
                    conf = "CHECK"
                elif top:
                    how += " + xref x%d" % top[0][1]
            elif top:
                res = top[0][0]
                agree, total = top[0][1], sum(v.values())
                how = "xref %d/%d" % (agree, total)
                if agree >= 2 and agree == total:
                    conf = "HIGH"
                if kind != "memory" and res not in hx.funcset:
                    how += "; not a Heroes function start"
                    conf = "CHECK"
            elif kind != "memory":
                fz = fuzzy(a)
                if fz:
                    res = fz[0][1]
                    how = "fuzzy %.2f" % fz[0][0] + (" (next %.2f sub_%X)" % (fz[1][0], fz[1][1]) if len(fz) > 1 else "")
                else:
                    how = "no candidate"
        rows.append((where, kind, a, res, conf, how, hint))

    # Layout check for unsure function candidates: inside one library section the code keeps its order,
    # so Heroes = Shadow - delta, with delta drifting slowly. Compare the candidate's delta with the
    # nearest HIGH matches before and after it (by Shadow address). Evidence only, never upgrades to HIGH.
    def sec_name(x, va):
        for va0, vsz, raw, rsz, fl in x.secs:
            if va0 <= va < va0 + vsz:
                o = x.off(va0)
                return (va0, vsz)
        return None
    high = sorted((a, h) for (_, _, a, h, c, _, _) in rows if c == "HIGH" and h is not None)
    out = []
    for where, kind, a, res, conf, how, hint in rows:
        if conf != "HIGH" and res is not None and kind in ("override", "call", "midasm"):
            sec = sec_name(sx, a)
            same = [(sa, sh) for sa, sh in high if sec_name(sx, sa) == sec]
            before = [p for p in same if p[0] < a][-1:]
            after = [p for p in same if p[0] > a][:1]
            if before and after:
                d, lo, hi = a - res, min(before[0][0] - before[0][1], after[0][0] - after[0][1]), \
                    max(before[0][0] - before[0][1], after[0][0] - after[0][1])
                how += "; layout %s (delta 0x%X, neighbours 0x%X..0x%X)" % (
                    "OK" if lo - 0x40 <= d <= hi + 0x40 else "OFF", d, lo, hi)
        out.append((where, kind, a, res, conf, how, hint))
    rows = out

    md = "--md" in sys.argv
    if md:
        print("| Where | Kind | Shadow | Heroes | Confidence | Evidence | Name / hint |\n|---|---|---|---|---|---|---|")
    for where, kind, a, res, conf, how, hint in rows:
        h = "0x%X" % res if res is not None else "?"
        if md:
            print("| `%s` | %s | `0x%X` | `%s` | %s | %s | %s |" % (where, kind, a, h, conf, how, hint.replace("|", "/")[:50]))
        else:
            print("%-36s %-8s 0x%-8X %-10s %-5s %-50s %s" % (where, kind, a, h, conf, how[:50], hint[:30]))
    c = Counter((r[1], r[4]) for r in rows)
    print("\nsummary:", ", ".join("%s %s: %d" % (k[0], k[1], n) for k, n in sorted(c.items())), file=sys.stderr)


if __name__ == "__main__":
    main()
