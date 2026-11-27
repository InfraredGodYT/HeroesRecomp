# Shadow → Heroes address matches

Generated with `python -I tools/xbe_tools/match_shadow.py --md` (method in the script's header). Rerun after changing the host, `[fixes]`, or the tool.

- **HIGH** is one of:
  - an exact masked-byte match, unique in both games
  - a "callees" match: identical bytes, told apart by already-matched call targets
  - two or more agreeing cross-references with none disagreeing
- **Checks behind HIGH:**
  - all HIGH function matches land in the same-named XBE section (D3D→D3D, DSOUND→DSOUND, .text→.text)
  - neighbouring DSOUND wrappers keep the same spacing in both games
- **CHECK:** verify by hand before use, in Ghidra or by comparing the disassembly. "layout OK" means a candidate sits where its HIGH neighbours in the same section predict. That's evidence, not proof. Fuzzy scores alone mean little; tiny wrappers all score 1.00.
- `?` means no candidate. Many of these are Shadow game code with no counterpart in Heroes (game-mode and HUD variables, achievement hooks); those get disabled, not ported.

| Where | Kind | Shadow | Heroes | Confidence | Evidence | Name / hint |
|---|---|---|---|---|---|---|
| `app.cpp:56` | override | `0x57630` | `0x3F6F0` | CHECK | fuzzy 0.05 (next 0.04 sub_5C9A0) |  |
| `app.cpp:63` | memory | `0x541FA8` | `?` | CHECK |  | load |
| `app.cpp:65` | memory | `0x541FA8` | `?` | CHECK |  | store |
| `app.cpp:68` | memory | `0x541FA8` | `?` | CHECK |  | load |
| `app.cpp:70` | memory | `0x5022A4` | `?` | CHECK |  | load |
| `app.cpp:81` | override | `0x11AD0` | `0x69A00` | CHECK | fuzzy 0.04 (next 0.02 sub_1C0B0) |  |
| `app.cpp:94` | override | `0x55BE0` | `0x1A6A0` | CHECK | fuzzy 0.07 (next 0.04 sub_119B0) |  |
| `app.cpp:98` | memory | `0x54DD8C` | `?` | CHECK |  | store |
| `audio/audio.cpp:377` | call | `0x25C750` | `0x14B90` | CHECK | fuzzy 0.79 (next 0.09 sub_14AD0) | stop all sound |
| `audio/audio.cpp:380` | memory | `0x54DD8C` | `?` | CHECK |  | store / set voice language to new value (eng or jp |
| `audio/audio.cpp:385` | call | `0x328BB0` | `0x2EFE60` | CHECK | fuzzy 0.12 (next 0.10 sub_263F70); layout OK (delta 0x38D50, neighbours 0x38640..0x101410) |  |
| `audio/audio.cpp:385` | memory | `0x51DDA0` | `?` | CHECK |  | load |
| `audio/audio.cpp:389` | memory | `0x51DD9C` | `?` | CHECK |  | load |
| `audio/audio.cpp:395` | call | `0x328B90` | `0xC26E0` | CHECK | fuzzy 0.21 (next 0.14 sub_263F70); layout OFF (delta 0x2664B0, neighbours 0x38640..0x101410) |  |
| `audio/audio.cpp:397` | call | `0x32AC10` | `0x4D070` | CHECK | fuzzy 1.00 (next 0.26 sub_672D0); layout OFF (delta 0x2DDBA0, neighbours 0x38640..0x101410) |  |
| `audio/audio.cpp:398` | call | `0x328F30` | `0x16200` | CHECK | fuzzy 0.15 (next 0.15 sub_161C0); layout OFF (delta 0x312D30, neighbours 0x38640..0x101410) |  |
| `audio/audio.cpp:405` | memory | `0x51DD80` | `?` | CHECK |  | store |
| `audio/audio.cpp:409` | override | `0x3E5A27` | `0x340B9B` | HIGH | exact | DirectSoundCreate_r |
| `audio/audio.cpp:410` | override | `0x3E5756` | `0x3408CA` | HIGH | exact | IDirectSound_CreateSoundBuffer |
| `audio/audio.cpp:411` | override | `0x3E4EE9` | `0x34005D` | HIGH | callees + xref x1 | IDirectSoundBuffer_SetBufferData |
| `audio/audio.cpp:412` | override | `0x3E421C` | `0x33F390` | HIGH | callees + xref x2 | IDirectSoundBuffer_GetStatus |
| `audio/audio.cpp:413` | override | `0x3E4274` | `0x33F3E8` | HIGH | exact | IDirectSoundBuffer_Lock |
| `audio/audio.cpp:414` | override | `0x3E34B8` | `?` | CHECK | no candidate | IDirectSoundBuffer_Unlock |
| `audio/audio.cpp:415` | override | `0x3E35B5` | `0x33E7D5` | HIGH | exact + xref x1 | CDirectSound_DownloadEffectsImage |
| `audio/audio.cpp:416` | override | `0x3E4238` | `0x33F3AC` | HIGH | callees | IDirectSoundBuffer_GetCurrentPosition |
| `audio/audio.cpp:417` | override | `0x3E4258` | `0x33F3CC` | CHECK | fuzzy 1.00 (next 1.00 sub_33F390); layout OK (delta 0xA4E8C, neighbours 0xA4E8C..0xA4E8C) | IDirectSoundBuffer_SetCurrentPosition |
| `audio/audio.cpp:418` | override | `0x3E419C` | `0x33F310` | HIGH | callees + xref x1 | IDirectSoundBuffer_Play |
| `audio/audio.cpp:419` | override | `0x3E41C0` | `0x33F334` | HIGH | exact + xref x1 | IDirectSoundBuffer_Stop |
| `audio/audio.cpp:420` | override | `0x3E41D8` | `0x33F34C` | HIGH | callees | IDirectSoundBuffer_StopEx |
| `audio/audio.cpp:421` | override | `0x3E34BD` | `0x33E6DD` | HIGH | exact + xref x1 | IDirectSoundBuffer_Release |
| `audio/audio.cpp:422` | override | `0x3E412C` | `0x33F2BC` | HIGH | callees + xref x2 | IDirectSoundBuffer_SetVolume |
| `audio/audio.cpp:423` | override | `0x3E41FC` | `0x33F370` | HIGH | callees | IDirectSoundBuffer_SetLoopRegion |
| `audio/audio.cpp:424` | override | `0x3E4148` | `0x33F2D8` | HIGH | callees | IDirectSoundBuffer_SetPitch |
| `audio/audio.cpp:425` | override | `0x3E4164` | `0x33F2F4` | HIGH | callees | IDirectSoundBuffer_SetHeadroom |
| `audio/audio.cpp:426` | override | `0x3E48FA` | `0x33FA6E` | HIGH | callees | IDirectSoundBuffer_SetFrequency |
| `audio/audio.cpp:429` | override | `0x3E497A` | `0x33FAEE` | HIGH | callees | IDirectSoundBuffer_SetMinDistance |
| `audio/audio.cpp:430` | override | `0x3E4180` | `0x33F3CC` | CHECK | fuzzy 1.00 (next 1.00 sub_33F390); layout OFF (delta 0xA4DB4, neighbours 0xA4E70..0xA4E8C) | IDirectSoundBuffer_SetMixBinVolumes_8 |
| `audio/audio.cpp:431` | override | `0x3E4916` | `0x33FA8A` | HIGH | callees | IDirectSoundBuffer_SetMixBins |
| `audio/audio.cpp:432` | override | `0x3E4956` | `0x33FACA` | HIGH | callees | IDirectSoundBuffer_SetMaxDistance |
| `audio/audio.cpp:433` | override | `0x3E4932` | `0x33FAA6` | HIGH | callees | IDirectSoundBuffer_SetConeAngles |
| `audio/audio.cpp:434` | override | `0x3E499E` | `0x33FB12` | HIGH | exact | IDirectSoundBuffer_SetPosition |
| `audio/audio.cpp:435` | override | `0x3E577A` | `0x3408EE` | HIGH | callees | IDirectSound_SetAllParameters |
| `audio/audio.cpp:436` | override | `0x3E5861` | `0x3409D5` | HIGH | callees | IDirectSound_SetI3DL2Listener |
| `audio/audio.cpp:437` | override | `0x3E579A` | `0x34090E` | HIGH | callees | IDirectSound_SetDistanceFactor |
| `audio/audio.cpp:438` | override | `0x3E583D` | `0x3409B1` | HIGH | callees | IDirectSound_SetRolloffFactor |
| `audio/audio.cpp:439` | override | `0x3E5808` | `0x34097C` | HIGH | exact | IDirectSound_SetPosition |
| `audio/audio.cpp:440` | override | `0x3E57BE` | `0x340932` | HIGH | exact | IDirectSound_SetOrientation |
| `audio/audio.cpp:441` | override | `0x3E52DC` | `0x340450` | HIGH | exact | IDirectSound_CommitDeferredSettings |
| `audio/audio.cpp:442` | override | `0x3E4FB4` | `0x340128` | HIGH | exact + xref x7 | CDirectSound_CommitDeferredSettings |
| `audio/driver/sdl3_driver.cpp:124` | override | `0x25CBB0` | `0x153A0` | HIGH | exact |  |
| `cpu/guest_thread.cpp:385` | override | `0x2ADC8D` | `0x27B51F` | HIGH | exact | GetThreadPriorityImpl |
| `cpu/guest_thread.cpp:387` | override | `0x2ADF79` | `0x27B80A` | HIGH | exact | XAPILIBGetCurrentThreadId |
| `cpu/guest_thread.cpp:389` | override | `0x2B126A` | `0x27B3D0` | HIGH | exact | Some kernel hardware check, not needed on PC. |
| `exports.cpp:10` | override | `0x55A80` | `0x397D0` | CHECK | fuzzy 0.17 (next 0.06 sub_1B950) |  |
| `exports.cpp:46` | call | `0x25BF80` | `0x120A0` | CHECK | fuzzy 0.06 (next 0.04 sub_18370) |  |
| `gpu/FixedFunction.cpp:863` | override | `0x3D0B70` | `0x32BE00` | HIGH | exact |  |
| `gpu/FixedFunction.cpp:873` | override | `0x3D08B0` | `0x32BB40` | HIGH | xref 2/2 |  |
| `gpu/FixedFunction.cpp:886` | override | `0x3CFDF0` | `0x32B080` | HIGH | exact + xref x1 | SetMaterial |
| `gpu/pixelshader.cpp:209` | override | `0x3DC790` | `0x337A50` | CHECK | fuzzy 0.91 (next 0.02 sub_13B10); layout OK (delta 0xA4D40, neighbours 0xA4D40..0xA4DE0) |  |
| `gpu/pixelshader.cpp:220` | override | `0x3DC590` | `0x337850` | HIGH | exact |  |
| `gpu/pushbuffer.cpp:153` | memory | `0x55B3C0` | `?` | CHECK |  | load |
| `gpu/pushbuffer.cpp:154` | memory | `0x55B3C4` | `?` | CHECK |  | load |
| `gpu/pushbuffer.cpp:155` | memory | `0x55B3C8` | `?` | CHECK |  | load |
| `gpu/pushbuffer.cpp:156` | memory | `0x55B3CC` | `?` | CHECK |  | load |
| `gpu/pushbuffer.cpp:221` | override | `0x3D4740` | `0x32FA00` | HIGH | exact | D3DDevice_BeginPush |
| `gpu/pushbuffer.cpp:222` | override | `0x3D4760` | `0x32FA20` | HIGH | exact | D3DDevice_EndPush |
| `gpu/pushbuffer.cpp:224` | override | `0x2C51C0` | `0x14F010` | CHECK | fuzzy 0.09 (next 0.01 sub_4DAB0); layout OFF (delta 0x1761B0, neighbours 0x28A50..0x35E9A) | _rpSkinXboxSetMaterialColor |
| `gpu/renderstate.cpp:376` | override | `0x3D2420` | `0x32D6F0` | HIGH | exact + xref x1 | D3DDevice_SetRenderState_Simple |
| `gpu/resources.cpp:20` | override | `0x3D5DB0` | `0x331070` | HIGH | exact + xref x10 | DestructResource |
| `gpu/textures.cpp:27` | override | `0x2DF9F0` | `0x2B6FA0` | HIGH | exact |  |
| `gpu/textures.cpp:490` | override | `0x2ED0D0` | `0x2B8920` | CHECK | fuzzy 0.04 (next 0.04 sub_28AE00); layout OFF (delta 0x347B0, neighbours 0x38640..0x101410) |  |
| `gpu/textures.cpp:533` | override | `0x2DF870` | `0x2B6E20` | CHECK | fuzzy 0.91 (next 0.06 sub_2B7830); layout OK (delta 0x28A50, neighbours 0x28A50..0x35E9A) |  |
| `gpu/textures.cpp:545` | override | `0x40B363` | `0x35E848` | CHECK | fuzzy 0.94 (next 0.11 sub_35EF99) |  |
| `gpu/textures.cpp:596` | override | `0x2DFE20` | `0x2B7430` | CHECK | fuzzy 0.09 (next 0.03 sub_ADA60); layout OFF (delta 0x289F0, neighbours 0x28A50..0x34140) |  |
| `gpu/textures.cpp:642` | override | `0x3D0CD0` | `0x32BF60` | HIGH | exact + xref x6 | SetTexture |
| `gpu/textures.cpp:643` | override | `0x3DD710` | `0x338930` | CHECK | fuzzy 0.89 (next 0.04 sub_32F780); layout OK (delta 0xA4DE0, neighbours 0xA4D40..0xA4DE0) | CreateTexture_ |
| `gpu/textures.cpp:644` | override | `0x3DD810` | `0x338A30` | CHECK | fuzzy 0.58 (next 0.33 sub_2A7040); layout OK (delta 0xA4DE0, neighbours 0xA4D40..0xA4DE0) | LockTextureRect |
| `gpu/vertexbuffer.cpp:57` | override | `0x2E6750` | `0x2B2690` | CHECK | fuzzy 0.23 (next 0.05 sub_3260D0); layout OFF (delta 0x340C0, neighbours 0x34140..0x38640) |  |
| `gpu/vertexbuffer.cpp:72` | override | `0x2E67D0` | `0x2B2710` | CHECK | fuzzy 0.11 (next 0.07 sub_2B2060); layout OFF (delta 0x340C0, neighbours 0x34140..0x38640) |  |
| `gpu/vertexbuffer.cpp:76` | memory | `0x55F8F0` | `?` | CHECK |  | load |
| `gpu/vertexbuffer.cpp:89` | override | `0x3D5FE0` | `0x3312A0` | HIGH | exact |  |
| `gpu/vertexbuffer.cpp:102` | override | `0x3D1E10` | `0x32D0E0` | HIGH | exact |  |
| `gpu/vertexshader.cpp:838` | override | `0x3D2010` | `0x32D2E0` | HIGH | exact + xref x3 |  |
| `gpu/vertexshader.cpp:947` | call | `0x3D2280` | `0x32D550` | HIGH | exact + xref x1 |  |
| `gpu/vertexshader.cpp:955` | override | `0x3D1F70` | `0x32D240` | HIGH | exact + xref x1 | SelectVertexShader |
| `gpu/vertexshader.cpp:956` | override | `0x3D1F00` | `0x32D1D0` | HIGH | exact | LoadVertexShaderProgram |
| `gpu/vertexshader.cpp:957` | override | `0x3D1EA0` | `0x32D170` | HIGH | exact + xref x1 | LoadVertexShader |
| `gpu/vertexshader.cpp:959` | override | `0x3D1CE0` | `0x32CFB0` | HIGH | exact | D3DDevice_SetVertexShaderConstant1Fast |
| `gpu/vertexshader.cpp:960` | override | `0x3D1D30` | `0x32D000` | HIGH | exact | D3DDevice_SetVertexShaderConstantNotInlineFast |
| `gpu/video.cpp:3560` | override | `0x2DDC00` | `0x2AD850` | CHECK | fuzzy 0.57 (next 0.11 sub_273D10); layout OK (delta 0x303B0, neighbours 0x28A50..0x35E9A) |  |
| `gpu/video.cpp:4167` | call | `0x3DA010` | `0x3352D0` | HIGH | exact + xref x1 |  |
| `gpu/video.cpp:4373` | call | `0x3DDAB0` | `0x338CD0` | HIGH | exact |  |
| `gpu/video.cpp:4925` | override | `0x3D5140` | `0x330400` | CHECK | fuzzy 0.93 (next 0.01 sub_60860); layout OK (delta 0xA4D40, neighbours 0xA4D40..0xA4D40) |  |
| `gpu/video.cpp:4959` | call | `0x3D0040` | `0x32B2D0` | HIGH | exact + xref x2 |  |
| `gpu/video.cpp:5050` | call | `0x3D0680` | `0x32B910` | HIGH | exact + xref x2 |  |
| `gpu/video.cpp:5206` | override | `0x3CFCD0` | `0x32AF60` | HIGH | exact + xref x2 |  |
| `gpu/video.cpp:6395` | override | `0x3D5F10` | `0x3311D0` | HIGH | exact + xref x6 |  |
| `gpu/video.cpp:6411` | override | `0x2E6140` | `0x2B2000` | HIGH | xref 2/2 |  |
| `gpu/video.cpp:6435` | override | `0x3DA0A0` | `0x335360` | CHECK | fuzzy 0.82 (next 0.03 sub_6DB70); layout OK (delta 0xA4D40, neighbours 0xA4D40..0xA4D40) |  |
| `gpu/video.cpp:6703` | override | `0x3DD950` | `0x338B70` | HIGH | exact | SetResolution |
| `gpu/video.cpp:6711` | override | `0x3D6BF0` | `0x331EB0` | HIGH | exact | D3DDevice_Clear |
| `gpu/video.cpp:6712` | override | `0x3DC1F0` | `0x3374B0` | HIGH | exact + xref x1 | DrawIndexedPrimitive |
| `gpu/video.cpp:6713` | override | `0x3DC150` | `0x337410` | CHECK | fuzzy 0.87 (next 0.04 sub_110B0); layout OK (delta 0xA4D40, neighbours 0xA4D40..0xA4D40) | DrawPrimitive |
| `gpu/video.cpp:6714` | override | `0x3DBFE0` | `0x3372A0` | HIGH | exact + xref x1 | DrawPrimitiveUP |
| `gpu/video.cpp:6715` | override | `0x3DD560` | `0x338780` | CHECK | fuzzy 0.72 (next 0.05 sub_18320); layout OK (delta 0xA4DE0, neighbours 0xA4D40..0xA4DE0) | D3DDevice_Swap |
| `gpu/video.cpp:6720` | override | `0x3CFE70` | `0x32B100` | HIGH | exact + xref x1 | CRI_BlockUntilVerticalBlank |
| `gpu/video.cpp:6723` | override | `0x3D3240` | `0x32E510` | HIGH | exact | D3D_CommonSetDebugRegisters |
| `gpu/video.cpp:6724` | override | `0x3D0F10` | `0x32C1A0` | HIGH | exact + xref x2 | D3Device_IsBusy |
| `gpu/video.cpp:6725` | override | `0x3D4720` | `0x32F9E0` | HIGH | exact | D3DDevice_IsFencePending |
| `gpu/video.cpp:6726` | override | `0x3D5F60` | `0x331220` | HIGH | callees | D3DResource_BlockUntilNotBusy |
| `gpu/video.cpp:6727` | override | `0x3D4950` | `0x32FC10` | HIGH | exact + xref x7 | D3D_BlockOnTime |
| `gpu/video.cpp:6728` | override | `0x3DD8B0` | `0x338AD0` | HIGH | exact | D3D_GetAdapterModeCount |
| `gpu/video.cpp:6729` | override | `0x3D0F40` | `0x32C210` | HIGH | xref 2/2 | D3D_SetTileNoWait |
| `input/input.cpp:954` | memory | `0x559984` | `?` | CHECK |  | store |
| `input/input.cpp:961` | override | `0x4248A6` | `0x3800A5` | HIGH | exact | input |
| `input/input.cpp:962` | override | `0x424919` | `0x380118` | HIGH | exact | input |
| `input/input.cpp:963` | override | `0x2AAC20` | `0x16050` | CHECK | fuzzy 0.06 (next 0.06 sub_163B0); layout OFF (delta 0x294BD0, neighbours 0x-781A4..0x247810) | XInputOpen |
| `kernel/heap.cpp:186` | override | `0x2ABFCB` | `0x276C91` | CHECK | fuzzy 0.57 (next 0.02 sub_88960); layout OFF (delta 0x3533A, neighbours 0x32A6F..0x352CF) | AllocateHeap_r |
| `kernel/heap.cpp:188` | override | `0x2AC3EF` | `0x2770B5` | CHECK | xref 1/1; layout OFF (delta 0x3533A, neighbours 0x32A6F..0x352CF) | RtlAllocateHeap |
| `kernel/heap.cpp:189` | override | `0x2ACD5E` | `0x277A24` | CHECK | xref 1/1; layout OFF (delta 0x3533A, neighbours 0x32A6F..0x352CF) | RtlReAllocateHeap |
| `kernel/heap.cpp:190` | override | `0x2ADA9A` | `0x27B573` | CHECK | fuzzy 0.09 (next 0.08 sub_27B4CD); layout OFF (delta 0x32527, neighbours 0x3276E..0x3359B) | XAPILIB_VirtualAlloc_r |
| `kernel/heap.cpp:193` | override | `0x2B0494` | `0x27A035` | HIGH | exact + xref x53 | XAPILIB_XMemFree_r |
| `kernel/heap.cpp:194` | override | `0x2ACB6A` | `0x277830` | CHECK | xref 1/1; layout OFF (delta 0x3533A, neighbours 0x32A6F..0x352CF) | FreeBlock |
| `kernel/heap.cpp:195` | override | `0x2ADAC8` | `0x27B573` | CHECK | fuzzy 0.09 (next 0.08 sub_27B4CD); layout OFF (delta 0x32555, neighbours 0x3276E..0x3359B) | XAPILIB_VirtualFree_r |
| `kernel/heap.cpp:197` | override | `0x2AD48C` | `0x2781BD` | HIGH | exact | RtlFreeHeap |
| `kernel/imports.cpp:1326` | override | `0x3D7B81` | `0x332E41` | HIGH | exact |  |
| `kernel/imports.cpp:1442` | override | `0x2ADB04` | `0x278425` | CHECK | fuzzy 0.18 (next 0.10 sub_27BD4C); layout OFF (delta 0x356DF, neighbours 0x3276E..0x3359B) | XGetLanguage |
| `kernel/imports.cpp:1445` | override | `0x2B17E8` | `0x27B817` | CHECK | fuzzy 0.26 (next 0.03 sub_1BBF0); layout OFF (delta 0x35FD1, neighbours 0x28A50..0x35E9A) | XAutoPowerDownResetTimer |
| `kernel/io/file_system.cpp:574` | override | `0x2A4420` | `0x26D540` | CHECK | xref 1/1; layout OK (delta 0x36EE0, neighbours 0x-781A4..0x247810) |  |
| `kernel/io/file_system.cpp:585` | override | `0x2AEC9A` | `0x278CC7` | CHECK | fuzzy 0.83 (next 0.04 sub_278F2A); layout OFF (delta 0x35FD3, neighbours 0x38AEA..0x39CA4) |  |
| `kernel/io/file_system.cpp:594` | override | `0x2AF653` | `0x2759AF` | HIGH | xref 3/3 | XCreateFile_r |
| `kernel/io/file_system.cpp:595` | override | `0x2ADFF0` | `0x275506` | HIGH | xref 4/4 | XReadFile_r |
| `kernel/io/file_system.cpp:596` | override | `0x2AE0DD` | `0x2755F3` | CHECK | fuzzy 0.92 (next 0.37 sub_275506); layout OK (delta 0x38AEA, neighbours 0x38AEA..0x38AEA) | XWriteFile_r |
| `kernel/io/file_system.cpp:597` | override | `0x2AE45E` | `0x275974` | CHECK | fuzzy 0.62 (next 0.08 sub_27590F); layout OK (delta 0x38AEA, neighbours 0x38AEA..0x39CA4) | XGetFileSize_r |
| `kernel/io/file_system.cpp:598` | override | `0x2AE420` | `0x275936` | CHECK | fuzzy 0.75 (next 0.07 sub_27A0BB); layout OK (delta 0x38AEA, neighbours 0x38AEA..0x39CA4) | XGetFileSizeEx_r |
| `kernel/io/file_system.cpp:599` | override | `0x2AE237` | `0x27574D` | HIGH | xref 3/3 | XSetFilePointer_r |
| `kernel/io/file_system.cpp:600` | override | `0x2AE331` | `0x275847` | CHECK | fuzzy 0.84 (next 0.09 sub_27574D); layout OK (delta 0x38AEA, neighbours 0x38AEA..0x39CA4) | XSetFilePointerEx_r |
| `kernel/io/file_system.cpp:601` | override | `0x2AD93C` | `0x27A3A1` | CHECK | fuzzy 0.88 (next 0.03 sub_27A4AE); layout OK (delta 0x3359B, neighbours 0x3359B..0x352CF) | XFindFirstFile |
| `kernel/io/file_system.cpp:602` | override | `0x2ADA49` | `0x27A4AE` | HIGH | exact | XFindNextFile |
| `kernel/io/file_system.cpp:605` | override | `0x2AF174` | `0x2791A1` | CHECK | fuzzy 0.69 (next 0.06 sub_27A4FF); layout OFF (delta 0x35FD3, neighbours 0x38AEA..0x39CA4) | UserPathExist |
| `kernel/io/file_system.cpp:606` | override | `0x2B04AE` | `0x27A04F` | CHECK | fuzzy 0.82 (next 0.14 sub_27B573); layout OK (delta 0x3645F, neighbours 0x35E9A..0x3645F) | CreateSaveFolder |
| `kernel/io/file_system.cpp:609` | override | `0x332B60` | `0xFC2C0` | CHECK | fuzzy 0.03 (next 0.03 sub_1BAA0); layout OFF (delta 0x2368A0, neighbours 0x38640..0x101410) | CacheGameFiles |
| `misc/rich_presence/shadow/shadow.cpp:17` | memory | `0x54DDD8` | `?` | CHECK |  | var g_stageid / GC: 0x805EF958 |
| `misc/rich_presence/shadow/shadow.cpp:19` | memory | `0x5022A4` | `?` | CHECK |  | var g_currentmode / GC: 0x805EC170 |
| `misc/rich_presence/shadow/shadow.cpp:21` | memory | `0x5415A0` | `?` | CHECK |  | var g_isgameplayfrozen / GC: 0x8057D768 |
| `misc/rich_presence/shadow/shadow.cpp:22` | memory | `0x54DDC4` | `?` | CHECK |  | var g_stagetimer / GC: 0x8057D734 |
| `misc/rich_presence/shadow/shadow.cpp:23` | memory | `0x541808` | `?` | CHECK |  | var g_moviemanager |
| `misc_impl.cpp:36` | override | `0x116B0` | `0xD6C60` | CHECK | fuzzy 0.14 (next 0.06 sub_BA6F0) | So it's 0.045% faster than should be. Return real  |
| `misc_impl.cpp:100` | override | `0x2AAF10` | `0x2784A1` | HIGH | exact + xref x1 | QueryPerformanceCounterImpl |
| `misc_impl.cpp:101` | override | `0x3D7000` | `0x3322C0` | CHECK | exact; xref disagrees (3D7000 x1); layout OK (delta 0xA4D40, neighbours 0xA4D40..0xA4D40) | GetRdtSc |
| `misc_impl.cpp:103` | override | `0x388458` | `?` | CHECK | no candidate | GetKeTickCountImpl |
| `misc_impl.cpp:105` | override | `0x37FDA0` | `0x27C500` | CHECK | fuzzy 0.93 (next 0.01 sub_1DA300); layout OFF (delta 0x1038A0, neighbours 0x5C160..0x5CA50) | memcpy |
| `misc_impl.cpp:106` | override | `0x380F60` | `?` | CHECK | no candidate | memset |
| `misc_impl.cpp:108` | override | `0x37FC10` | `0x323E10` | HIGH | exact + xref x1 | strcpy |
| `misc_impl.cpp:109` | override | `0x37F1E0` | `0x27BEA0` | HIGH | xref 7/7 | strncpy |
| `misc_impl.cpp:110` | override | `0x37FC20` | `0x323E20` | HIGH | exact + xref x1 | strcat |
| `misc_impl.cpp:111` | override | `0x380720` | `0x323CD0` | HIGH | xref 3/3 | strncat |
| `misc_impl.cpp:112` | override | `0x2EC170` | `0x2B3B30` | HIGH | exact + xref x1 | _StrChr |
| `misc_impl.cpp:113` | override | `0x2EC190` | `0x2B3B50` | HIGH | exact + xref x1 | _StrrChr |
| `misc_impl.cpp:114` | override | `0x37EB60` | `0x27CCA0` | CHECK | xref 1/1; layout OFF (delta 0x101EC0, neighbours 0x102320..0x103340) | strstr_ |
| `misc_impl.cpp:115` | override | `0x37E820` | `0x27C500` | HIGH | exact + xref x5 | memmove |
| `misc_impl.cpp:118` | override | `0x3800E0` | `0x323C40` | CHECK | xref 1/1; layout OK (delta 0x5C4A0, neighbours 0x5C160..0x5CA50) | strcmp_ |
| `misc_impl.cpp:119` | override | `0x37DEC0` | `0x27CAB0` | HIGH | xref 27/27 | strncmp_ |
| `misc_impl.cpp:120` | override | `0x2EC0C0` | `0x2B3A80` | HIGH | exact + xref x1 | _stricmp |
| `misc_impl.cpp:121` | override | `0x37FD10` | `0x323BB0` | HIGH | exact + xref x1 | strlen |
| `misc_impl.cpp:122` | override | `0x2EC110` | `0x2B3AD0` | HIGH | exact + xref x1 | _strupr |
| `misc_impl.cpp:123` | override | `0x2EC140` | `0x2B3B00` | HIGH | exact + xref x1 | _strlwr |
| `misc_impl.cpp:124` | override | `0x38066D` | `0x323B08` | CHECK | xref 1/1; layout OFF (delta 0x5CB65, neighbours 0x5C160..0x5CA50) | strtok |
| `misc_impl.cpp:128` | override | `0x2AAEDE` | `0x323082` | HIGH | exact + xref x1 | OutputDebugStringA |
| `mod/mod_loader.cpp:707` | override | `0x8D1B0` | `0x15830` | CHECK | fuzzy 0.11 (next 0.08 sub_1A260) |  |
| `mod/mod_loader.cpp:721` | override | `0x38F810` | `0x276CEC` | CHECK | fuzzy 0.01 (next 0.01 sub_18980) |  |
| `mod/mod_loader.cpp:976` | override | `0x1F7F0` | `0x169310` | CHECK | fuzzy 0.02 (next 0.01 sub_2EDE0) |  |
| `mod/mod_loader.cpp:1045` | override | `0x1F360` | `0x3B760` | CHECK | fuzzy 0.01 (next 0.01 sub_11E50) |  |
| `mod/mod_loader.cpp:1106` | override | `0x1F570` | `0x50A50` | CHECK | fuzzy 0.03 (next 0.02 sub_6CCD0) |  |
| `mod/mod_loader.cpp:1156` | call | `0x1F470` | `0x180A30` | CHECK | fuzzy 0.08 (next 0.07 sub_19B280) |  |
| `mod/mod_loader.cpp:1164` | override | `0x32A9A0` | `0x13F90` | CHECK | fuzzy 0.22 (next 0.22 sub_13F40); layout OFF (delta 0x316A10, neighbours 0x38640..0x101410) |  |
| `mod/mod_loader.cpp:1179` | memory | `0x54DD8C` | `?` | CHECK |  | load |
| `mod/mod_loader.cpp:1210` | call | `0x32A9D0` | `0x2EEDD0` | CHECK | xref 1/1; layout OK (delta 0x3BC00, neighbours 0x38640..0x101410) |  |
| `os/gamelogs.cpp:7` | override | `0x32EFE0` | `0x69830` | CHECK | fuzzy 0.08 (next 0.04 sub_1361B0); layout OFF (delta 0x2C57B0, neighbours 0x38640..0x101410) |  |
| `os/gamelogs.cpp:17` | override | `0x11430` | `0x67A80` | CHECK | fuzzy 0.17 (next 0.05 sub_6EC90) |  |
| `os/gamelogs.cpp:25` | override | `0x578B0` | `0x12A40` | CHECK | fuzzy 0.02 (next 0.02 sub_3E2A0) |  |
| `os/gamelogs.cpp:33` | override | `0x28AF30` | `0x1C4C0` | CHECK | fuzzy 0.04 (next 0.03 sub_18980); layout OFF (delta 0x26EA70, neighbours 0x-781A4..0x247810) |  |
| `ui/multiplayer_hud.cpp:30` | memory | `0x54DE78` | `?` | CHECK |  | load |
| `ui/multiplayer_hud.cpp:34` | memory | `0x5496E4` | `?` | CHECK |  | load |
| `ui/multiplayer_hud.cpp:55` | memory | `0x5394B8` | `?` | CHECK |  | load |
| `ui/multiplayer_hud.cpp:61` | memory | `0x4E5D88` | `?` | CHECK |  | load |
| `ui/multiplayer_hud.cpp:62` | memory | `0x4E5D8C` | `?` | CHECK |  | load |
| `ui/options_menu.cpp:136` | memory | `0x54DE78` | `?` | CHECK |  | load |
| `ui/options_menu.cpp:1380` | call | `0x5CD00` | `0x1A720` | CHECK | fuzzy 0.47 (next 0.31 sub_39710) |  |
| `ui/options_menu.cpp:1382` | call | `0x28D8B0` | `0x1A720` | CHECK | fuzzy 0.07 (next 0.04 sub_14590); layout OFF (delta 0x273190, neighbours 0x-781A4..0x247810) |  |
| `ui/options_menu.cpp:1387` | call | `0x55B40` | `0x1A720` | CHECK | fuzzy 0.47 (next 0.31 sub_39710) |  |
| `ui/options_menu.cpp:1388` | call | `0x25BE10` | `0x2B760` | CHECK | fuzzy 0.04 (next 0.04 sub_2B6C0) |  |
| `user/achievement_manager.cpp:324` | memory | `0x549640` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:346` | memory | `0x549640` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:390` | override | `0x1B2C30` | `0x1A720` | CHECK | fuzzy 0.07 (next 0.01 sub_1C0B0) |  |
| `user/achievement_manager.cpp:394` | memory | `0x549640` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:460` | override | `0x2378B0` | `0x1A5C0` | CHECK | fuzzy 0.05 (next 0.04 sub_19E00) |  |
| `user/achievement_manager.cpp:464` | memory | `0x54DDF4` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:478` | memory | `0x54DDDC` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:482` | memory | `0x4EB160` | `?` | CHECK |  | load |
| `user/achievement_manager.cpp:528` | override | `0x1D0900` | `0x6A800` | CHECK | fuzzy 0.02 (next 0.01 sub_1E830) |  |
| `user/config.cpp:840` | memory | `0x54DD88` | `?` | CHECK |  | store |
| `user/config.cpp:841` | memory | `0x541504` | `?` | CHECK |  | store |
| `app.cpp` | midasm | `0x2AFBBC` | `?` | CHECK | containing sub_2AFB53 not matched exactly; fuzzy sub_27972B (0.82) -> offset unknown | UpdateSDLEventsMidAsmHook |
| `gpu/pushbuffer.cpp` | midasm | `0x2DE00B` | `?` | CHECK | containing sub_2DDC50 not matched exactly; fuzzy sub_2AD8B0 (0.68) -> offset unknown | ClearWithQuadsMidAsmHook |
| `gpu/pushbuffer.cpp` | midasm | `0x2ED959` | `?` | CHECK | containing sub_2ED400 not matched exactly; fuzzy sub_2B8D70 (0.11) -> offset unknown | RasterRenderQuadMidAsmHook |
| `gpu/pushbuffer.cpp` | midasm | `0x2F1056` | `?` | CHECK | containing sub_2F0CF0 not matched exactly; fuzzy sub_2BC0E0 (0.99) -> offset unknown | _XbSubmitNodeMidAsmHook |
| `kernel/io/file_system.cpp` | midasm | `0x2AEE1C` | `?` | CHECK | containing sub_2AEC9A not matched exactly; fuzzy sub_278CC7 (0.83) -> offset unknown | CreateSaveMetaMidAsmHook |
| `gpu/video.cpp` | midasm | `0x2E0562` | `?` | CHECK | containing sub_2E04C0 not matched exactly; fuzzy sub_2A56B0 (0.09) -> offset unknown | DestructResourceMidAsmHook |
| `user/achievement_manager.cpp` | midasm | `0x23D620` | `?` | CHECK | containing sub_23D560 not matched exactly; fuzzy sub_622D0 (0.02) -> offset unknown | SetEndingAchievementMidAsmHook |
| `user/achievement_manager.cpp` | midasm | `0x237CD4` | `?` | CHECK | containing sub_237950 not matched exactly; fuzzy sub_1ADB0 (0.01) -> offset unknown | UpdateARankCountMidAsmHook |
| `user/achievement_manager.cpp` | midasm | `0x16EF87` | `?` | CHECK | containing sub_16EDE0 not matched exactly; fuzzy sub_6BA00 (0.03) -> offset unknown | StageActionGoalActionMidAsmHook |

Summary: call CHECK: 12, call HIGH: 5, memory CHECK: 35, midasm CHECK: 9, override CHECK: 62, override HIGH: 89
