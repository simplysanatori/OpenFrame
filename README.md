# OpenFrame

Halo: Combat Evolved in VR, running natively on the Valve Steam Frame.

OpenFrame is the Steam Frame fork of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo: Combat Evolved. The game is the Xbox build's own code, compiled for 64-bit Arm Linux and drawn with Vulkan, with OpenXR VR through the Frame's SteamVR. There is no emulator, no Lepton, no APK and no PC. OpenFrame adds the Steam Frame packaging layer (install, launch wrapper, Steam shortcut, logs, release) and brings VR gameplay features from the Halo VR mods into OpenH1's VR mode.

You need your own Xbox disc of Halo: Combat Evolved. No game data is in this repository or its releases.

What it does:

- Plays Halo: Combat Evolved in the headset, in stereo, with the head tracked in six degrees of freedom, at the Frame's refresh rate (90 Hz by default; 72 and 120 Hz are selectable).
- Uses the Steam Frame controllers through SteamVR's own controller profile: the right controller holds and aims the weapon, the hands appear, the left hand reloads, throws grenades and holds long guns by the foregrip.
- Keeps OpenH1's renderer, netcode and menus: the game plays as it shipped (30 ticks a second), with frames drawn between ticks at the display's rate, and it can play online with OpenH1 and OpenCE players.

## Status

**OpenFrame has not been tested in a headset yet.** Today this repository holds the packaging layer, which has been exercised only against scratch folders and a dry-run Steam configuration, and the documentation. OpenH1's VR mode itself was made on a Steam Frame with SteamVR, by its authors; OpenFrame has not confirmed it on the Frame it targets.

| Field | Value |
|---|---|
| Tested hardware | not yet |
| SteamOS | not yet |
| SteamVR | not yet |
| Game version | OpenH1 `build-734` (the base of branch `steam-frame`) |
| Disc / maps | not yet |
| Refresh rates seen working | not yet |

Known:

- Nothing is confirmed in a headset, so nothing is promised here. The notes in [dev/DEV.md](steam-frame/dev/DEV.md) list what has and has not been run.
- The moistman VR features listed below are not in the game yet.
- OpenH1's self-updater is turned off in OpenFrame (see [Updates](#updates)). A code-level disable is still to do.
- There is no Steam library art yet; the library entry has Steam's default look.

## Requirements

- A Steam Frame with SteamOS and SteamVR, and about 4 GB free: the maps are 1.8 GB and the game builds a cache of about 800 MB.
- The maps of your own Xbox disc of Halo: Combat Evolved: the 24 `.map` files of the disc's `maps` folder (about 1.8 GB), extracted from a disc image you made from a disc you own. Halo PC's maps and the MCC's do not work. Every version of the Xbox game works; the game changes the PAL maps to play as the North American ones do. [openh1.com/maps](https://openh1.com/maps) explains how to make and read a disc image.
- A terminal on the Frame: SSH, or Konsole from the desktop.
- Steam running on the Frame while you install, so the installer can add the library entry. (It is on by default on SteamOS. `OPENFRAME_SKIP_STEAM=1` installs without it.)

## Install

1. Get the repository:

   ```sh
   git clone -b steam-frame https://github.com/simplysanatori/OpenFrame.git
   cd OpenFrame
   ```

