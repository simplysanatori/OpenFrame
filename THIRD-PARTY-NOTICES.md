# Notices and third-party software

## OpenFrame

Copyright (C) 2026 simplysanatori and OpenFrame contributors.

OpenFrame is free software: you can redistribute it and change it under the terms of the GNU Affero
General Public License as published by the Free Software Foundation, version 3 of the License or (at your
option) any later version (`AGPL-3.0-or-later`). It comes with no warranty. The full text is in
[LICENSE](LICENSE). OpenFrame is a modified version of OpenH1; every file it did not change keeps
OpenH1's and its authors' notices, and the changes OpenFrame makes are listed in the git history of the
branches that carry them. If you run a modified version where others interact with it over a network,
section 13 of the AGPL gives them the right to its source.

The packaging layer in `steam-frame/` is OpenFrame's own work. `steam-frame/tools/vrtiming.py` and
`steam-frame/tools/privacy-check.sh` descend from the tools of Moonlight XR Native (GPLv3), by the same
author. Nothing in this repository is Bungie's or Microsoft's: the game data (the maps) is supplied by the
player.

Halo and Halo: Combat Evolved are trademarks of Microsoft. OpenFrame is not affiliated with or endorsed
by Microsoft, Bungie, 343 Industries, Valve, OpenH1 or OpenCE.

## What OpenFrame is built on

| Work | Licence |
|---|---|
| OpenH1 (https://github.com/OpenH1/OpenH1), OpenCE (https://github.com/OpenCommunityEdition/OpenCE) | AGPL-3.0-or-later from the licence change; CC0 1.0 before it (`LICENSES/CC0-1.0.txt`) |
| The decompilation: punpckhdq/halo, bnunu/halo-1 | AGPL-3.0 from 17 October 2026 (punpckhdq/halo); CC0 1.0 before that date, and bnunu/halo-1 |
| astromaddie/HaloCE-VR (the OpenXR foundation of the Halo VR mod) | CC0 1.0 |
| moistman42069/HaloCE-DecompRemix-VR-PC-Android (features being ported) | AGPL-3.0-or-later (CC0 1.0 before its release 1.0.37) |

Features ported from a project under another licence keep that licence and its notices. [CREDITS.md](CREDITS.md)
says whose work is whose.

## Libraries

OpenH1's `port/third_party/` holds the following, unchanged unless its README says otherwise; each has
its licence file beside it and in a release's `licenses/` folder.

| Library | Authors | Licence | Where |
|---|---|---|---|
| SDL3 (3.4.16, built from source into `libSDL3.so.0`) | Sam Lantinga and the SDL contributors | zlib | `licenses/SDL3-LICENSE.txt` in a release |
| OpenXR SDK headers (release 1.1.63; the game loads the runtime itself) | The Khronos Group | Apache-2.0 or MIT | downloaded at configure time |
| Vulkan headers | The Khronos Group | Apache-2.0 or MIT | downloaded at configure time |
| LLVM / Clang / lld (22.1.8, the build's pinned toolchain; not shipped) | the LLVM Project | Apache-2.0 with LLVM exceptions | `~/.cache/openh1` |
| musl libc 1.2.5 and its maths functions | Rich Felker and contributors | MIT | `port/third_party/musl-math/COPYRIGHT` |
| Mbed TLS 3.6 | Arm and the Mbed TLS contributors | Apache-2.0 or GPL-2.0-or-later | `port/third_party/mbedtls/LICENSE` |
| Monocypher 4.0.3 | Loup Vaillant, Michael Savage, Fabio Scotoni | CC0-1.0 or BSD-2-Clause | `port/third_party/monocypher/LICENCE.md` |
| KCP | Lin Wei (skywind3000) | MIT | `port/third_party/kcp/LICENSE` |
| miniupnpc 2.3.3 | Thomas Bernard | BSD-3-Clause | `port/third_party/miniupnpc/LICENSE` |
| Opus | Xiph.Org Foundation and contributors | BSD-3-Clause | `port/third_party/opus/COPYING` |
| Expat 2.8.5 | James Clark, Sebastian Pipping and contributors | MIT | `port/third_party/expat/COPYING` |
| zlib 1.3.2 | Jean-loup Gailly and Mark Adler | zlib | `port/third_party/zlib/LICENSE` |
| tomlc17 | CK Tan | MIT | `port/third_party/tomlc17/LICENSE` |
| jsmn | Serge Zaitsev | MIT | `port/third_party/jsmn/LICENSE` |
| stb_truetype, stb_vorbis | Sean Barrett and contributors | public domain or MIT | `port/third_party/stb/`, `stb_vorbis/` |
| SMAA | Jorge Jimenez, Jose I. Echevarria, Belen Masia, Fernando Navarro, Diego Gutierrez | MIT | `port/third_party/smaa/LICENSE` |
| Arm ASTC encoder 5.7.0 | Arm Limited | Apache-2.0 | `port/third_party/astc-encoder/LICENSE.txt` |
| dwyl/english-words | dwyl and contributors | Unlicense | `port/third_party/english-words/LICENSE.md` (a streamer-mode tool only) |

Assets in OpenH1's `port/assets/`:

| Asset | Licence |
|---|---|
| Overpass (Red Hat, Delve Withrington) | SIL OFL 1.1 (`port/assets/fonts/Overpass-OFL.txt`) |
| OpenCE title font, from Newtown (Roger White, public domain) | SIL OFL 1.1 (`port/assets/fonts/OpenCE-OFL.txt`) |
| Lucide icons (Lucide Icons and Contributors) | ISC (`port/assets/icons/lucide/LICENSE`) |
| Kenney Mobile Controls (www.kenney.nl) | CC0 |
| The high-res HUD's weapon and skull pictures (Aerocatia and Jessie, via Halo PC Restored) | not OpenH1's licence; used with permission on condition of credit (`port/assets/hud/NOTICE.md`) |
| The rest of the art | see `AUTHORS.md` of OpenH1 (kept as `AUTHORS.md`) |

Jolt Physics is not part of OpenH1 at `build-734`. If the ragdoll feature is ported, it will be listed
here (MIT, Jorrit Rouwe and contributors).

## On the Steam Frame

The Steam Frame, SteamOS, Steam and SteamVR belong to Valve; the OpenXR runtime and `vrcmd` are SteamVR's.
Mesa's Turnip Vulkan driver is under the MIT licence. None of them is shipped with OpenFrame.
