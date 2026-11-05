# Changelog

## 1.1.0 (November 2026)

OpenFrame is now on OpenH1 `build-1615`. That brings in a lot of OpenH1's work since 1.0.0. The controls and the VR features from 1.0.0 stay as they were. You still need the maps from your own Xbox disc; no game data is included.

### What came with OpenH1 build-1615

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

### Settings and controls

- VR SETUP is in Settings > CONTROLS now, in the CONTROLLER section.
- The crouch is OpenH1's now. CROUCH sets hold or toggle. CROUCH WALK is on, so a full push of the stick keeps you crouched, as in 1.0.0.
- MAP STORAGE in Settings > PROFILES sets how much room the map cache takes.
- Everything else about the controls is as it was. [CONTROLS.md](CONTROLS.md) has all of them.

### Install and update

- The map cache holds every stock map unpacked, about 3 GB. You need about 5 GB free in all. DISK SPACE SAVER in MAP STORAGE keeps the cache to 730 MB.
- After an update, the game unpacks the maps in the background while you play.
- The tarball carries OpenH1's `openh1.assets` beside the game. The installer puts it in place.
- Install 1.1.0 over 1.0.0 the same way. Your maps, saves and settings stay.

### Known issues

- The campaign has one save slot per profile. Starting a new game, or any level, overwrites it. Copy the saves folder aside first if you want to come back.
- In bullet time the sound plays at normal speed.
- The launch wrapper can only set SteamVR's refresh rate and smoothing when the game starts from the Steam library.

### Credits

- The OpenH1 team, for everything in build-1615 listed above, and for the game and VR mode this is built on.
- moistman42069's [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) for sprint, bullet time, the third person, parkour, fists, weapon pull, prop throw, hand swap, the wrist HUD and the vignette on foot.
- Everyone else is in [CREDITS.md](../CREDITS.md).

## 1.0.0 (October 2026)

The first release of OpenFrame: Halo: Combat Evolved in VR for the Steam Frame, running natively. It is my Steam Frame version of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo for Arm64 Linux with an OpenXR VR mode, with VR features from other people's Halo VR work brought in and credited. If you have played OpenH1's own Frame build, this is the same game, set up for the Frame, and it runs better. You need the maps from your own Xbox disc; no game data is included.

### The game on the Frame

- OpenH1 `build-769`: the Xbox game's own code, built for Arm64 Linux, drawn with Vulkan, in VR through the Frame's SteamVR. No emulator, no Lepton, no APK, no PC.
- An installer that copies the game and your maps, adds the Steam library entry, and keeps your maps, saves and settings on an update. An uninstaller beside it.
- A launch wrapper that sets SteamVR's per-app settings each time you start from the library: your refresh rate, no throttling, no motion smoothing.
- 90 Hz by default. 72 and 120 Hz are in VR Setup.
- Saves, settings and logs in their own folder, apart from OpenH1's.
- OpenH1's self-updater is switched off, so it never swaps this build for OpenH1.
- A log collector that masks addresses, names and ids before you share anything.

### What came with OpenH1 build-769

OpenH1's own work since its first Frame build is in here too. Lights past the 128-per-window cap no longer drop lit objects, a portal corner on a clip line no longer flashes the level, respawn goes straight to first person, and the d-pad keeps its place when you back out of a settings screen. Multiplayer gained host controls, OpenH1 games first in the server browser, the Kills Score, Multi-Hill and Force Team Spawns rules, and network errors in plain words. All of that is OpenH1's.

### Performance

- Both eyes at 0.9 of SteamVR's recommended resolution. Full detail over the middle 0.4 of each eye's image, across and up, where you look.
- The GPU renders straight to memory instead of in tiles. On the Frame that is about 10 fps more in the same rooms.
- In the heaviest firefight of the first level, OpenH1's own settings averaged about 77 fps. With mine it averages 86 to 89, and the first level's firefights hold the display's 90 Hz most of the time.
- Over a session, SteamVR used to fill in about a third of the frames. Now it is about one in ten.
- OpenH1 froze the world for one to two seconds each time a level moved on to its next section. Now that is a short hitch.
- The first level used to take about 17 seconds to load. Now it takes about 5.

### Fixed

What OpenH1's own Frame build did, and what this one does instead.

- SteamVR's recentre was ignored, so the menu sat off to one side and the view could start outside your body. Now recentring works, and putting the headset back on recentres too.
- The game recentres by itself on the first frame of each level start, restart and checkpoint reload, and when play comes back after a cinematic. Menus open in front of you. AUTO RECENTRE on the PLAYER page turns this off.
- At the start in the cryo tube, the tutorial's look tests waited for the stick. Now looking with your head counts.
- While a script holds you still, your head can lean the view up to 15 cm from the player's eye instead of the view being locked. HELD LEAN on the PLAYER page sets the distance.
- The HUD's corners sat partly outside the Frame's view. Now they default to positions that fit, and you can move them.
- Pushing the left stick all the way dropped the crouch. Now a click of the stick crouches at a full run.
- The right stick pushed down used to eject your magazine. It no longer does anything by itself: reload with X or take a magazine from your left hip.

### Controls

The controls, the turning stick's gestures and how each added feature is used are in [CONTROLS.md](CONTROLS.md).

### From other people's work

These come from moistman42069's Halo VR mod, rewritten for OpenH1's renderer and the Frame's controllers. The mod is an Android build, so on the Frame it would run through Lepton; here it runs natively. Each is off until you turn it on in VR Setup, apart from the third person, which needs no setting.

- Sprint, by a flick of the stick or by swinging your arms.
- Bullet time: the world slows and your head and hands do not.
- Third person on foot.
- Parkour: slide, wall jump and wall run.
- Fists: punch, stagger and parry.
- Weapon pull: call a gun on the floor to your hand.
- Prop throw: pick up a loose item and throw it.
- Hand swap: pass the weapon to your other hand.
- The wrist HUD: shields, ammo and tracker on the back of your wrist.
- The comfort vignette on foot, not only in vehicles.
- A HUD layout page with sliders to bring the corners in and move or resize each one.

### VR Setup

- VR Setup is split into pages: DISPLAY, HUD, PLAYER, MOVEMENT AND COMFORT, HANDS AND WEAPONS, each with its sub-pages.
- DISPLAY: RESOLUTION, FOVEA SIZE and REFRESH RATE, 72, 90 or 120 Hz. These take effect at the next launch.
- Left-handed play carries through: the sticks swap, the pouch and holsters mirror, the third-person camera sits over your left shoulder, and a hand swap saves your hand.
- The menus show which build you are running.

### Known issues

- Big fights with many explosions fall into the 60s for as long as the fight lasts.
- The first few seconds after a level loads run slower.
- The campaign has one save slot per profile. Starting a new game, or any level, overwrites it. Copy the saves folder aside first if you want to come back.
- In bullet time the sound plays at normal speed.
- The launch wrapper can only set SteamVR's refresh rate and smoothing when the game starts from the Steam library.

### Credits

- The OpenH1 team, whose game and VR mode this is built on.
- moistman42069's [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) for the features listed above. Their fork builds on astromaddie's [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR), and its own credits carry over.
- OpenCE and the decompilation of the Xbox game it starts from.
- Everyone else is in [CREDITS.md](../CREDITS.md).