2. Get the game, either way:

   - Download `openframe-<version>-linux-arm64.tar.gz` and `SHA256SUMS` from the [Releases](https://github.com/simplysanatori/OpenFrame/releases) page (there is no release yet), check it, and install it:

     ```sh
     sha256sum -c --ignore-missing SHA256SUMS
     steam-frame/install.sh --maps /path/to/your/maps ~/Downloads/openframe-<version>-linux-arm64.tar.gz
     ```

   - Or build it on the Frame (the first build downloads about 1.5 GB of toolchain and takes a long time; see [dev/DEV.md](steam-frame/dev/DEV.md)):

     ```sh
     steam-frame/build.sh
     steam-frame/install.sh --dev --maps /path/to/your/maps
     ```

   `--maps` takes the folder with your 24 `.map` files, or the folder that contains `maps/`. They are copied, not moved, into `~/.local/share/OpenFrame/maps`.

   The installer copies the program to `~/Games/OpenFrame/app` (`app-dev` with `--dev`), writes `~/.local/share/OpenFrame/config.toml`, and adds **OpenFrame** to your Steam library as a non-Steam shortcut (with its OpenVR, Steam overlay and desktop configuration flags on), or updates the entry if it is there. It talks to Steam through its local debugging port, which SteamOS has on by default. It needs no sudo.

3. Start SteamVR, put the headset on and launch **OpenFrame** from your Steam library.

To update, install a newer tarball the same way. Your maps, saves and settings stay where they are. To remove it: `~/Games/OpenFrame/app/tools/uninstall.sh` (add `--purge` to delete the data folder too).

The installer changes nothing outside `~/Games/OpenFrame`, `~/.local/share/OpenFrame`, the Steam shortcut and its art, and SteamVR's settings for that shortcut (which the launch wrapper sets at each launch).

### Updates

OpenH1 updates itself from its own GitHub releases. That would replace OpenFrame with OpenH1, so the installer writes `update.auto = false` into `config.toml` and the launch wrapper exports `HALO_UPDATE_AUTO=false` on every start. As installed here the game's update check does not run at all, because it needs OpenH1's own Steam Frame launcher and package files, which OpenFrame does not use; the setting and the variable are a second lock. Update OpenFrame by installing a newer OpenFrame release.

## Controls

The Steam Frame's controllers are the halves of an Xbox controller. This is OpenH1's default VR mapping; the full list, with every `vr.*` setting that changes it, is in the "VR" section of [port/linux/README.md](port/linux/README.md#vr).

| Steam Frame | Game |
|---|---|
| Left stick | Move (click: crouch) |
| Right stick, left and right | Turn, in steps (`vr.turn = "snap"`) or smoothly |
| Right stick click | Zoom: a magnified window about the reticle (smart-link), or a raised scope's magnification |
| Head | Look. Crouch in the room and the game crouches |
| Right controller | Aim: it holds the weapon and the reticle shows where it points |
| Left controller | The left hand; the left grip closes it |
| Right trigger | Fire |
| Left grip held, the left hand drawn back and swung, the grip let go | Throw a grenade where the hand threw it (`vr.grenade_throw`; the left trigger throws nothing on foot by default) |
| A, B, X, Y | Jump, melee, action and reload, switch weapon |
| Left grip on a long gun's foregrip | Aim it with both hands |
| A swing of the right controller | Melee |
| Right hand over the shoulder or at the hip, gripping | Draw the weapon carried there (holsters) |
| The sniper rifle's or pistol's scope raised to an eye | Look through it, magnified |
| Left hand at the left hip, then pushed into the weapon | Reload by hand |
| Left bumper, right bumper | Flashlight, switch grenade |
| D-pad, View, Menu | Menu d-pad, Back (scoreboard), Start (pause) |
| Both grips, held | Recentre |
| Both grips on a Ghost's handles or a Warthog's wheel, turned | Steer it |

Left-handed play (`vr.dominant_hand = "left"`) mirrors the weapon, the hands and the holsters.

## Settings

`~/.local/share/OpenFrame/config.toml` holds the settings. The game writes its own settings into it with a comment on each; a setting commented out follows the default. Every setting also has a `HALO_*` environment variable, which wins over the file. Put `NAME=value` lines in `~/.local/share/OpenFrame/session.env` for one launch, or in front of the command in the shortcut's Steam launch options (an install resets those). The wrapper's own settings:

| Variable | Default | What |
|---|---|---|
| `OPENFRAME_REFRESH_HZ` | 90 | Refresh rate asked of SteamVR (per-app `preferredRefreshRate`) and of the game (`HALO_VR_REFRESH_RATE`): 72, 90 or 120 |
| `OPENFRAME_SYSMEM` | on | `TU_DEBUG=sysmem`: the GPU renders straight to memory instead of in tiles. `0` turns it off |
| `OPENFRAME_PACING` | unset | Reserved for the game's pacing modes; passed through, ignored today |
| `OPENFRAME_MOTION_SMOOTHING` | 2 (off) | SteamVR `motionSmoothingOverride` for the app |
| `OPENFRAME_VRTIMING` | on | `0`: no SteamVR compositor timing log |

At each launch the wrapper also sets SteamVR's per-app `framesToThrottle=0` and `additionalFramesToPredict=0`.

## Features

### OpenH1's VR mode (in the game, from `build-734`)

- OpenXR through SteamVR with Vulkan, single-pass multiview stereo, eye-tracked foveated rendering, and a late latch of the head and controller poses.
- Hands and arms that follow the controllers, with the weapon in the right hand; left-handed mode.
- Two-handed aiming of long guns, smart-link and at-the-eye scopes, holsters at the shoulder and hip, reloading by hand, grenades thrown by the left hand, melee by swinging.
- Picking up weapons by hand, pressing controls by hand, flipping vehicles, climbing into vehicles, steering a Ghost by its handlebars and a Warthog by its wheel, driving a Scorpion from its cockpit, flying a Banshee.
- The HUD on the helmet's visor, with depth, wrap, glow and the shields' effects on the glass; snap or smooth turning; physical crouch; a comfort vignette in vehicles; head-in-wall handling; death view; 3D sound from your head (HRTF).
- Everything OpenH1 has outside VR: the Vulkan renderer with SMAA, anisotropic filtering and per-pixel lights, up to 128 players online, mods and Custom Edition maps.

### Planned from the Halo VR mods (in progress)

These come from [moistman42069's HaloCE-DecompRemix-VR-PC-Android](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android), a Quest-focused fork of OpenCE. They are being ported into OpenFrame's OpenH1 base with credit. **None is in a build yet; this list is the plan, not the product.**

| Feature | State |
|---|---|
| Bullet time | in progress, not merged |
| Sprint (toggle) | in progress, not merged |
| General comfort vignette (OpenH1's covers vehicles only) | in progress, not merged |
| Third-person camera in VR | planned |
| Sliding and parkour | planned |
| Fists, and deflecting projectiles | planned |
| Grabbing the world with a hand | planned |
| Wrist HUD | planned |
| Ragdolls (needs a physics library) | planned |
| Co-op pose sync | planned |

## Files

| What | Where |
|---|---|
| Program | `~/Games/OpenFrame/app` (`halo`, `libSDL3.so.0`, `openframe.sh`, `tools/`) |
| Settings, maps, saves, internet play's `brokers.txt` | `~/.local/share/OpenFrame` (`config.toml`, `maps/`, `saves/`, `brokers.txt`) |
| Logs | `~/.local/share/OpenFrame/sessions/latest/` (`launch.log`, `stdout.log`, `debug.txt`, `vrtiming.log`, `perfcriteria.txt`) |
| One-launch test settings | `~/.local/share/OpenFrame/session.env` (`NAME=value` lines; `session.env.next` for the launch after) |

## Internet play

Internet play is OpenH1's and is on by default. Invites and the server browser find each other through public MQTT brokers listed in `brokers.txt` (`network.brokers_file`) and OpenH1's master servers, so using it contacts them. `network.online = false` in `config.toml` turns internet play off and leaves system link on the local network, which needs no brokers.

## Troubleshooting

- **`install.sh` says Steam's debugging port is not reachable.** Start Steam and run it again, or set `OPENFRAME_SKIP_STEAM=1`.
- **The game asks for the disc image at start.** It did not find `maps/` in `~/.local/share/OpenFrame`. Run `install.sh --maps` with your maps folder.
- **It plays flat in a window.** SteamVR was not running, or the headset was not connected, when the game started. Start SteamVR first. `HALO_VR=0` forces flat play.
- **Check what happened.** `~/.local/share/OpenFrame/sessions/latest/launch.log` shows the refresh rate asked for and whether SteamVR took the per-app settings; `stdout.log` is the game's output, with a crash's backtrace.

## Reporting a problem

Open an [issue](https://github.com/simplysanatori/OpenFrame/issues) with what you did, what you expected and what happened. Run `~/Games/OpenFrame/app/tools/collect-logs.sh`: it writes one archive of the newest session's logs with IP and MAC addresses, host names, Wi-Fi names, serial numbers, Steam ids, e-mail addresses, tokens and your user name masked. The masking is best effort: read the files before you attach them. A problem that is in OpenH1 itself, not the Frame packaging or a ported feature, belongs on [OpenH1's tracker](https://github.com/OpenH1/OpenH1/issues).

## Build from source

[steam-frame/dev/DEV.md](steam-frame/dev/DEV.md) has the build, the install for testing, the wrapper, the release and the privacy check. `steam-frame/tools/privacy-check.sh` checks our files, the history and a release tarball for personal data before publishing.

## Relationship to upstream

OpenFrame is a fork of [OpenH1](https://github.com/OpenH1/OpenH1), which is a fork of [OpenCE](https://github.com/OpenCommunityEdition/OpenCE) (Open Community Edition), which starts from the decompilation of the Xbox build 2342 in [bnunu/halo-1](https://github.com/bnunu/halo-1), a fork of [punpckhdq/halo](https://github.com/punpckhdq/halo). The branch `steam-frame` is pinned to OpenH1's release tag `build-734`; OpenH1's own README is [README.upstream.md](README.upstream.md). The packaging layer lives in `steam-frame/` and touches nothing in OpenH1's `source/`, `port/` or `tools/`. OpenFrame keeps OpenH1's netcode, so its players can play with OpenH1's and OpenCE's. Bug reports for OpenH1's own behaviour should go to OpenH1; send fixes that are not Frame-specific upstream.

[CREDITS.md](CREDITS.md) lists everyone whose work this stands on. If you want to support someone, support them first: star and follow OpenH1, OpenCE and the decompilation, and report what you find there.

## Licence

OpenFrame is free software under the GNU Affero General Public License, version 3 or (at your option) any later version ([LICENSE](LICENSE)), as OpenH1 is. Third-party parts keep their own licences: see [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md). The packaging scripts in `steam-frame/` are the author's own, under the same licence; `tools/vrtiming.py` and `tools/privacy-check.sh` descend from Moonlight XR Native's tools, which are GPLv3. No Halo game data is included.

Halo, Halo: Combat Evolved and their logos are trademarks of Microsoft. This is an unofficial fan project. It is not affiliated with or endorsed by Microsoft, Bungie, 343 Industries, Valve, OpenH1 or OpenCE.

## How this was made

I wanted Halo in VR on my Frame without an emulator or a PC, and OpenH1 already had a native Arm64 build with an OpenXR VR mode, made on a Frame. So I forked it: the packaging layer here was written to be the same shape as my other Frame ports, and the VR gameplay features of the Halo VR mods are being ported in with attribution, one at a time.

This is vibe coded, with manual review. I wrote it with Claude Code, using Anthropic's Fable 5.1, Opus 5.5 and Sonnet 5.5 models running on the Frame itself. I chose what to build and review what is written; Claude reads the code and the logs and changes the files. The packaging layer and these documents were written by Claude, and the ported features are written by Claude from the mods' sources.  **None of it has been tested in a headset yet**, and none of the upstream projects has reviewed this fork. If that is not for you, no worries. Read the code before trusting it.
