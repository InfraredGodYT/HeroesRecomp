# Heroes Recompiled

Heroes Recompiled is an unofficial, work-in-progress PC port of the original Xbox version of Sonic Heroes (2003), made through static recompilation.

It is a fork of [Shadow Recompiled](https://github.com/FoxWorkshop/ShadowRecomp), which ported Shadow the Hedgehog the same way. [XboxRecomp2] turns the game's original x86 code into C++, and Shadow Recompiled's runtime provides graphics, audio, input, the installer and mod loading.

**This project does not include any game assets. You must provide the files from your own legally acquired copy of the game (Xbox, NTSC-U) to install or build Heroes Recompiled.**

> [!WARNING]
> **Early development: the game does not boot yet.** There are no releases.

## Status

- [x] Heroes' `default.xbe` recompiles to C++ with XboxRecomp2
- [ ] Rename and remove Shadow-specific code
- [ ] First build, then boot: title screen, Story Select, first stage
- [ ] Mod support through [SA Mod Manager](https://github.com/X-Hax/SA-Mod-Manager): file mods and built-in codes first, then code mods
- [ ] Opt-in improvements: widescreen, high frame rate, fixes

The original behavior is always the default. Fixes and improvements are opt-in.

## Building

See [docs/BUILDING.md](docs/BUILDING.md). That file is inherited from Shadow Recompiled and is not yet updated for Heroes.

## License

GPL-3.0, like Shadow Recompiled. See [LICENSE](LICENSE).

## Credits

### Heroes Recompiled

- [InfraredGodYT](https://github.com/InfraredGodYT): Heroes port.

### Based on Shadow Recompiled

Heroes Recompiled is built on the work of the Shadow Recompiled team. The following credits are theirs, kept as they wrote them:

- [Sora](https://github.com/Sora-yx): Creator and Lead Developer of the recompilation, as well as the developer of technologies created for it such as [XboxRecomp](https://github.com/FoxWorkshop/XboxRecomp) (before the XboxRecomp2 switch) and [XShaderRecomp](https://github.com/FoxWorkshop/XShaderRecomp). Provided French localization for the custom menus. Final design decisions and UI for installer, achievements, and other custom menus.

- [dreamsyntax](https://github.com/dreamsyntax): Second Developer, source for Shadow knowledge, ensuring port accuracy, and identifying flaws with the Xbox port of the game. Provided English localization for the custom menus.

- [Sewer56](https://github.com/Sewer56): Third Developer. Added nix support, Discord RPC, implemented better widescreen, better framepacing and >4x'd performance with shaders and [XboxRecomp2].

- [TheHatedGravity](https://github.com/TheHatedGravity): First installer design, UI polish, spotting visual flaws

- [DonutStopGaming](https://github.com/DonutStopGaming): Help with understanding RenderWare and finding Shadow functions.

- [Jesus_PK](https://github.com/Jesus-PK): Provided Spanish localization for the custom menus, also helped testing and finding bugs.

- [Deep-Sea Berry](https://github.com/Deep-SeaBerry): Artist behind the project logo, along with custom options icons. Provided German localization for the custom menus and helped testing and finding bugs.

- [Emasplatter](https://twitter.com/emasplatter): Provided Italian localization for the custom menus.

- [Windii](https://www.youtube.com/@WindiiGitlord/videos): Provided Japanese localization for the custom menus.

- [Kondrashov's Lair](https://www.youtube.com/channel/UCUU-r-7kVY3ig-GY7KTFvlw): Produced the [installer music](https://www.youtube.com/watch?v=7_pwiwb9V14) (original prod. by Sins Of A Divine Mother).

#### Tester Thanks
- [Rawlins]
- [DarkShadow17]
- [The Spectral Star]
- [Pixel The Retro]
- [ultra0000]
- Many others who helped briefly test

#### Special Thanks

- CXBX-Reloaded Development Team: This project would not have been possible without the [CXBX emulator](https://github.com/Cxbx-Reloaded/Cxbx-Reloaded-legacy). Nearly every aspect of the development was guided by referencing CXBX's research on the original Xbox, especially the graphics part.

- [extract-xiso](https://github.com/XboxDev/extract-xiso): extract-xiso devs (in@fishtank.com and other extract-xiso contributors) for core extraction in the installer.

- Unleashed Recompiled Development Team: Created much of the ground work that made all of this possible and sped up development time considerably.

- [Marathon Recompiled](https://github.com/sonicnext-dev/MarathonRecomp): Used as a reference for various features and help with debugging.

- [Skyth](https://github.com/blueskythlikesclouds): Provided help to understand how Recompilation works, also the author of the original graphics pipeline code which was (mostly) reused here.

- [Darío](https://github.com/DarioSamo): Creator of the graphics hardware abstraction layer [plume](https://github.com/renderbag/plume), used by the project's graphics backend.

- [MattKC](https://github.com/itsmattkc): Useful explanation of Xbox's Push Buffers system on [his blog with Reburn3.](https://forum.mattkc.com/viewtopic.php?t=154)

- [ocornut](https://github.com/ocornut): Creator of Dear ImGui, which is used as the backbone of the custom menus.

[XboxRecomp2]: https://sewer56.dev/XboxRecomp2
[ultra0000]: https://github.com/ultra0000
[Rawlins]: https://www.youtube.com/channel/UCjbUqRzVay-6HjoJjODre5w
[DarkShadow17]: https://www.youtube.com/@DarkShadowtH
[The Spectral Star]: https://www.youtube.com/@the-spectral-star5086
[Pixel The Retro]: https://www.youtube.com/@PixelTheRetro
