# Progress

What is ported, disabled or still bound to Shadow. Update with every change.

## Shadow → Heroes porting checklist (host outside `patches/`)

Generated 2026-11-27 with `python -I tools/xbe_tools/shadow_bindings.py --md`. Rerun it after changes; when a row is ported, it drops out of the list.

Status of the first build (`heroes-local`, 2026-11-27):
- configure works
- all 80 generated Heroes units compile
- 22 host files fail: 121 Shadow overrides, 12 direct Shadow calls and 4 kernel imports, plus whatever was hidden behind Clang's 20-errors-per-file limit in `audio.cpp`, `video.cpp` and `misc_impl.cpp`

Kinds:
- **override:** `X86_OVERRIDE_FUNC`/`GUEST_FUNCTION_HOOK*` on a Shadow address. It is a compile error until the matching Heroes function is found and listed in `[fixes] function_hooks`. `COLLIDES` means Heroes has an unrelated function at the same address (`static_assert` failure).
- **call:** any other use of a Shadow function (direct call, `GuestToHostFunction`, function pointer). A link error, **or silent** when Heroes has a function at that address.
- **memory:** a fixed Shadow data address. **Always silent**: it reads or writes Heroes memory. Must be ported or guarded before the game runs.
- **import:** a kernel function Shadow imports but Heroes doesn't. Leave it out for Heroes.

**Matches:** `docs/shadow-matches.md` (2026-11-27) has the Heroes address for each row.
- 94 HIGH: 89 overrides and 5 calls.
- 62 overrides, 12 calls, 9 midasm hooks and 35 memory addresses need a manual check or have no counterpart.
- Port HIGH rows first. For CHECK rows, confirm in Ghidra first.

Disabled entirely (not in the build): `patches/*.cpp`, which is Shadow gameplay. Reference config: `HeroesRecompLib/config/shadow/recompConfig.toml`.

