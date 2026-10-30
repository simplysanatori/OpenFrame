# OpenFrame

Halo: Combat Evolved in VR, running natively on the Valve Steam Frame.

OpenFrame is my Steam Frame version of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo: Combat Evolved. The game is the Xbox build's own code, compiled for Arm64 Linux, drawn with Vulkan, and shown in VR through the Frame's own SteamVR. You do not need an emulator, Lepton, an APK or a PC. On top of OpenH1 I added the Frame setup, faster loads, settings tuned for 90 Hz, controls made for the Frame controllers, and VR features from other people's Halo VR work.

You need the maps from your own Xbox disc of Halo: Combat Evolved. No game data is in this repository or its releases.

## Status

Version 1.0.0, the first release. I play the campaign on my Steam Frame at 90 Hz, both eyes at 0.9 of SteamVR's recommended resolution with eye-tracked foveation, and the first level's firefights hold the display's 90 Hz most of the time. The menus and the HUD open in front of you and fit the Frame's view.

**Compared with OpenH1's own Frame build.** OpenH1 ignored SteamVR's recentre, so the menu could sit off to one side and the view could start outside your body. The HUD's corners sat partly outside the Frame's view. The world froze for one to two seconds each time a level moved on to its next section, and the first level took about 17 seconds to load. Here recentring works and the game recentres itself at each level start, the HUD fits, a section change is a short hitch, and the first level loads in about 5 seconds. In the heaviest firefight of the first level, OpenH1's resolution and foveation averaged about 77 fps; mine average 86 to 89. That is less work for the GPU every frame.

Known issues:

