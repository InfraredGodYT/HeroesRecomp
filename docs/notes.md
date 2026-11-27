# Research notes

Discoveries, dead ends and open questions. Newest sections at the bottom.

## 2026-11-26: Heroes `default.xbe` facts

Source: Sonic Heroes NTSC-U Xbox ISO (plain XISO, XDVDFS header at 0x10000, no video partition).
Regenerate with `tools/xbe_tools/xbe_info.py`.

### Identity
| Field | Value |
|---|---|
| File size | 5,525,504 bytes |
| SHA-1 | `32c9fe34c5db74b1f5f4890388a4e9b84f6f0ae7` |
| Title | `SONICHEROES` |
| Title ID | `0x5345002B` (`SE-043`: SEGA) |
| Region | `0x1` (North America) |
| Version | 1 |
| Kind | retail (entry/thunk XOR keys) |

### Layout
| Field | Value |
|---|---|
| Base address | `0x10000` |
| Entry point | `0x27972B` |
| Kernel thunk table | `0x386620` (start of `.rdata`) |
| Image size | `0x742FA0` (~7.3 MB) |
| Stack commit | `0x40000` |
| Heap reserve | `0x100000` |
| Init flags | `0x5` |

Sections: guest virtual address, virtual size, file offset, size in the file, flags:
```
.text      va 0x00011000  vsize 0x00319E8C  raw 0x00001000  rsize 0x00319E8C  flags 0x16
D3D        va 0x0032AEA0  vsize 0x00013588  raw 0x0031B000  rsize 0x0000FCC0  flags 0x7
DSOUND     va 0x0033E440  vsize 0x0001C614  raw 0x0032B000  rsize 0x0001C3AC  flags 0x7
PSFD00     va 0x0035AA60  vsize 0x00001758  raw 0x00348000  rsize 0x00001758  flags 0x26
PSFD_I     va 0x0035C1C0  vsize 0x000003C8  raw 0x0034A000  rsize 0x000003C8  flags 0x36
PSFD_B     va 0x0035C5A0  vsize 0x00000898  raw 0x0034B000  rsize 0x00000898  flags 0x36
PSFD_P     va 0x0035CE40  vsize 0x00000610  raw 0x0034C000  rsize 0x00000610  flags 0x16
DOLPHIN    va 0x0035D460  vsize 0x00000024  raw 0x0034D000  rsize 0x00000024  flags 0x6
D3DX       va 0x0035D4A0  vsize 0x00000470  raw 0x0034E000  rsize 0x00000470  flags 0x7
XGRPH      va 0x0035D920  vsize 0x00021358  raw 0x0034F000  rsize 0x000206DC  flags 0x7
XPP        va 0x0037EC80  vsize 0x0000799C  raw 0x00370000  rsize 0x0000799C  flags 0x7
.rdata     va 0x00386620  vsize 0x00055FDC  raw 0x00378000  rsize 0x00055FCC  flags 0x6
.data      va 0x003DC600  vsize 0x0036BA4C  raw 0x003CE000  rsize 0x00168D14  flags 0x7
DOLBY      va 0x00748060  vsize 0x00007180  raw 0x00537000  rsize 0x0000716C  flags 0x26
$$XTINFO   va 0x0074F1E0  vsize 0x00000084  raw 0x0053F000  rsize 0x00000084  flags 0x38
$$XTIMAGE  va 0x0074F280  vsize 0x00002800  raw 0x00540000  rsize 0x00002800  flags 0x38
$$XSIMAGE  va 0x00751A80  vsize 0x00001000  raw 0x00543000  rsize 0x00001000  flags 0x38
.XTLID     va 0x00752A80  vsize 0x00000510  raw 0x00544000  rsize 0x00000510  flags 0x38
```
Observations (unverified guesses are marked "?"):
- **Code is in `.text`.** Guest addresses in Ghidra/IDA and in `recompConfig.toml` start at 0x11000. File offset = address − 0x10000 within `.text`.
- **`D3D`, `DSOUND`, `D3DX`, `XGRPH` and `XPP`** are XDK library code linked in. The recompiler translates them too, so the host emulates the hardware they talk to (NV2A, APU) rather than the D3D8 API.
- **`DOLBY`** (flags 0x26) is probably the Dolby Digital encoder image for the audio DSP?
- **`PSFD*` are probably the "PSF" (pixel shader / fixed-function?) data?** Unknown, check before relying on it.
- **`DOLPHIN`** (0x24 bytes): an unknown name. Possibly a GameCube-port leftover (Dolphin is the GameCube codename)?
- **`.data`** has 0x36BA4C of virtual size but only 0x168D14 stored in the file. The rest is zero-filled at load (BSS).

