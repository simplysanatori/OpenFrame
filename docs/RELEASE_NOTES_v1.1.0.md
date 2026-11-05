# OpenFrame 1.1.0

Halo: Combat Evolved in VR for the Steam Frame, running natively. OpenFrame is my Steam Frame version of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo built for Arm64 Linux with an OpenXR VR mode. This release moves it to OpenH1 `build-1615`. That brings in a lot of OpenH1's work since 1.0.0. The controls and the VR features from 1.0.0 stay as they were. You need the maps from your own Xbox disc; no game data is included.

## What came with OpenH1 build-1615

All of this is OpenH1's work. I merged it and set it up for the Frame.

- Dynamic lights reach only what the game itself lights. A closed door stops them.
- A pipeline that is not ready is built in the background. The frame does not wait for it.
- A map's first frame also prepares its mirrors, its fog and its lens flares.
- Checkpoints are saved on a thread of their own.
- The menus open without a hitch.
- The stock maps stay unpacked in a cache, so a load never waits to unpack one.
- Crossing into the next section of a level shows no loading screen.
- The loading screen is sharper.
- The reverb sounds as it did on the Xbox.
- Sounds go through the HRTF on any stereo output.
- In VR, whether a wall muffles a sound is judged from your head.
- The VR Tutorial is on the main menu in the headset.
- Fixes in play, among them the flashlight, the needler's reload and picking up a second weapon in multiplayer.
- In multiplayer, your hits on other players show at once.
- Bots drive and fly, and play Capture the Flag, Oddball and King of the Hill.
- Relays connect players whose networks keep them apart.
- The server browser has a search.
- A new Campaign screen. Profiles and Mods are inside Settings now. There is a gamma setting.
- The game runs on kernels with 16 KB and 64 KB pages.

## Settings and controls

- VR SETUP is in Settings > CONTROLS now, in the CONTROLLER section.
- The crouch is OpenH1's now. CROUCH sets hold or toggle. CROUCH WALK is on, so a full push of the stick keeps you crouched, as in 1.0.0.
- MAP STORAGE in Settings > PROFILES sets how much room the map cache takes.
- Everything else about the controls is as it was. [docs/CONTROLS.md](https://github.com/simplysanatori/OpenFrame/blob/steam-frame/docs/CONTROLS.md) has all of them.

## Install and update

- The map cache holds every stock map unpacked, about 3 GB. You need about 5 GB free in all. DISK SPACE SAVER in MAP STORAGE keeps the cache to 730 MB.
- After an update, the game unpacks the maps in the background while you play.
- The tarball carries OpenH1's `openh1.assets` beside the game. The installer puts it in place.
- Install 1.1.0 over 1.0.0 the same way. Your maps, saves and settings stay.

## Known issues

- The campaign has one save slot per profile. Starting a new game, or any level, overwrites it. Copy the saves folder aside first if you want to come back.
- In bullet time the sound plays at normal speed.
- The launch wrapper can only set SteamVR's refresh rate and smoothing when the game starts from the Steam library.

## Download

- `openframe-1.1.0-linux-arm64.tar.gz`
- `SHA256SUMS`

## Install

Clone the repository, then run `steam-frame/install.sh --maps` with the folder of your 24 Xbox `.map` files and the tarball as its argument. It installs the game, creates or updates the Steam shortcut and keeps your maps, saves and settings. The [README](https://github.com/simplysanatori/OpenFrame#install) has the steps, the controls and the settings, and the [changelog](https://github.com/simplysanatori/OpenFrame/blob/steam-frame/docs/CHANGELOG.md) has this list.

## Credits

The OpenH1 team, for everything in build-1615 listed above, and for the game and VR mode this is built on; moistman42069's [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) for the VR features from 1.0.0, itself built on astromaddie's [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR); OpenCE and the Xbox decompilation it starts from. Everyone else is in [CREDITS.md](https://github.com/simplysanatori/OpenFrame/blob/steam-frame/CREDITS.md).

## Reporting problems

Open an issue with what you did, what you expected and what happened, and the archive from `~/Games/OpenFrame/app/tools/collect-logs.sh`. It masks addresses, names and ids before anything leaves your Frame. A problem in OpenH1 itself belongs on [OpenH1's tracker](https://github.com/OpenH1/OpenH1/issues).
