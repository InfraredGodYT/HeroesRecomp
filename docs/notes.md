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
- **Open question:** which XDK build does Shadow (2005) use? If it differs, any D3D/DSOUND code the host matches by address or layout (pushbuffer, `patches/*`) may differ too.

### Kernel imports
Heroes imports **116** kernel exports. It has no non-kernel import directory.
- **94** are already implemented in the host: `GUEST_FUNCTION_HOOK*(__imp__Name, ...)` in `HeroesRecomp/kernel/imports.cpp` and others. The host implements 98 in total.
- **22** are not. XboxRecomp2 (`xbe/kernel/mod.rs`) classes them as:
  - **14 data exports** (variables, keys and objects, not functions): `ExEventObjectType`, `HalBootSMCVideoMode`, `HalDiskCachePartitionCount`, `IdexChannelObject`, `KeTickCount`, `LaunchDataPage`, `PsThreadObjectType`, `XboxAlternateSignatureKeys`, `XboxHDKey`, `XboxHardwareInfo`, `XboxKrnlVersion`, `XboxSignatureKey`, `XeImageFileName`, `XePublicKeyData`.
    - Shadow handles at least `KeTickCount` differently: `HeroesRecomp/misc_impl.cpp` hooks a Shadow function `sub_388458` that reads it. Find how the host serves data exports before Phase 2.
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