### XDK libraries
Every library is from **XDK build 5659**:
```
XAPILIB   1.0.5659  qfe 1     D3D8      1.0.5659  qfe 4     DSOUND    1.0.5659  qfe 1
XBOXKRNL  1.0.5659  qfe 1     LIBCMT    1.0.5659  qfe 1     D3DX8     1.0.5659  qfe 1
XGRAPHC   1.0.5659  qfe 1
```
- XboxRecomp2 writes the D3D8 build into `x86_config.h` as `X86_CODE_D3D8V`. `HeroesRecomp/gpu/renderstate.cpp` (a CXBX-Reloaded table) uses it to pick the render-state layout. 5659 is a well-known build in CXBX's tables, so the layout should be right, but verify it once rendering starts.
- **Shadow uses XDK 5849** (see the comparison below), so the SDK code differs between the two games.

### Kernel imports
Heroes imports **116** kernel exports. It has no non-kernel import directory.
- **94** are already implemented in the host: `GUEST_FUNCTION_HOOK*(__imp__Name, ...)` in `HeroesRecomp/kernel/imports.cpp` and others. The host implements 98 in total.
- **22** are not. XboxRecomp2 (`xbe/kernel/mod.rs`) classes them as:
  - **14 data exports** (variables, keys and objects, not functions): `ExEventObjectType`, `HalBootSMCVideoMode`, `HalDiskCachePartitionCount`, `IdexChannelObject`, `KeTickCount`, `LaunchDataPage`, `PsThreadObjectType`, `XboxAlternateSignatureKeys`, `XboxHDKey`, `XboxHardwareInfo`, `XboxKrnlVersion`, `XboxSignatureKey`, `XeImageFileName`, `XePublicKeyData`.
    - Shadow imports most of these too, and they aren't `GUEST_FUNCTION_*` registrations, so XboxRecomp2/the host handle data exports some other way. Confirm for the ones only Heroes uses.
  - **8 functions:** `HalIsResetOrShutdownPending`, `IoCreateDevice`, `IoInvalidDeviceRequest`, `IoMarkIrpMustComplete`, `IoStartNextPacket`, `IoStartPacket`, `IofCompleteRequest`, `XcVerifyPKCS1Signature`.
    - The `Io*` set is the device-driver API, probably used by XAPI's statically linked memory unit/USB driver. `XcVerifyPKCS1Signature` is probably for save-signature checks? Find out which are actually called at runtime before implementing them; stub them so they log once.

## 2026-11-26: First XboxRecomp2 run

Config: `HeroesRecompLib/private/heroes_min.toml`. It has Shadow's generic `[optimizations]`, `[speed_hacks] enable = false`, and an empty `[fixes]`.
- **First run: a panic in `eliminate_stack_args`.**
  - It moves stack arguments into registers. Heroes passes `-1` (`push 0xFFFFFFFF`), and `decode/encode.rs` handed it to Zydis as an unsigned 64-bit value (`u64::from`).
  - Zydis rejected that as an imm32.
  - **Fixed** in our XboxRecomp2 fork (`heroes` branch, `0de40bf`) by sign-extending. Shadow never hit this case.
- **After the fix:** the run takes about 8 s.
  - 20,201 functions: 2,417 from known entry points + 17,784 "fallback" (brute-force) functions
  - 875 switch tables
  - 79 C++ units
  - about 3,600 unresolved calls/jumps
  - a few `DecodeFailed`/`OverlappingInstruction` stops, where data was read as code
- **Next:** only 2,417 functions were reached from known entry points. Most Heroes code is reached through function pointers (vtables, object tables), so discovery leans on brute force. Shadow's config has many `[fixes] functions` entries for the same reason. A missing function shows up at runtime as `UNKNOWN FUNCTION REACHED`.

## 2026-11-27: How ShadowRecomp's host depends on Shadow's binary

The "generic" host is not generic. It hooks **Shadow's** XDK and engine functions by address:
- `sub_3E5A27` = `DirectSoundCreate` in Shadow's XBE
- `sub_3DDAB0` = D3D `CreateDevice`
- file I/O, heap, textures, vertex buffers, shaders and so on

Everything goes through `X86_OVERRIDE_FUNC(sub_X)`, `GUEST_FUNCTION_HOOK*(sub_X, ...)` and `[[midasm_hook]]` entries.

### Reference binary
Shadow's `default.xbe`, from the owner's Xbox ISO, is in `HeroesRecompLib/private/shadow_default.xbe` (gitignored).
- 5,447,680 bytes, SHA-1 `7e63ee47ec4e37c3104d4f5fec9bde37e9cf1aa3`
- Title ID `0x53450086`, version 9
- This is the build ShadowRecomp targets: `0x3E5A27`, `0x3E419C` and `0x3DDAB0` are each the exact start of a function. The byte before is a `ret`/padding, and the function opens with `push ebp; mov ebp, esp`.
- **XDK 5849** (D3D8 qfe 15, DSOUND qfe 11), vs. Heroes' 5659. SDK functions changed between the two builds, so matching Shadow → Heroes must allow small differences, and some functions will need manual checks.

