# Credits

OpenFrame is a small layer on a very large amount of other people's work. This file says whose, and for what. Credits distinguish code that is in this repository, code ported into it, and work that only informed it. They do not imply that anyone listed endorses OpenFrame or that every feature of their project is here. If a credit is missing or wrong, open an issue.

## The game

- **Bungie**, and Microsoft as the rights holder: Halo: Combat Evolved, its code, art, audio, maps and story. OpenFrame contains none of the game's data. You supply the maps from your own Xbox disc.

## The decompilation and its lineage

The game's code is the Xbox build 2342 (`cachebeta.exe`), decompiled to C by the community.

- **[punpckhdq/halo](https://github.com/punpckhdq/halo)** (punpckhdq) and its contributors: the Xbox Halo: Combat Evolved decompilation that everything below builds on. CC0 before 17 October 2026, AGPL-3.0 from then.
- **[bnunu/halo-1](https://github.com/bnunu/halo-1)** (bnunu), a fork of the above, CC0: the decompilation OpenCE starts from.
- **[cybersecurity/halo-ce-universal](https://github.com/cybersecurity/halo-ce-universal)** (MrBruh, "cybersecurity") and its contributors: the first native Linux, Windows and Android port of the decompilation, with the guest and host architecture, the renderer, the netcode and the launcher.
- **[OpenCommunityEdition/OpenCE](https://github.com/OpenCommunityEdition/OpenCE)** (Open Community Edition), the project that halo-ce-universal became, and its contributors, among them MrBruh, MrMilenko, saulob, chestahh, eltanschauung, hkmodd, pugzysixtyfour and bnunu: the port OpenH1 forks. OpenH1 keeps OpenCE's netcode, so OpenFrame's players can play with OpenCE's.
- **startupfoundry**: OpenCE pull requests #71, native 64-bit Arm Linux, and #85, OpenXR VR on the Steam Frame, the earlier work, which OpenH1 continues and OpenFrame packages.

## OpenH1

**[OpenH1](https://github.com/OpenH1/OpenH1)** is the code OpenFrame is a fork of (the release tag `build-769`; OpenFrame began at `build-734`): the macOS, Windows, Linux and Android ports, the Vulkan and Metal renderers, the arm64 guest/host architecture, the new netcode, internet play, the mods system and, for this project, the VR mode: the OpenXR host, the hand, weapon and vehicle interactions, the visor HUD, foveation and the Steam Frame install. Thank you to the OpenH1 team and its contributors. Their handles, from the repository's `AUTHORS.md` and history, include erincena, g0atbutt, MrBruh, MrMilenko, xshxdex98, punpckhdq, bnunu, justinfilip, eltanschauung, Mootjuh, hkmodd, Berthalamew, chestahh, saulob, BirchWoodGod, thelinkin3000, RockBooch, WhiteBulL, jcompto95-tech and many more: run `git shortlog -sn` for the full list. Named in `AUTHORS.md` for art in the game:

- Paul Hontz: the loading screen.
- Erin Cena: the OpenH1 mark, icon and Steam library artwork of OpenH1 (not used here; see `steam-frame/art/README.md`).
- saulob: the redrawn A, B, X and Y buttons.
- chestahh: the HUD button styles.
- MrBruh: the high-res HUD, based on reference photos by Aerocatia.
- Aerocatia and Jessie: the weapons' and skull's HUD pictures, from [Halo PC Restored](https://github.com/Aerocatia/halopc-restored) (not under OpenH1's licence; see `port/assets/hud/NOTICE.md`), taken through ChupathingyCE's work by MrMilenko.

## Halo VR mods

- **[astromaddie/HaloCE-VR](https://github.com/astromaddie/HaloCE-VR)** (astromaddie): the OpenXR VR foundation for Halo on the Steam Frame, released as CC0: stereo rendering, tracked inputs, hand aiming, arm IK, scopes, vehicles, room-scale and VR menus, and the Frame findings (the Frame's controller profile, Lepton, Mesa's trace markers). It is the base of the fork below and the earliest proof that this works on the Frame.
- **[moistman42069/HaloCE-DecompRemix-VR-PC-Android](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android)** (moistman42069), AGPL-3.0-or-later (CC0 before release 1.0.37): a Quest, PICO and Steam Frame fork of OpenCE with VR gameplay features. OpenFrame ports these from it into OpenH1's VR mode, rewritten for OpenH1's code, with attribution in the commits:

  - Bullet time
  - Sprint, by toggle or arm swing
  - The comfort vignette on foot
  - Third person on foot
  - Slide, wall jump and wall run
  - Fists: punch, stagger and parry
  - Weapon pull, prop throw and hand swap
  - The wrist HUD and HUD modes

  That fork's own credits (to LivingFray's HaloCEVR, to the OpenCE fork by jcompto95-tech, to Mjolnir VR by Cesarus85 for the Frame findings, to ChupathingyCE and others) are in its `CREDITS.md`; they carry over to what is ported.
- **[thelinkin3000/halo-ce-universal](https://github.com/thelinkin3000/halo-ce-universal)** (thelinkin3000): an OpenCE fork with Android, Turnip, Nintendo Switch and PlayStation 5 ports. Prior Turnip work on the road to this one, and a contributor to OpenH1.
- **[ClassicWasTaken/HaloSteamFrameMod](https://github.com/ClassicWasTaken/HaloSteamFrameMod)** (ClassicWasTaken): earlier work on Halo on the Steam Frame. Context only; not used here.

## Libraries and tools

[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md) has the full list with licences. In short:

- **SDL3**, by Sam Lantinga and the SDL contributors (zlib licence): windowing, input, audio and the Vulkan surface, built from source and shipped as `libSDL3.so.0`.
- **OpenXR SDK**, by The Khronos Group (Apache-2.0 / MIT): the headers VR is built against. The game loads the runtime itself.
- **Vulkan headers**, by The Khronos Group.
- **LLVM and Clang** (Apache-2.0 with LLVM exceptions): the pinned toolchain that compiles the guest image.
- **musl libc** and its maths functions, by Rich Felker and contributors (MIT): the guest's C runtime and the deterministic maths.
- **Mbed TLS** (Arm and contributors), **Monocypher** (Loup Vaillant and contributors), **KCP** (Lin Wei), **miniupnpc** (Thomas Bernard), **Opus** (Xiph.Org Foundation and contributors), **Expat**, **zlib**, **tomlc17** (CK Tan), **jsmn** (Serge Zaitsev), **stb_truetype** and **stb_vorbis** (Sean Barrett), **SMAA** (Jorge Jimenez and others), **Arm ASTC encoder**, **Lucide icons**, **Kenney Mobile Controls**, **Overpass** (Red Hat, Delve Withrington), **Newtown** (Roger White), **dwyl/english-words**.

## The Steam Frame and its software

- **Valve**: the Steam Frame, SteamOS, Steam and **SteamVR** with its OpenXR runtime and the Frame's controller profile (`XR_VALVE_frame_controller_interaction`). OpenFrame is unofficial and unaffiliated.
- **Mesa and its Turnip Vulkan driver** for the Adreno GPU, by the Mesa contributors.

## Packaging tools

The packaging layer in `steam-frame/` follows the shape of my other Steam Frame ports and reuses their scripts, which are my own:

- **GalaxyFrame** (simplysanatori): the installer, Steam shortcut tool (`steam-cef.py`), launch wrapper, release and README structure.
- **PrimedFrame** (simplysanatori): the session folders, `session.env`, SteamVR per-app settings read-back, log collection and the headset-in-use convention.
- **Moonlight XR Native** (simplysanatori), GPLv3: `vrtiming.py` and `privacy-check.sh` come from its tools.

## Artwork

The Steam library artwork of my own OpenFrame entry (capsules, hero, logo, icon) comes from [SteamGridDB](https://www.steamgriddb.com), by its community artists: UncookedMoa (portrait capsule), ender-blade (wide capsule), Gector(lint)Nathan (hero), Bcoder (logo) and Onyx8011 (icon). The asset ids are in `steam-frame/art/SOURCES.md`. The images are not part of this repository or of its releases. The Halo artwork in them belongs to Microsoft, 343 Industries and Bungie; it is used only for my own library entry.
