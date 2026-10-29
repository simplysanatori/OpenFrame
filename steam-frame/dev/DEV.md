# OpenFrame developer notes

OpenFrame is the Steam Frame fork of [OpenH1](https://github.com/OpenH1/OpenH1): native Arm64 Linux
Halo: Combat Evolved with OpenXR VR, drawn with Vulkan (Mesa's Turnip on the Frame's Adreno), run in
the Frame's own SteamVR. This folder (`steam-frame/`) is the Frame packaging layer: build, install,
launch wrapper, logs, release. It touches nothing under `source/`, `port/`, `tools/` or
`configure.py`. The branch `steam-frame` began at OpenH1's release tag `build-734`; its current base is `build-769`.

Everything below is what the scripts do. What has been run and measured on a Frame is in
[What is confirmed](#what-is-confirmed).

## Layout

| File | What |
|---|---|
| `build.sh` | the OpenH1 build, serialised: `python3 configure.py --vr --release`, then `ninja linux_arm64` |
| `install.sh` | installs a release tarball or unpacked release; `--dev` a build tree as "OpenFrame (dev)"; `--from-build [WORKTREE]` a build tree as the normal install (it runs `release.sh --stage` into a temp dir) |
| `uninstall.sh` | removes the shortcut and the program folder; `--purge` also the data |
| `openframe.sh` | the Steam shortcut's launch wrapper |
| `steam-cef.py` | adds and edits Steam shortcuts through Steam's local debugging port, or offline in a `shortcuts.vdf` |
| `collect-logs.sh` | one archive of a session's logs for a bug report, personal data masked |
| `release.sh` | `openframe-<version>-linux-arm64.tar.gz` and `SHA256SUMS` |
| `tools/vrtiming.py` | SteamVR compositor timing, a line a second |
| `tools/privacy-check.sh` | looks for personal data in our files, the history or a tarball |
| `art/` | Steam library images (placeholder; see its README) |

## Build

```
steam-frame/build.sh                 # build/linux_arm64/halo and libSDL3.so.0
```

`build.sh` runs what OpenH1 documents (`python3 configure.py --vr --release`, then `ninja
linux_arm64`), with ccache as the compiler launcher, `env -u LD_LIBRARY_PATH` (Steam's runtime
libraries must not reach the toolchain), `nice` and `-j4`. It takes a lock so only one heavy job runs
at a time, and waits (and pauses a running build) while `~/.cache/headset-in-use` names a live process.
The first configure downloads the pinned guest LLVM (about 1.5 GB), musl, SDL3 and the Vulkan and
OpenXR headers. The game is C89 (`gnu89`); `AGENTS.md` has the repository's conventions. Environment:
`OPENFRAME_BUILD_CONFIG` (`release` or `debug`), `OPENFRAME_CONFIGURE_FLAGS`, `OPENFRAME_RECONFIGURE=1`,
`OPENFRAME_CONFIGURE_ONLY=1`, `JOBS`, `CCACHE_DIR`.

## Install a build for testing

```
steam-frame/install.sh --dev --maps /path/to/maps    # "OpenFrame (dev)" in ~/Games/OpenFrame/app-dev
steam-frame/install.sh --from-build [WORKTREE]       # install a build tree like a release: "OpenFrame" in app/, no tarball
OPENFRAME_SKIP_STEAM=1 steam-frame/install.sh --dev  # trial mode: no Steam shortcut, Steam need not run
```

How the install maps onto OpenH1's settings (OpenH1's `port/linux/README.md`, "Settings" and "Files and
folders"):

- The game and `libSDL3.so.0` go into the app folder (`~/Games/OpenFrame/app`); the binary finds the
  library beside itself (its rpath is `$ORIGIN`).
- `OPENFRAME_HOME` (default `~/.local/share/OpenFrame`) is the data folder: `config.toml`, `maps/`,
  `saves/`, `brokers.txt`, `sessions/`, and the game's own `debug.txt`.
- `config.toml` is written once. `install.sh` sets `paths.data` (the folder that contains `maps/`),
  `paths.saves` (`$OPENFRAME_HOME/saves`, so the fork's saves and map cache stay apart from an OpenH1
  install's `~/.local/share/halo-linux`), `update.auto = false`, and `vr.refresh_rate` (90.0) when the file has
  none. Every other line is left as it is. The wrapper points the game at it with `HALO_CONFIG_FILE`.
- `update.auto = false` matters: OpenH1's self-updater downloads OpenH1's own release package and would
  replace this fork. The wrapper also exports `HALO_UPDATE_AUTO=false` on every launch, and never sets
  `HALO_FRAME_LAUNCHER`, which the updater needs before it looks for a release at all. A code-level
  disable of the updater is a separate change in `port/linux/src/updater.c` (not part of this folder).
- `brokers.txt` (internet play's public MQTT brokers) is copied beside `config.toml`, where
  `network.brokers_file` finds it. Internet play is OpenH1's and is on by default
  (`network.online`); it talks to those public brokers when you use it.
- The 24 `.map` files (about 1.8 GB) are copied from the folder you give with `--maps` into
  `$OPENFRAME_HOME/maps` (via `maps.partial`, renamed when complete). They are never bundled.

## The wrapper

`openframe.sh` is the shortcut's launch option: `"<app>/openframe.sh" %command%`, with the shortcut's
program `<app>/halo`. Steam's `%command%` is a chain of its own helpers ending in that binary; the
wrapper runs the chain as given and does not `exec` it, so it can stop the timing logger and collect the
logs afterwards. Per launch it writes `$OPENFRAME_HOME/sessions/<stamp>/` (`launch.log`, `stdout.log`,
`vrtiming.log`, plus `debug.txt` and `perfcriteria.txt` when this run wrote them), a `latest` symlink, and
prunes to the newest 30. Settings (environment, or `session.env`): `OPENFRAME_REFRESH_HZ` (72, 90 or 120;
when given, asked of SteamVR as `preferredRefreshRate` and of the game as `HALO_VR_REFRESH_RATE`; when not,
nothing is exported and the game uses `vr.refresh_rate` from `config.toml`, so VR Setup's REFRESH RATE works,
and SteamVR is asked for that same rate: 90 when the file has none, and 90 with a warning in `launch.log`
when it holds a rate the Frame lacks),
`OPENFRAME_PACING` (reserved; passed through), `OPENFRAME_SYSMEM` (default on, `TU_DEBUG=sysmem`),
`OPENFRAME_MOTION_SMOOTHING`, `OPENFRAME_VRTIMING=0`, `OPENFRAME_KEEP_SESSIONS`. Each launch re-applies
SteamVR's per-app settings with `vrcmd` (`preferredRefreshRate`, `framesToThrottle=0`,
`additionalFramesToPredict=0`, `motionSmoothingOverride=2`) and reads them back into `launch.log`.

`session.env` is sourced for one launch and renamed `session.env.used`; a `session.env.next` becomes
the next launch's `session.env` (two-launch tests). Any `HALO_*` variable in it reaches the game and wins
over `config.toml`.

## Release

```
steam-frame/release.sh --version 1.0.0   # steam-frame/release/openframe-1.0.0-linux-arm64.tar.gz + SHA256SUMS (the script's default version)
```

Needs a finished build. The tarball has the game, the library, the scripts, `brokers.txt`, `art/` (if it
has images), the docs and the licence files. It never has the maps, a `config.toml`, a save or a log.
`privacy-check.sh` runs on the tarball and the tarball is deleted if it finds anything. Bundled third-party
licence texts (`licenses/`, `LICENSE*`, `COPYING*`, `THIRD-PARTY-NOTICES.md`) are shipped verbatim and are exempt from the e-mail check only.

## Privacy check

```
steam-frame/tools/privacy-check.sh              # our files since build-769 (the base)
steam-frame/tools/privacy-check.sh --history    # commits after build-769: lines, authors, messages
steam-frame/tools/privacy-check.sh TARBALL|DIR
```

Personal strings (real names, e-mail, SSIDs, hosts) live in `~/.config/openframe-privacy.txt`, one
case-insensitive regular expression a line, outside the repository. The script falls back to the list the
other Frame ports read. Commits must be authored and committed by a GitHub no-reply address.

## Left out on purpose

- **Turning-stick magazine eject.** OpenH1's right stick pushed down ejected the magazine
  (`vr.manual_reload`). OpenFrame removed it entirely: in play it left
  the HUD showing rounds (e.g. 29) while the gun could not fire, which was confusing. A flick down
  does nothing (the log says `vr: stick down: held ... -> nothing`); the hand eject (the left grip at
  the left hip takes a new magazine and the old one ejects itself) and X reload stay. There is no
  config key that brings the stick eject back; do not re-add it as a default path.

## What is confirmed

Run on a Steam Frame (SteamOS, SteamVR, the Frame's controllers and eye tracking), version 1.0.0, at the
shipped defaults, in the first campaign level:

- The installer on a Frame with Steam running: the program folder, `config.toml`, the maps and the
  "OpenFrame" Steam shortcut (OpenVR on) are made, and an update keeps maps, saves and settings.
- The wrapper under Steam: SteamVR takes the per-app settings (refresh rate, no throttling, no motion
  smoothing) and `launch.log` reads them back; the timing logger writes `vrtiming.log` each session.
- 90 Hz, eyes at 0.9 of the recommended 1728 (1554x1554), eye-tracked foveation with an inset of 0.4,
  `TU_DEBUG=sysmem`: firefights average 88.5 fps over two rooms, none below 70; the heaviest firefight
  averages 86 to 89 fps (OpenH1's 1.0 and 0.5: about 77); GPU time is 7.2 ms median of the 11.1 ms frame;
  no dropped frames in play.
- SteamVR fills in about one frame in ten over a session (OpenH1's resolution and foveation: about one in
  three). Tiled rendering instead of `sysmem` was about 10 fps slower in the same rooms.
- A level loads in 4 to 8 seconds (OpenH1: about 17 for the first level); a section change is a hitch of
  about 0.4 s of preparation (OpenH1: a freeze of one to two seconds).
- Recentring (both grips, SteamVR's own, and the automatic one at each level start), the menus in front of
  you, and the HUD layout defaults.
- The controls in the README: the right stick press (bullet time once enabled, else zoom), flick up
  (sprint), hold down for 0.8 s (third person), the left stick click while sprinting (slide), reload by
  X or the left-hip magazine, and no stick eject.

The figures are for 90 Hz in the first level; other refresh rates and levels are not measured here.
