# Progress

What is ported, disabled or still bound to Shadow. Update with every change.

## Build status (2026-11-28)
**`HeroesRecomp.exe` builds and links** (`heroes-local` preset): all 80 generated Heroes units and the whole host. **Not run yet.**
- The installer still checks **Shadow's** file hashes (`install/hashes/game.cpp`).
- Several host features are disabled until ported (see below).

## Ported from Shadow (94 functions)
The 94 HIGH matches from `docs/shadow-matches.md` are now Heroes addresses in the host, and each is listed in `[fixes] function_hooks` (`HeroesRecompLib/config/recompConfig.toml`, which names the old Shadow address next to each).
- DirectSound: create, buffers, play/stop, volume, 3D, ...
- D3D: device creation, draw calls, render states, shaders, vertex buffers, ...
- XAPI file I/O: `XCreateFile`, `XReadFile`, `XSetFilePointer`, `XFindNextFile`
- heap: `RtlFreeHeap`, `XMemFree`
- threads, timers (`QueryPerformanceCounter`), ...

Not yet verified at runtime. XDK 5849 → 5659 struct layouts may differ in places the host reads (D3D device fields, pushbuffer).

## New Heroes code
- `HeroesRecomp/heroes/kernel_stubs.cpp`: the 8 kernel imports Heroes uses and the host lacked (`HalIsResetOrShutdownPending`, `IoCreateDevice`, `IoInvalidDeviceRequest`, `IoMarkIrpMustComplete`, `IoStartNextPacket`, `IoStartPacket`, `IofCompleteRequest`, `XcVerifyPKCS1Signature`). They are **stubs that log once** when first called.
- `HeroesRecomp/heroes/patch_stubs.cpp`: what the core needs from the excluded `patches/*.cpp`.
  - Shadow features become no-ops / `false` / `0`.
  - `AspectRatioPatches::ComputeOffsets` and `FindHudWidthPreset` are copied verbatim: pure host layout math.

## Disabled: `HEROES-TODO` blocks
Find them with `git grep -n "HEROES-TODO:"`. Each is `#if 0` code that still targets Shadow, so the original Heroes code runs unmodified there.

**Needed before the game is really playable.** Port these first; each needs a manual check (Ghidra) because the matcher was unsure:
- `app.cpp`:
  - main-loop hook `sub_57630`: frame timer, profiler, target FPS
  - init `sub_11AD0`: language/registry
  - voice language `sub_55BE0`
- `UpdateSDLEventsMidAsmHook` (midasm, not in the Heroes config yet): pumps SDL events, so without it **the window may be unresponsive**
- `gpu/video.cpp`:
  - surface creation `sub_3DA0A0`
  - `sub_3D5140`, `sub_2DDC00`
  - `DrawPrimitive`/`D3DDevice` stubs `sub_3DC150` and `sub_3DD560`
- `gpu/textures.cpp`: 6 overrides; `gpu/vertexbuffer.cpp`: 2; `gpu/pixelshader.cpp`: 1; `gpu/pushbuffer.cpp`: 1 + the RenderWare skin material-color function
- `kernel/io/file_system.cpp`: 10 overrides (XAPI file functions, save data)
- `kernel/heap.cpp`: 6 heap overrides (XAPI/CRT internals changed between XDK 5849 and 5659)
- `misc_impl.cpp`: 8 overrides, including `KeTickCount`
- `audio/audio.cpp`: `IDirectSoundBuffer_Unlock`, `SetCurrentPosition`, `SetMixBinVolumes`
- `input/input.cpp`: `XInputOpen` hook
- the midasm hooks `ClearWithQuads`, `RasterRenderQuad`, `_XbSubmitNode` and `DestructResource` (graphics)

**Shadow-only, stays off** (or gets a Heroes version later):
- voice-language live switch
- UI sounds through Shadow's sound engine
- Shadow's controller-port table and language variables
- options-menu HUD hiding and sound test
- achievements, Discord presence, multiplayer HUD
- `os/gamelogs.cpp` boot log messages
- `mod/mod_loader.cpp` hooks (Phase 3)
- the 4 kernel imports only Shadow uses

## Remaining live Shadow addresses
`python -I tools/xbe_tools/shadow_bindings.py` lists 10. All are in `misc/rich_presence/shadow/shadow.cpp` and `ui/multiplayer_hud.cpp`, and both modules are unreachable: their `Start`/`Init`/`Draw` calls are disabled.

## Next
1. Installer: Heroes file hashes (`install/hashes/game.cpp` via `tools/fshasher`), so the installer accepts the Heroes ISO.
2. First run, then the boot loop. Port the "needed" list above, starting with `UpdateSDLEvents` and the main loop, then whatever the first crash or hang points at.
