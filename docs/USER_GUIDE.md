# OpenFrame user guide

The detail behind the [README](../README.md): install options, every setting, performance, files and the full troubleshooting list. The controls are in [CONTROLS.md](CONTROLS.md), and every VR feature's settings in [openframe-features.md](openframe-features.md).

## Known issues

- The campaign has one save slot. Starting a new game overwrites it (see [Files](#files)).
- In bullet time the sound plays at normal speed.
- The launch wrapper can only set SteamVR's refresh rate and smoothing when the game starts from the Steam library.

## Requirements in detail

- A Steam Frame with SteamOS and SteamVR, and about 5 GB free: the maps are 1.8 GB, and the game keeps them unpacked in a cache of about 3 GB. MAP STORAGE set to DISK SPACE SAVER keeps the cache to 730 MB.
- The 24 `.map` files from the `maps` folder of your own Xbox disc of Halo: Combat Evolved (about 1.8 GB). Halo PC's maps and the MCC's do not work. Every version of the Xbox game works; the game plays PAL maps as the North American ones. [openh1.com/maps](https://openh1.com/maps) explains how to make and read a disc image.
- A terminal on the Frame for the one-time setup: SSH, or Konsole from desktop mode.
- Steam running on the Frame while you install, so the installer can add the library entry.

No game data is in this repository or its releases.

## Install options

`SHA256SUMS` names the tarball without a folder, so run `sha256sum -c --ignore-missing SHA256SUMS` in the folder you downloaded both files to (the README uses `~/Downloads`).

`--maps` takes the folder with your 24 `.map` files, or the folder that holds `maps/`. They are copied, not moved, into `~/.local/share/OpenFrame/maps`.

The installer needs no sudo. `OPENFRAME_SKIP_STEAM=1` installs without touching Steam.

To update, install a newer tarball the same way. Your maps, saves and settings stay where they are. OpenH1's own updater is off: it would replace OpenFrame with OpenH1.

To remove OpenFrame, run `~/Games/OpenFrame/app/tools/uninstall.sh`. It keeps your data folder unless you pass `--purge`.

### Install with Frametanium

If you use [Frametanium](https://github.com/Cogential/frametanium), add `simplysanatori/OpenFrame` there. It downloads the release, runs its `install.sh` and updates it when I publish a new version. Copy the 24 `.map` files of your Xbox disc into `~/.local/share/OpenFrame/maps` yourself; the installer sets up the settings file for that folder. Without maps it still installs and says so.

### Install with FrameDrop

[FrameDrop](https://framedropvr.com) can install this too: [Install with FrameDrop](https://framedropvr.com/install?manifest=https%3A%2F%2Fgithub.com%2Fsimplysanatori%2FOpenFrame%2Freleases%2Flatest%2Fdownload%2Fopenframe.framedrop.json). The release has a `openframe-<version>-framedrop-linux-arm64.zip` and a `openframe.framedrop.json` for it. Copy the maps of your own Xbox disc into `~/.local/share/OpenFrame/maps` first, or run `install.sh --maps DIR` once.

## First run

On the first start the game makes your profile in `~/.local/share/OpenFrame/saves`, and unpacks the maps into its cache in `saves/z`, about 3 GB in all.

Sit or stand as you mean to play when the loading picture fades: the game recentres on that first frame. Hold both grips for a moment at any time to recentre again.

## Settings

Most settings are in the headset: **Settings > CONTROLS > VR SETUP**, in the CONTROLLER section. It appears while the game plays in VR, and also at the foot of the player profile menu. Its pages are **DISPLAY...**, **HUD...**, **PLAYER...**, **MOVEMENT AND COMFORT...** and **HANDS AND WEAPONS...**. The HUD page opens **HUD LAYOUT...**, with sliders to bring the corners in and to move or resize each one. MOVEMENT AND COMFORT opens **GESTURES...**. CROUCH and CROUCH WALK are just below VR SETUP on the CONTROLS tab.

Everything is also in `~/.local/share/OpenFrame/config.toml`, with a comment on each line. A line commented out follows the default. A comment is written when its setting first goes into the file, and an update does not rewrite it. Every setting has a `HALO_*` environment variable too, which wins over the file. The ones that matter most on the Frame:

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
| `vr.runtime_wait` | 60 | Seconds to wait at the start for SteamVR and the headset, then quit. 0 does not wait |

For one launch only, put `NAME=value` lines in `~/.local/share/OpenFrame/session.env`. It is read once and renamed `session.env.used`. The wrapper's own settings:

| Variable | Default | What it does |
|---|---|---|
| `OPENFRAME_REFRESH_HZ` | unset | 72, 90 or 120 for this launch, instead of `vr.refresh_rate` |
| `OPENFRAME_SYSMEM` | on | The GPU renders straight to memory (`TU_DEBUG=sysmem`). `0` goes back to tiled rendering, about 10 fps slower |
| `OPENFRAME_MOTION_SMOOTHING` | 2 (off) | SteamVR's motion smoothing for the game |
| `OPENFRAME_VRTIMING` | on | `0` turns off the SteamVR timing log |

[steam-frame-defaults.md](steam-frame-defaults.md) explains each Frame default against OpenH1's.

### Left-handed play

VR Setup > PLAYER... > **DOMINANT HAND** = LEFT. The left controller then fires, aims and draws from the holsters on your left, and the right hand holds the foregrip, takes magazines from your right hip and throws grenades. **SWAP STICKS** follows the hand by default, so the right stick moves and crouches, and the left one turns, zooms and carries the stick gestures. A, B, X, Y, the bumpers, the d-pad, View and Menu stay where they are. The third-person view sits over your left shoulder and the wrist HUD moves to your right wrist.

### Comfort

Seated play, physical crouch, snap or smooth turning, the vignette, world scale, and the HUD's and menus' size and distance are all in VR Setup, on the PLAYER..., HUD... and MOVEMENT AND COMFORT... pages.

### Internet play

Internet play is OpenH1's and is on by default. Invites and the server browser use the public MQTT brokers in `brokers.txt` and OpenH1's master servers. `network.online = false` in `config.toml` turns it off and keeps system link on your local network. OpenFrame plays online with OpenH1 and OpenCE players. The added features stay offline.

## Performance

Measured on my Frame with 1.0.0, from the session logs, at the defaults, in the first level:

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

- `launch.log` in each session folder shows the refresh rate asked for and whether SteamVR took the per-app settings. `vrtiming.log` has SteamVR's timing per second.
- Keep builds and other heavy work off the Frame while you play.

## Files

| What | Where |
|---|---|
| Program | `~/Games/OpenFrame/app` (`halo`, `libSDL3.so.0`, `openframe.sh`, `tools/`) |
| Settings, maps, saves | `~/.local/share/OpenFrame` (`config.toml`, `maps/`, `saves/`, `brokers.txt`) |
| Campaign save | `saves/u/UDATA/<profile id>/savegame.bin`. One slot per profile: **starting a new game, or any level, overwrites it**. Copy `saves/` aside first if you want to come back |
| Map cache | `saves/z`: the maps unpacked, about 3 GB. Settings > PROFILES > MAP STORAGE sets its size |
| Session logs | `~/.local/share/OpenFrame/sessions/<date-time>/` (`launch.log`, `stdout.log`, `debug.txt`, `vrtiming.log`); `sessions/latest` points at the newest |
| One launch's settings | `~/.local/share/OpenFrame/session.env` |

The Vulkan pipeline cache is shared with OpenH1, in `~/.cache/openh1`. Nothing else is written to OpenH1's folders.

## Troubleshooting

- **`install.sh` says Steam's debugging port is not reachable.** Start Steam and run it again, or set `OPENFRAME_SKIP_STEAM=1`.
- **The game asks for the disc image at start.** It did not find `maps/` in `~/.local/share/OpenFrame`. Run `install.sh --maps` with your maps folder.
- **It takes a while to start, or quits by itself after a minute.** When SteamVR is still starting, or the headset is asleep, the game waits for them, trying once a second, for up to `vr.runtime_wait` seconds (60 by default). If they are still not ready, it quits, since a flat window cannot be seen in the headset. Wake the headset, let SteamVR start, and start the game again. `stdout.log` says `vr: waiting up to 60 s for SteamVR and the headset` and why. Steam's Exit game ends it while it waits. `vr.runtime_wait = 0` turns the wait off.
- **It plays flat in a window.** SteamVR was not running when the game started, and the game found no OpenXR runtime at all. Start SteamVR first.
- **The menu is off to the side, or the view is not where the player is.** Hold both grips for a moment to recentre. SteamVR's own recentre works too.
- **The HUD's corners are cut off,** for example with a glasses spacer. VR Setup > HUD... > HUD LAYOUT... > **HUD INSET** brings them all in. Each corner also has its own sliders.
- **Something went wrong.** `~/.local/share/OpenFrame/sessions/latest/stdout.log` is the game's output, with a crash's backtrace.

## Reporting a problem

Open an [issue](https://github.com/simplysanatori/OpenFrame/issues) with what you did, what you expected and what happened, and the output of:

```sh
~/Games/OpenFrame/app/tools/collect-logs.sh
```

It gathers the newest session's logs into one archive and masks IP and MAC addresses, host names, Wi-Fi names, serial numbers, Steam ids, email addresses, tokens and user names in paths. The masking is a best effort. Read the files before you attach them. A problem in OpenH1 itself belongs on [OpenH1's tracker](https://github.com/OpenH1/OpenH1/issues).

## Support

OpenFrame exists because of [OpenH1](https://github.com/OpenH1/OpenH1), [OpenCE](https://github.com/OpenCommunityEdition/OpenCE) and the Xbox decompilation they start from, and because of moistman42069's [Halo VR mod](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android). If you want to support someone, support them first: star and follow them, report what you find there, and send fixes that are not Frame-specific upstream.

If you would like more Steam Frame ports, you can also [buy me a coffee](https://ko-fi.com/sana748653). Thank you.