| File | Line | Kind | Shadow address | Name / hint | Risk |
|---|---|---|---|---|---|
| `app.cpp` | 56 | override | `sub_57630` |  |  |
| `app.cpp` | 63 | memory | `0x541FA8` | load | SILENT |
| `app.cpp` | 65 | memory | `0x541FA8` | store | SILENT |
| `app.cpp` | 68 | memory | `0x541FA8` | load | SILENT |
| `app.cpp` | 70 | memory | `0x5022A4` | load | SILENT |
| `app.cpp` | 73 | call | `sub_57630` |  |  |
| `app.cpp` | 81 | override | `sub_11AD0` |  |  |
| `app.cpp` | 89 | call | `sub_11AD0` |  |  |
| `app.cpp` | 94 | override | `sub_55BE0` |  |  |
| `app.cpp` | 96 | call | `sub_55BE0` |  |  |
| `app.cpp` | 98 | memory | `0x54DD8C` | store | SILENT |
| `audio/audio.cpp` | 377 | call | `sub_25C750` | stop all sound |  |
| `audio/audio.cpp` | 380 | memory | `0x54DD8C` | store / set voice language to new value (eng or jp) | SILENT |
| `audio/audio.cpp` | 385 | call | `sub_328BB0` |  |  |
| `audio/audio.cpp` | 385 | memory | `0x51DDA0` | load | SILENT |
| `audio/audio.cpp` | 389 | call | `sub_328BB0` |  |  |
| `audio/audio.cpp` | 389 | memory | `0x51DD9C` | load | SILENT |
| `audio/audio.cpp` | 395 | call | `sub_328B90` |  |  |
| `audio/audio.cpp` | 397 | call | `sub_32AC10` |  |  |
| `audio/audio.cpp` | 398 | call | `sub_328F30` |  |  |
| `audio/audio.cpp` | 405 | memory | `0x51DD80` | store | SILENT |
| `audio/audio.cpp` | 409 | override | `sub_3E5A27` | DirectSoundCreate_r |  |
| `audio/audio.cpp` | 410 | override | `sub_3E5756` | IDirectSound_CreateSoundBuffer |  |
| `audio/audio.cpp` | 411 | override | `sub_3E4EE9` | IDirectSoundBuffer_SetBufferData |  |
| `audio/audio.cpp` | 412 | override | `sub_3E421C` | IDirectSoundBuffer_GetStatus |  |
| `audio/audio.cpp` | 413 | override | `sub_3E4274` | IDirectSoundBuffer_Lock |  |
| `audio/audio.cpp` | 414 | override | `sub_3E34B8` | IDirectSoundBuffer_Unlock |  |
| `audio/audio.cpp` | 415 | override | `sub_3E35B5` | CDirectSound_DownloadEffectsImage |  |
| `audio/audio.cpp` | 416 | override | `sub_3E4238` | IDirectSoundBuffer_GetCurrentPosition |  |
| `audio/audio.cpp` | 417 | override | `sub_3E4258` | IDirectSoundBuffer_SetCurrentPosition |  |
| `audio/audio.cpp` | 418 | override | `sub_3E419C` | IDirectSoundBuffer_Play |  |
| `audio/audio.cpp` | 419 | override | `sub_3E41C0` | IDirectSoundBuffer_Stop |  |
| `audio/audio.cpp` | 420 | override | `sub_3E41D8` | IDirectSoundBuffer_StopEx |  |
| `audio/audio.cpp` | 421 | override | `sub_3E34BD` | IDirectSoundBuffer_Release |  |
| `audio/audio.cpp` | 422 | override | `sub_3E412C` | IDirectSoundBuffer_SetVolume |  |
| `audio/audio.cpp` | 423 | override | `sub_3E41FC` | IDirectSoundBuffer_SetLoopRegion |  |
| `audio/audio.cpp` | 424 | override | `sub_3E4148` | IDirectSoundBuffer_SetPitch |  |
| `audio/audio.cpp` | 425 | override | `sub_3E4164` | IDirectSoundBuffer_SetHeadroom |  |
| `audio/audio.cpp` | 426 | override | `sub_3E48FA` | IDirectSoundBuffer_SetFrequency |  |
| `audio/audio.cpp` | 429 | override | `sub_3E497A` | IDirectSoundBuffer_SetMinDistance |  |
| `audio/audio.cpp` | 430 | override | `sub_3E4180` | IDirectSoundBuffer_SetMixBinVolumes_8 |  |
| `audio/audio.cpp` | 431 | override | `sub_3E4916` | IDirectSoundBuffer_SetMixBins |  |
| `audio/audio.cpp` | 432 | override | `sub_3E4956` | IDirectSoundBuffer_SetMaxDistance |  |
| `audio/audio.cpp` | 433 | override | `sub_3E4932` | IDirectSoundBuffer_SetConeAngles |  |
| `audio/audio.cpp` | 434 | override | `sub_3E499E` | IDirectSoundBuffer_SetPosition |  |
| `audio/audio.cpp` | 435 | override | `sub_3E577A` | IDirectSound_SetAllParameters |  |
| `audio/audio.cpp` | 436 | override | `sub_3E5861` | IDirectSound_SetI3DL2Listener |  |
| `audio/audio.cpp` | 437 | override | `sub_3E579A` | IDirectSound_SetDistanceFactor |  |
| `audio/audio.cpp` | 438 | override | `sub_3E583D` | IDirectSound_SetRolloffFactor |  |
| `audio/audio.cpp` | 439 | override | `sub_3E5808` | IDirectSound_SetPosition |  |
| `audio/audio.cpp` | 440 | override | `sub_3E57BE` | IDirectSound_SetOrientation |  |
| `audio/audio.cpp` | 441 | override | `sub_3E52DC` | IDirectSound_CommitDeferredSettings |  |
| `audio/audio.cpp` | 442 | override | `sub_3E4FB4` | CDirectSound_CommitDeferredSettings |  |
| `audio/driver/sdl3_driver.cpp` | 124 | override | `sub_25CBB0` |  | COLLIDES |
| `audio/driver/sdl3_driver.cpp` | 133 | call | `sub_25CBB0` |  | SILENT: Heroes has sub_25CBB0 |
| `cpu/guest_thread.cpp` | 385 | override | `sub_2ADC8D` | GetThreadPriorityImpl |  |
| `cpu/guest_thread.cpp` | 387 | override | `sub_2ADF79` | XAPILIBGetCurrentThreadId |  |
| `cpu/guest_thread.cpp` | 389 | override | `sub_2B126A` | Some kernel hardware check, not needed on PC. |  |
| `exports.cpp` | 10 | override | `sub_55A80` |  |  |
| `exports.cpp` | 13 | call | `sub_55A80` |  |  |
| `exports.cpp` | 46 | call | `sub_25BF80` |  | SILENT: Heroes has sub_25BF80 |
| `gpu/FixedFunction.cpp` | 863 | override | `sub_3D0B70` |  |  |
| `gpu/FixedFunction.cpp` | 867 | call | `sub_3D0B70` |  |  |
| `gpu/FixedFunction.cpp` | 873 | override | `sub_3D08B0` |  |  |
| `gpu/FixedFunction.cpp` | 877 | call | `sub_3D08B0` |  |  |
| `gpu/FixedFunction.cpp` | 886 | override | `sub_3CFDF0` | SetMaterial |  |
| `gpu/pixelshader.cpp` | 209 | override | `sub_3DC790` |  |  |
| `gpu/pixelshader.cpp` | 211 | call | `sub_3DC790` |  |  |
| `gpu/pixelshader.cpp` | 220 | override | `sub_3DC590` |  |  |
| `gpu/pixelshader.cpp` | 224 | call | `sub_3DC590` |  |  |
| `gpu/pushbuffer.cpp` | 153 | memory | `0x55B3C0` | load | SILENT |
| `gpu/pushbuffer.cpp` | 154 | memory | `0x55B3C4` | load | SILENT |
| `gpu/pushbuffer.cpp` | 155 | memory | `0x55B3C8` | load | SILENT |
| `gpu/pushbuffer.cpp` | 156 | memory | `0x55B3CC` | load | SILENT |
| `gpu/pushbuffer.cpp` | 221 | override | `sub_3D4740` | D3DDevice_BeginPush |  |
| `gpu/pushbuffer.cpp` | 222 | override | `sub_3D4760` | D3DDevice_EndPush |  |
| `gpu/pushbuffer.cpp` | 224 | override | `sub_2C51C0` | _rpSkinXboxSetMaterialColor |  |
| `gpu/renderstate.cpp` | 376 | override | `sub_3D2420` | D3DDevice_SetRenderState_Simple |  |
| `gpu/resources.cpp` | 20 | override | `sub_3D5DB0` | DestructResource |  |
| `gpu/textures.cpp` | 27 | override | `sub_2DF9F0` |  |  |
| `gpu/textures.cpp` | 29 | call | `sub_2DF9F0` |  |  |
| `gpu/textures.cpp` | 490 | override | `sub_2ED0D0` |  |  |
| `gpu/textures.cpp` | 493 | call | `sub_2ED0D0` |  |  |
| `gpu/textures.cpp` | 533 | override | `sub_2DF870` |  |  |
| `gpu/textures.cpp` | 540 | call | `sub_2DF870` |  |  |
| `gpu/textures.cpp` | 545 | override | `sub_40B363` |  |  |
| `gpu/textures.cpp` | 550 | call | `sub_40B363` |  |  |
| `gpu/textures.cpp` | 596 | override | `sub_2DFE20` |  |  |
| `gpu/textures.cpp` | 604 | call | `sub_2DFE20` |  |  |
| `gpu/textures.cpp` | 642 | override | `sub_3D0CD0` | SetTexture |  |
| `gpu/textures.cpp` | 643 | override | `sub_3DD710` | CreateTexture_ |  |
| `gpu/textures.cpp` | 644 | override | `sub_3DD810` | LockTextureRect |  |
| `gpu/vertexbuffer.cpp` | 57 | override | `sub_2E6750` |  |  |
| `gpu/vertexbuffer.cpp` | 61 | call | `sub_2E6750` |  |  |
| `gpu/vertexbuffer.cpp` | 72 | override | `sub_2E67D0` |  |  |
| `gpu/vertexbuffer.cpp` | 74 | call | `sub_2E67D0` |  |  |
| `gpu/vertexbuffer.cpp` | 76 | memory | `0x55F8F0` | load | SILENT |
| `gpu/vertexbuffer.cpp` | 89 | override | `sub_3D5FE0` |  |  |
| `gpu/vertexbuffer.cpp` | 97 | call | `sub_3D5FE0` |  |  |
| `gpu/vertexbuffer.cpp` | 102 | override | `sub_3D1E10` |  |  |
| `gpu/vertexbuffer.cpp` | 112 | call | `sub_3D1E10` |  |  |
| `gpu/vertexshader.cpp` | 838 | override | `sub_3D2010` |  |  |
| `gpu/vertexshader.cpp` | 841 | call | `sub_3D2010` |  |  |
| `gpu/vertexshader.cpp` | 947 | call | `sub_3D2280` |  |  |
| `gpu/vertexshader.cpp` | 954 | override | `sub_3D2280` | SetVertexShader |  |
| `gpu/vertexshader.cpp` | 955 | override | `sub_3D1F70` | SelectVertexShader |  |
| `gpu/vertexshader.cpp` | 956 | override | `sub_3D1F00` | LoadVertexShaderProgram |  |
| `gpu/vertexshader.cpp` | 957 | override | `sub_3D1EA0` | LoadVertexShader |  |
| `gpu/vertexshader.cpp` | 959 | override | `sub_3D1CE0` | D3DDevice_SetVertexShaderConstant1Fast |  |
| `gpu/vertexshader.cpp` | 960 | override | `sub_3D1D30` | D3DDevice_SetVertexShaderConstantNotInlineFast |  |
| `gpu/video.cpp` | 3560 | override | `sub_2DDC00` |  |  |
| `gpu/video.cpp` | 3563 | call | `sub_2DDC00` |  |  |
| `gpu/video.cpp` | 4167 | call | `sub_3DA010` |  |  |
| `gpu/video.cpp` | 4373 | call | `sub_3DDAB0` |  |  |
| `gpu/video.cpp` | 4925 | override | `sub_3D5140` |  |  |
| `gpu/video.cpp` | 4928 | call | `sub_3D5140` |  |  |
| `gpu/video.cpp` | 4959 | call | `sub_3D0040` |  |  |
| `gpu/video.cpp` | 5050 | call | `sub_3D0680` |  |  |
| `gpu/video.cpp` | 5206 | override | `sub_3CFCD0` |  |  |
| `gpu/video.cpp` | 5213 | call | `sub_3CFCD0` |  |  |
| `gpu/video.cpp` | 6395 | override | `sub_3D5F10` |  |  |
| `gpu/video.cpp` | 6401 | call | `sub_3D5F10` |  |  |
| `gpu/video.cpp` | 6411 | override | `sub_2E6140` |  |  |
| `gpu/video.cpp` | 6430 | call | `sub_2E6140` |  |  |
| `gpu/video.cpp` | 6435 | override | `sub_3DA0A0` |  |  |
| `gpu/video.cpp` | 6441 | call | `sub_3DA0A0` |  |  |
| `gpu/video.cpp` | 6702 | override | `sub_3DDAB0` | Direct3D_CreateDevice_r |  |
| `gpu/video.cpp` | 6703 | override | `sub_3DD950` | SetResolution |  |
| `gpu/video.cpp` | 6706 | override | `sub_3DA010` | CreateSurfaceOfTexture |  |
| `gpu/video.cpp` | 6707 | override | `sub_3D0040` | D3DDevice_SetRenderTarget |  |
| `gpu/video.cpp` | 6710 | override | `sub_3D0680` | D3DDevice_SetViewport |  |
| `gpu/video.cpp` | 6711 | override | `sub_3D6BF0` | D3DDevice_Clear |  |
| `gpu/video.cpp` | 6712 | override | `sub_3DC1F0` | DrawIndexedPrimitive |  |
| `gpu/video.cpp` | 6713 | override | `sub_3DC150` | DrawPrimitive |  |
| `gpu/video.cpp` | 6714 | override | `sub_3DBFE0` | DrawPrimitiveUP |  |
| `gpu/video.cpp` | 6715 | override | `sub_3DD560` | D3DDevice_Swap |  |
| `gpu/video.cpp` | 6720 | override | `sub_3CFE70` | CRI_BlockUntilVerticalBlank |  |
| `gpu/video.cpp` | 6723 | override | `sub_3D3240` | D3D_CommonSetDebugRegisters |  |
| `gpu/video.cpp` | 6724 | override | `sub_3D0F10` | D3Device_IsBusy |  |
| `gpu/video.cpp` | 6725 | override | `sub_3D4720` | D3DDevice_IsFencePending |  |
| `gpu/video.cpp` | 6726 | override | `sub_3D5F60` | D3DResource_BlockUntilNotBusy |  |
| `gpu/video.cpp` | 6727 | override | `sub_3D4950` | D3D_BlockOnTime |  |
| `gpu/video.cpp` | 6728 | override | `sub_3DD8B0` | D3D_GetAdapterModeCount |  |
| `gpu/video.cpp` | 6729 | override | `sub_3D0F40` | D3D_SetTileNoWait |  |
| `input/input.cpp` | 954 | memory | `0x559984` | store | SILENT |
| `input/input.cpp` | 961 | override | `sub_4248A6` | input |  |
| `input/input.cpp` | 962 | override | `sub_424919` | input |  |
| `input/input.cpp` | 963 | override | `sub_2AAC20` | XInputOpen | COLLIDES |
| `kernel/heap.cpp` | 186 | override | `sub_2ABFCB` | AllocateHeap_r |  |
| `kernel/heap.cpp` | 188 | override | `sub_2AC3EF` | RtlAllocateHeap |  |
| `kernel/heap.cpp` | 189 | override | `sub_2ACD5E` | RtlReAllocateHeap |  |
| `kernel/heap.cpp` | 190 | override | `sub_2ADA9A` | XAPILIB_VirtualAlloc_r |  |
| `kernel/heap.cpp` | 193 | override | `sub_2B0494` | XAPILIB_XMemFree_r |  |
| `kernel/heap.cpp` | 194 | override | `sub_2ACB6A` | FreeBlock |  |
| `kernel/heap.cpp` | 195 | override | `sub_2ADAC8` | XAPILIB_VirtualFree_r |  |
| `kernel/heap.cpp` | 197 | override | `sub_2AD48C` | RtlFreeHeap |  |
| `kernel/imports.cpp` | 1326 | override | `sub_3D7B81` |  |  |
| `kernel/imports.cpp` | 1328 | call | `sub_3D7B81` |  |  |
| `kernel/imports.cpp` | 1343 | import | `NtQueryFullAttributesFile` | not imported by Heroes |  |
| `kernel/imports.cpp` | 1366 | import | `NtYieldExecution` | not imported by Heroes |  |
| `kernel/imports.cpp` | 1386 | import | `MmFreeSystemMemory` | not imported by Heroes |  |
| `kernel/imports.cpp` | 1388 | import | `MmAllocateSystemMemory` | not imported by Heroes |  |
| `kernel/imports.cpp` | 1442 | override | `sub_2ADB04` | XGetLanguage |  |
| `kernel/imports.cpp` | 1445 | override | `sub_2B17E8` | XAutoPowerDownResetTimer |  |
| `kernel/io/file_system.cpp` | 574 | override | `sub_2A4420` |  |  |
| `kernel/io/file_system.cpp` | 576 | call | `sub_2A4420` |  |  |
| `kernel/io/file_system.cpp` | 585 | override | `sub_2AEC9A` |  |  |
| `kernel/io/file_system.cpp` | 587 | call | `sub_2AEC9A` |  |  |
| `kernel/io/file_system.cpp` | 594 | override | `sub_2AF653` | XCreateFile_r |  |
| `kernel/io/file_system.cpp` | 595 | override | `sub_2ADFF0` | XReadFile_r |  |
| `kernel/io/file_system.cpp` | 596 | override | `sub_2AE0DD` | XWriteFile_r |  |
| `kernel/io/file_system.cpp` | 597 | override | `sub_2AE45E` | XGetFileSize_r |  |
| `kernel/io/file_system.cpp` | 598 | override | `sub_2AE420` | XGetFileSizeEx_r |  |
| `kernel/io/file_system.cpp` | 599 | override | `sub_2AE237` | XSetFilePointer_r |  |
| `kernel/io/file_system.cpp` | 600 | override | `sub_2AE331` | XSetFilePointerEx_r |  |
| `kernel/io/file_system.cpp` | 601 | override | `sub_2AD93C` | XFindFirstFile |  |
| `kernel/io/file_system.cpp` | 602 | override | `sub_2ADA49` | XFindNextFile |  |
| `kernel/io/file_system.cpp` | 605 | override | `sub_2AF174` | UserPathExist |  |
| `kernel/io/file_system.cpp` | 606 | override | `sub_2B04AE` | CreateSaveFolder |  |
| `kernel/io/file_system.cpp` | 609 | override | `sub_332B60` | CacheGameFiles |  |
| `misc/rich_presence/shadow/shadow.cpp` | 17 | memory | `0x54DDD8` | var g_stageid / GC: 0x805EF958 | SILENT |
| `misc/rich_presence/shadow/shadow.cpp` | 19 | memory | `0x5022A4` | var g_currentmode / GC: 0x805EC170 | SILENT |
| `misc/rich_presence/shadow/shadow.cpp` | 21 | memory | `0x5415A0` | var g_isgameplayfrozen / GC: 0x8057D768 | SILENT |
| `misc/rich_presence/shadow/shadow.cpp` | 22 | memory | `0x54DDC4` | var g_stagetimer / GC: 0x8057D734 | SILENT |
| `misc/rich_presence/shadow/shadow.cpp` | 23 | memory | `0x541808` | var g_moviemanager | SILENT |
| `misc_impl.cpp` | 36 | override | `sub_116B0` | So it's 0.045% faster than should be. Return real ones instead. |  |
| `misc_impl.cpp` | 100 | override | `sub_2AAF10` | QueryPerformanceCounterImpl |  |
| `misc_impl.cpp` | 101 | override | `sub_3D7000` | GetRdtSc |  |
| `misc_impl.cpp` | 103 | override | `sub_388458` | GetKeTickCountImpl |  |
| `misc_impl.cpp` | 105 | override | `sub_37FDA0` | memcpy |  |
| `misc_impl.cpp` | 106 | override | `sub_380F60` | memset |  |
| `misc_impl.cpp` | 108 | override | `sub_37FC10` | strcpy |  |
| `misc_impl.cpp` | 109 | override | `sub_37F1E0` | strncpy |  |
| `misc_impl.cpp` | 110 | override | `sub_37FC20` | strcat |  |
| `misc_impl.cpp` | 111 | override | `sub_380720` | strncat |  |
| `misc_impl.cpp` | 112 | override | `sub_2EC170` | _StrChr |  |
| `misc_impl.cpp` | 113 | override | `sub_2EC190` | _StrrChr |  |
| `misc_impl.cpp` | 114 | override | `sub_37EB60` | strstr_ |  |
| `misc_impl.cpp` | 115 | override | `sub_37E820` | memmove |  |
| `misc_impl.cpp` | 118 | override | `sub_3800E0` | strcmp_ |  |
| `misc_impl.cpp` | 119 | override | `sub_37DEC0` | strncmp_ |  |
| `misc_impl.cpp` | 120 | override | `sub_2EC0C0` | _stricmp |  |
| `misc_impl.cpp` | 121 | override | `sub_37FD10` | strlen |  |
| `misc_impl.cpp` | 122 | override | `sub_2EC110` | _strupr |  |
| `misc_impl.cpp` | 123 | override | `sub_2EC140` | _strlwr |  |
| `misc_impl.cpp` | 124 | override | `sub_38066D` | strtok |  |
| `misc_impl.cpp` | 128 | override | `sub_2AAEDE` | OutputDebugStringA |  |
| `mod/mod_loader.cpp` | 707 | override | `sub_8D1B0` |  |  |
| `mod/mod_loader.cpp` | 714 | call | `sub_8D1B0` |  |  |
| `mod/mod_loader.cpp` | 721 | override | `sub_38F810` |  |  |
| `mod/mod_loader.cpp` | 726 | call | `sub_38F810` |  |  |
| `mod/mod_loader.cpp` | 976 | override | `sub_1F7F0` |  |  |
| `mod/mod_loader.cpp` | 984 | call | `sub_1F7F0` |  |  |
| `mod/mod_loader.cpp` | 1045 | override | `sub_1F360` |  | COLLIDES |
| `mod/mod_loader.cpp` | 1048 | call | `sub_1F360` |  | SILENT: Heroes has sub_1F360 |
| `mod/mod_loader.cpp` | 1106 | override | `sub_1F570` |  |  |
| `mod/mod_loader.cpp` | 1143 | call | `sub_1F570` |  |  |
| `mod/mod_loader.cpp` | 1156 | call | `sub_1F470` |  |  |
| `mod/mod_loader.cpp` | 1164 | override | `sub_32A9A0` |  |  |
| `mod/mod_loader.cpp` | 1173 | call | `sub_32A9A0` |  |  |
| `mod/mod_loader.cpp` | 1179 | memory | `0x54DD8C` | load | SILENT |
| `mod/mod_loader.cpp` | 1194 | call | `sub_32A9A0` |  |  |
| `mod/mod_loader.cpp` | 1210 | call | `sub_32A9D0` |  |  |
| `os/gamelogs.cpp` | 7 | override | `sub_32EFE0` |  |  |
| `os/gamelogs.cpp` | 12 | call | `sub_32EFE0` |  |  |
| `os/gamelogs.cpp` | 17 | override | `sub_11430` |  | COLLIDES |
| `os/gamelogs.cpp` | 21 | call | `sub_11430` |  | SILENT: Heroes has sub_11430 |
| `os/gamelogs.cpp` | 25 | override | `sub_578B0` |  |  |
| `os/gamelogs.cpp` | 29 | call | `sub_578B0` |  |  |
| `os/gamelogs.cpp` | 33 | override | `sub_28AF30` |  |  |
| `os/gamelogs.cpp` | 36 | call | `sub_28AF30` |  |  |
| `ui/multiplayer_hud.cpp` | 30 | memory | `0x54DE78` | load | SILENT |
| `ui/multiplayer_hud.cpp` | 34 | memory | `0x5496E4` | load | SILENT |
| `ui/multiplayer_hud.cpp` | 55 | memory | `0x5394B8` | load | SILENT |
| `ui/multiplayer_hud.cpp` | 61 | memory | `0x4E5D88` | load | SILENT |
| `ui/multiplayer_hud.cpp` | 62 | memory | `0x4E5D8C` | load | SILENT |
| `ui/options_menu.cpp` | 136 | memory | `0x54DE78` | load | SILENT |
| `ui/options_menu.cpp` | 1380 | call | `sub_5CD00` |  |  |
| `ui/options_menu.cpp` | 1382 | call | `sub_28D8B0` |  |  |
| `ui/options_menu.cpp` | 1387 | call | `sub_55B40` |  |  |
| `ui/options_menu.cpp` | 1388 | call | `sub_25BE10` |  |  |
| `user/achievement_manager.cpp` | 324 | memory | `0x549640` | load | SILENT |
| `user/achievement_manager.cpp` | 346 | memory | `0x549640` | load | SILENT |
| `user/achievement_manager.cpp` | 390 | override | `sub_1B2C30` |  | COLLIDES |
| `user/achievement_manager.cpp` | 392 | call | `sub_1B2C30` |  | SILENT: Heroes has sub_1B2C30 |
| `user/achievement_manager.cpp` | 394 | memory | `0x549640` | load | SILENT |
| `user/achievement_manager.cpp` | 460 | override | `sub_2378B0` |  |  |
| `user/achievement_manager.cpp` | 462 | call | `sub_2378B0` |  |  |
| `user/achievement_manager.cpp` | 464 | memory | `0x54DDF4` | load | SILENT |
| `user/achievement_manager.cpp` | 478 | memory | `0x54DDDC` | load | SILENT |
| `user/achievement_manager.cpp` | 482 | memory | `0x4EB160` | load | SILENT |
| `user/achievement_manager.cpp` | 528 | override | `sub_1D0900` |  |  |
| `user/achievement_manager.cpp` | 531 | call | `sub_1D0900` |  |  |
| `user/config.cpp` | 840 | memory | `0x54DD88` | store | SILENT |
| `user/config.cpp` | 841 | memory | `0x541504` | store | SILENT |