- Big fights with many explosions fall into the 60s for as long as the fight lasts.
- The first few seconds after a level loads run slower.
- The campaign has one save slot. Starting a new game overwrites it (see [Files](#files)).
- In bullet time the sound plays at normal speed.

## What this adds over OpenH1 build-769

OpenH1 already plays Halo in VR on the Frame. OpenFrame began at OpenH1's `build-734` and is now on `build-769`. I changed the setup around it, tuned it, and brought in features:

- An installer, a Steam library entry and a launch wrapper that sets SteamVR's per-app settings at every launch: the refresh rate, no throttling, no motion smoothing.
- Its own data folder, apart from OpenH1's, and OpenH1's self-updater switched off so it never swaps this build for OpenH1.
- Defaults for the Frame: 0.9 resolution, a 0.4 foveation area, the GPU rendering straight to memory, and the HUD's corners where the Frame's view can see them.
- Faster section changes and loads: the shader preparation for each section no longer walks every light, and Xbox maps get a larger texture cache.
- Recentring that works, an automatic recentre at each level start, menus in front of you, and a cryo tube tutorial that counts head looks.
- Right-stick gestures for sprint, third person and bullet time, made for the Frame controllers.
- VR Setup split into pages, with a HUD layout page of sliders and a refresh rate choice of 72, 90 or 120 Hz.
- From moistman42069's Halo VR mod, rewritten for OpenH1: sprint, bullet time, third person on foot, slide, wall jump and wall run, fists with stagger and parry, weapon pull, prop throw, hand swap, the wrist HUD, and the comfort vignette on foot.

### What came with OpenH1 build-769

OpenH1's own fixes between `build-734` and `build-769` are in this release:

- Lights past the 128-per-window cap no longer drop lit objects.
- A portal corner that sits on a clip line no longer flashes the level.
- Respawn goes straight to first person.
- The d-pad's focus is kept when you back out of a settings screen.
- Multiplayer, from OpenH1: host controls, OpenH1 games listed first in the server browser, the Kills Score, Multi-Hill and Force Team Spawns rules, and network errors in plain words.

## Requirements

- A Steam Frame with SteamOS and SteamVR, and about 4 GB free: the maps are 1.8 GB and the game builds a cache of about 800 MB.
- The 24 `.map` files from the `maps` folder of your own Xbox disc of Halo: Combat Evolved (about 1.8 GB). Halo PC's maps and the MCC's do not work. Every version of the Xbox game works; the game plays PAL maps as the North American ones. [openh1.com/maps](https://openh1.com/maps) explains how to make and read a disc image.
- A terminal on the Frame for the one-time setup: SSH, or Konsole from desktop mode.
- Steam running on the Frame while you install, so the installer can add the library entry.

## Install

1. Get the repository:

   ```sh
   git clone -b steam-frame https://github.com/simplysanatori/OpenFrame.git
   cd OpenFrame
   ```

2. Download `openframe-1.0.0-linux-arm64.tar.gz` and `SHA256SUMS` from the [Releases](https://github.com/simplysanatori/OpenFrame/releases) page. Check the download and install it with your maps:

   ```sh
   sha256sum -c --ignore-missing SHA256SUMS
   steam-frame/install.sh --maps /path/to/your/maps ~/Downloads/openframe-1.0.0-linux-arm64.tar.gz
   ```

   `--maps` takes the folder with your 24 `.map` files, or the folder that holds `maps/`. They are copied, not moved, into `~/.local/share/OpenFrame/maps`.

   The installer copies the program to `~/Games/OpenFrame/app`, writes `~/.local/share/OpenFrame/config.toml`, and adds **OpenFrame** to your Steam library as a non-Steam shortcut with its OpenVR flag on, or updates the entry if it is there. It talks to Steam through its local debugging port, which SteamOS has on by default. It needs no sudo. `OPENFRAME_SKIP_STEAM=1` installs without touching Steam.

3. Start SteamVR, put the headset on and launch **OpenFrame** from your Steam library.

To update, install a newer tarball the same way. Your maps, saves and settings stay where they are. OpenH1's own updater is off: it would replace OpenFrame with OpenH1.

The installer changes nothing outside `~/Games/OpenFrame`, `~/.local/share/OpenFrame`, the Steam shortcut and SteamVR's settings for that shortcut. To remove OpenFrame, run `~/Games/OpenFrame/app/tools/uninstall.sh`. It keeps your data folder unless you pass `--purge`.

## First run

On the first start the game makes your profile and its cache of map data, about 800 MB, in `~/.local/share/OpenFrame/saves`.

Sit or stand as you mean to play when the loading picture fades: the game recentres on that first frame. Hold both grips for a moment at any time to recentre again.

## Controls

The Frame's controllers are the two halves of an Xbox controller. The right one holds and aims the weapon. If you play left-handed, the two swap: see [Left-handed play](#left-handed-play).

| Steam Frame | In the game |
|---|---|
| Right controller | Aim. It holds the weapon and the reticle shows where it points |
| Head | Look. Crouch in the room and the game crouches |
| Right trigger | Fire |
| Left trigger | Second weapon in a Banshee. On foot you throw grenades by hand |
| Left stick | Move |
| Left stick click | Crouch, at any speed. While sprinting, slide (PARKOUR on) |
| Right stick, left and right | Turn, in steps or smoothly |
| Right stick, flick up | Sprint on or off (SPRINT set to TOGGLE) |
| Right stick, flick down | Bullet time on or off (BULLET TIME on) |
| Right stick, held down 0.8 seconds | Third person on or off. Needs no setting |
| Right stick, pressed in | Zoom, always, in bullet time as well |
| **A** | Jump |
| **B** | Melee. A hard swing of the right controller melees too |
| **X** | Action and reload. Hold to pick up a weapon |
| **Y** | Switch weapon |
| Left bumper | Flashlight |
| Right bumper | Switch grenade type |
| D-pad | Menus |
| **View** | Back, and the multiplayer scoreboard |
| **Menu** | Pause |
| Both grips, held | Recentre |
| Right grip | Draw a weapon from a holster, over the shoulder or at the hip |
| Left grip on a long gun's foregrip | Aim it with both hands |
| Left grip at the left hip | Take a magazine. The old one drops out of the weapon by itself; push the new one in to load |
| Left grip held, swung and let go | Throw a grenade where your hand threw it |
| A raised scope at your eye | Look through it, magnified |
| Either hand pushed into a control | Press it |
| Both grips on a Ghost's handles or a Warthog's wheel | Steer it |

A flick counts when the right stick goes within about 34 degrees of straight up or down; while it is inside that cone, turning pauses, so a flick never turns you, and a diagonal push only turns you. A flick down switches bullet time as you let go. There is no magazine eject on the stick: reload with **X**, or take a magazine from your left hip. Sprint, bullet time and the third person show a label on the HUD when they switch.

Sprint, bullet time, parkour, fists, weapon pull, prop throw and hand swap come from moistman42069's mod and are each off until you turn them on in VR Setup. The third person needs no setting. [docs/CONTROLS.md](docs/CONTROLS.md) explains how each one is used, and [docs/openframe-features.md](docs/openframe-features.md) has every setting.

## Settings

Most settings are in the headset: **Settings > Controls > VR SETUP**, the last row. It appears while the game plays in VR, and also at the foot of the player profile menu. Its pages are **DISPLAY...**, **HUD...**, **PLAYER...**, **MOVEMENT AND COMFORT...** and **HANDS AND WEAPONS...**. The HUD page opens **HUD LAYOUT...**, with sliders to bring the corners in and to move or resize each one.

Everything is also in `~/.local/share/OpenFrame/config.toml`, with a comment on each line. A line commented out follows the default. Every setting has a `HALO_*` environment variable too, which wins over the file. The ones that matter most on the Frame:

| Key | Default | What it does |
|---|---|---|
| `vr.refresh_rate` | 90 | 72, 90 or 120 Hz. VR Setup > DISPLAY... > REFRESH RATE, from the next start |
| `vr.resolution_scale` | 0.9 | Eye size against SteamVR's recommended 1728 per eye. 1.0 is sharper and costs GPU time |
| `vr.foveation` | `"eye"` | Eye-tracked foveation. `"fixed"` keeps the sharp area in the middle |
| `vr.foveation_inset` | 0.4 | Size of the full-detail area. 0.5 is OpenH1's default and costs more |
| `vr.hud_inset` and `vr.hud_weapon_*`, `vr.hud_status_*`, `vr.hud_tracker_*` | inset 0.05, weapon 0.2 / -0.1, status -0.08 / -0.1, tracker 0.1 / 0.0 | The HUD's corners, moved in for the Frame. HUD LAYOUT... sets them |
| `vr.turn`, `vr.snap_turn_angle` | snap, 30 | Snap or smooth turning |
| `vr.height` | `"seated"` | Your eyes are wherever you recentre. `"standing"` uses your height and the floor |
| `vr.auto_recentre` | true | Recentre at each level start, and open menus in front of you |

For one launch only, put `NAME=value` lines in `~/.local/share/OpenFrame/session.env`. It is read once and renamed `session.env.used`. The wrapper's own settings:

| Variable | Default | What it does |
|---|---|---|
| `OPENFRAME_REFRESH_HZ` | unset | 72, 90 or 120 for this launch, instead of `vr.refresh_rate` |
| `OPENFRAME_SYSMEM` | on | The GPU renders straight to memory (`TU_DEBUG=sysmem`). `0` goes back to tiled rendering, about 10 fps slower |
| `OPENFRAME_MOTION_SMOOTHING` | 2 (off) | SteamVR's motion smoothing for the game |
| `OPENFRAME_VRTIMING` | on | `0` turns off the SteamVR timing log |

### Left-handed play

VR Setup > PLAYER... > **DOMINANT HAND** = LEFT. The left controller then fires, aims and draws from the holsters on your left, and the right hand holds the foregrip, takes magazines from your right hip and throws grenades. **SWAP STICKS** follows the hand by default, so the right stick moves and the left one turns and carries the stick gestures. The third-person view sits over your left shoulder and the wrist HUD moves to your right wrist.

### Comfort

Seated play, physical crouch, snap or smooth turning, the vignette, world scale, and the HUD's and menus' size and distance are all in VR Setup, on the PLAYER..., HUD... and MOVEMENT AND COMFORT... pages.

### Internet play

Internet play is OpenH1's and is on by default. Invites and the server browser use the public MQTT brokers in `brokers.txt` and OpenH1's master servers. `network.online = false` in `config.toml` turns it off and keeps system link on your local network. OpenFrame plays online with OpenH1 and OpenCE players. The added features stay offline.

## Performance

Measured on my Frame from the session logs, at the defaults, in the first level:

| | |
|---|---|
| Display | 90 Hz, 1554 x 1554 per eye (0.9 of SteamVR's 1728) |
| Foveation | Eye-tracked, about a third of a full eye's pixels drawn |
| Firefights | 88.5 fps average over two rooms of the first level. The heaviest fights, with many explosions, fall into the 60s while they last |
| The heaviest firefight | 86 to 89 fps average (OpenH1's resolution and foveation: about 77) |
| GPU per frame | 7.2 ms median of the 11.1 ms a frame at 90 Hz |
| Dropped frames | a handful over a session |
| Frames SteamVR filled in | about one in ten over a session (OpenH1's resolution and foveation: about one in three) |
| A section change | about 0.4 s of preparation, a short hitch (OpenH1: a freeze of one to two seconds) |
| Loading a level | about 5 seconds (OpenH1: about 17 for the first level) |

What the build does, so you know what the numbers mean:

- Both eyes are drawn in one pass, foveated around where you look.
- The game runs at its own 30 ticks a second, and frames are drawn between ticks at the display's rate.
- The launch wrapper sets the GPU to render straight to memory. In the same rooms, tiled rendering was about 10 fps slower.
- `launch.log` in each session folder shows the refresh rate asked for and whether SteamVR took the per-app settings. `vrtiming.log` has SteamVR's timing per second.
- Keep builds and other heavy work off the Frame while you play.

## Files

| What | Where |
|---|---|
| Program | `~/Games/OpenFrame/app` (`halo`, `libSDL3.so.0`, `openframe.sh`, `tools/`) |
| Settings, maps, saves | `~/.local/share/OpenFrame` (`config.toml`, `maps/`, `saves/`, `brokers.txt`) |
| Campaign save | `saves/u/UDATA/<profile id>/savegame.bin`. One slot per profile: **starting a new game, or any level, overwrites it**. Copy `saves/` aside first if you want to come back |
| Session logs | `~/.local/share/OpenFrame/sessions/<date-time>/` (`launch.log`, `stdout.log`, `debug.txt`, `vrtiming.log`); `sessions/latest` points at the newest |
| One launch's settings | `~/.local/share/OpenFrame/session.env` |

The Vulkan pipeline cache is shared with OpenH1, in `~/.cache/openh1`. Nothing else is written to OpenH1's folders.

## Troubleshooting

- **`install.sh` says Steam's debugging port is not reachable.** Start Steam and run it again, or set `OPENFRAME_SKIP_STEAM=1`.
- **The game asks for the disc image at start.** It did not find `maps/` in `~/.local/share/OpenFrame`. Run `install.sh --maps` with your maps folder.
- **It plays flat in a window.** SteamVR was not running when the game started. Start SteamVR first.
- **The menu is off to the side, or the view is not where the player is.** Hold both grips for a moment to recentre. SteamVR's own recentre works too.
- **The HUD's corners are cut off,** for example with a glasses spacer. VR Setup > HUD... > HUD LAYOUT... > **HUD INSET** brings them all in. Each corner also has its own sliders.
- **Something went wrong.** `~/.local/share/OpenFrame/sessions/latest/stdout.log` is the game's output, with a crash's backtrace.

## Reporting a problem

Open an [issue](https://github.com/simplysanatori/OpenFrame/issues) with what you did, what you expected and what happened, and the output of:

```sh
~/Games/OpenFrame/app/tools/collect-logs.sh
```

It gathers the newest session's logs into one archive and masks IP and MAC addresses, host names, Wi-Fi names, serial numbers, Steam ids, email addresses, tokens and user names in paths. The masking is a best effort. Read the files before you attach them. A problem in OpenH1 itself belongs on [OpenH1's tracker](https://github.com/OpenH1/OpenH1/issues).

## Build from source

[steam-frame/dev/DEV.md](steam-frame/dev/DEV.md) has the build, the install of a build for testing, the wrapper and the release. The first build downloads about 1.5 GB of toolchain. `steam-frame/tools/privacy-check.sh` checks the tree, the history and a release tarball for personal data before publishing.

## Support

OpenFrame exists because of [OpenH1](https://github.com/OpenH1/OpenH1), [OpenCE](https://github.com/OpenCommunityEdition/OpenCE) and the Xbox decompilation they start from, and because of moistman42069's [Halo VR mod](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android). If you want to support someone, support them first: star and follow them, report what you find there, and send fixes that are not Frame-specific upstream.

If you would like more Steam Frame ports, you can also [buy me a coffee](https://ko-fi.com/sana748653). Thank you.

## Credits

- Built by [simplysanatori](https://github.com/simplysanatori).
- [OpenH1](https://github.com/OpenH1/OpenH1) and its team: the game, the Vulkan renderer, the netcode and the VR mode this is built on, made on a Steam Frame.
- [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) by moistman42069: sprint, bullet time, third person, slide, wall jump and wall run, fists, weapon pull, prop throw, hand swap, the wrist HUD and the vignette on foot are ported from it. Its own credits carry over: LivingFray's HaloCEVR, jcompto95-tech's OpenCE fork, Cesarus85's Mjolnir VR and others.
- [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR) by astromaddie, the OpenXR VR work for Halo on the Frame that moistman42069's fork builds on.
- [OpenCE](https://github.com/OpenCommunityEdition/OpenCE) and halo-ce-universal (MrBruh and contributors), the port OpenH1 forks, and startupfoundry's Arm64 Linux and Steam Frame VR work in it.
- The Xbox Halo decompilation: [punpckhdq/halo](https://github.com/punpckhdq/halo) and [bnunu/halo-1](https://github.com/bnunu/halo-1).
- Bungie, for Halo: Combat Evolved.
- [SDL3](https://github.com/libsdl-org/SDL), the [OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK), and Mesa's Turnip driver.

[CREDITS.md](CREDITS.md) has everyone in full, and [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md) has every library with its licence.

Halo, Halo: Combat Evolved and their logos are trademarks of Microsoft. This is an unofficial fan project. It is not affiliated with or endorsed by Microsoft, Bungie, 343 Industries, Valve, OpenH1 or OpenCE.

## Licence

GNU Affero General Public License, version 3 or later, as OpenH1: see [LICENSE](LICENSE). Third-party parts keep their own licences: see [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md). `steam-frame/tools/vrtiming.py` and `privacy-check.sh` come from Moonlight XR Native's tools, which are GPLv3. No Halo game data is included.

OpenH1's own README is kept as [README.upstream.md](README.upstream.md).

## How this was made

I wanted Halo in VR on my Frame without an emulator or a PC. OpenH1 already had a native Arm64 build with an OpenXR VR mode, so I forked it and set it up for the Frame: the install and launch, recentring, the HUD, faster loads and section changes, resolution and foveation for 90 Hz, and controls made for the Frame controllers. Then I brought in the VR features from moistman42069's mod that I wanted to play with.

This is vibe coded, with manual review throughout. I wrote it with Claude Code running on the Frame itself. If you buy me a coffee, it pays for the Claude API credits these ports are built with, spent as carefully as I can.

Developer notes are in [steam-frame/dev/DEV.md](steam-frame/dev/DEV.md).