### What depends on Shadow addresses
| Where | Count | What | Plan |
|---|---|---|---|
| `patches/*.cpp` | 470 overrides + 378 of 387 midasm hooks | Shadow gameplay fixes and features | **Excluded from the build** (2026-11-27). Kept as reference. |
| core host: `gpu/` (~70), `audio/audio.cpp` (32), `misc_impl.cpp` (22), `kernel/io/file_system.cpp` (16), `mod/mod_loader.cpp` (12), `kernel/heap.cpp` (8), `os/gamelogs.cpp` (8), `app.cpp` (6), `input/` (3), ... | about 210 | XDK replacements (DirectSound, D3D, file I/O, heap) and engine glue | Must be **re-pointed at Heroes' addresses**. Next: a Shadow → Heroes function-matching tool. |
| Shadow-only features: achievements, Discord RPC (`misc/rich_presence/shadow/`), multiplayer HUD | | read Shadow game state | Disable. |

### Safety: what fails loudly and what doesn't
- **Loud (safe):** `X86_OVERRIDE_FUNC(sub_X)` has a `static_assert` requiring `sub_X` in `[fixes].function_hooks`. With Heroes' empty list, every leftover Shadow override is a compile error. Calls to Shadow addresses that don't exist in Heroes are link errors (169 of 175 referenced addresses).
- **Silent (dangerous):** these compile fine and would corrupt or misread Heroes at runtime. Every one must be guarded or ported before the game runs:
  - **Direct call that lands on a real Heroes function:** `exports.cpp:46` calls `sub_25BF80`, Shadow's "play sound effect" (thiscall on `IsndSeInstance`). Heroes has an unrelated function at `0x25BF80`.
    - The other 5 address collisions (`sub_11430`, `sub_1B2C30`, `sub_1F360`, `sub_25CBB0`, `sub_2AAC20`) are overrides, so the `static_assert` catches them.
  - **Fixed guest memory addresses.** All of them fall inside Heroes' `.data` (`0x3DC600`–`0x748060`).
    - **Writes:**
      - `app.cpp`: frame time `0x541FA8`, written **every frame**; voice language `0x54DD8C`
      - `audio/audio.cpp`: `0x54DD8C`, `0x51DD80`, plus calls to `sub_328BB0`
      - `user/config.cpp`: `0x54DD88`, `0x541504`
      - `input/input.cpp`: controller ports `0x559984 + 4*i`
    - **Reads:**
      - `app.cpp`: game mode `0x5022A4`
      - `gpu/pushbuffer.cpp`: a color at `0x55B3C0..CC`
      - `gpu/vertexbuffer.cpp`: `0x55F8F0`
      - `mod/mod_loader.cpp`: `0x54DD8C`
      - `ui/options_menu.cpp` and `ui/multiplayer_hud.cpp`: HUD state `0x54DE78` and more
      - `user/achievement_manager.cpp`
      - `misc/rich_presence/shadow/shadow.cpp`
  - Find these again with:
    `git grep -n -E 'X86_(LOAD|STORE)_[A-Z0-9]+\(0x|GuestVariable|GuestToHostFunction<.*sub_' -- HeroesRecomp ':!HeroesRecomp/patches'`

## 2026-11-27: Matching Shadow functions to Heroes

Tool: `tools/xbe_tools/match_shadow.py` writes `docs/shadow-matches.md`. It has no disassembler and uses no new dependency. It masks call/jump distances and in-image addresses, then compares bytes.
- **2,547 functions** are identical and unique in both games. Call-target matching adds 141 tiny wrappers, for 2,688 matched pairs in total.
- **Result:** 94 of the host's ~200 Shadow function bindings have a HIGH-confidence Heroes address.
  - All of them land in the same-named section, a check the tool doesn't use.
  - The DSOUND wrappers keep identical spacing.
- **D3D library layout:** Heroes = Shadow − about `0xA4D70`, with small drift where functions changed size between XDK 5849 and 5659. The tool's "layout" evidence uses this.
- **What doesn't match automatically:**
  - XAPI/CRT internals that changed between XDK builds (heap, `XInputOpen`)
  - Shadow's own game code and data, which has no copy in Heroes
  - mid-function hooks inside functions that changed
- `IDirectSoundBuffer_SetMixBinVolumes` has no slot in Heroes' DSOUND layout (`SetHeadroom` → `Play` is 0x1C apart in Heroes, but 0x38 in Shadow with it in between). Heroes' 5659 DSOUND probably doesn't link it.
