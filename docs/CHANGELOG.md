# Changelog

## 1.0.0 (October 2026)

The first release of OpenFrame: Halo: Combat Evolved in VR, running natively on the Valve Steam Frame. It is my Steam Frame version of OpenH1, with VR features from other people's Halo VR work brought in. If you have played OpenH1's own Frame build, this is the same game, set up for the Frame, and it runs better.

### The game on the Frame

- OpenH1's `build-769` (the fork began at `build-734`): the Xbox game's own code, built for Arm64 Linux, drawn with Vulkan, in VR through the Frame's SteamVR. No emulator, no Lepton, no APK, no PC.
- An installer that copies the game and your maps, adds the Steam library entry, and keeps your saves and settings on an update.
- A launch wrapper that sets SteamVR's per-app settings at every launch: your refresh rate, no throttling, no motion smoothing.
- 90 Hz by default. 72 and 120 Hz are in VR Setup.
- Saves, settings and logs in their own folder, apart from OpenH1's.
- OpenH1's self-updater is switched off, so it never swaps this build for OpenH1.
- A log collector that masks personal details before you share anything.

### What came with OpenH1 build-769

OpenH1's own fixes between `build-734` and `build-769` are in this release:

- Lights past the 128-per-window cap no longer drop lit objects.
- A portal corner that sits on a clip line no longer flashes the level.
- Respawn goes straight to first person.
- The d-pad's focus is kept when you back out of a settings screen.
- Multiplayer, from OpenH1: host controls, OpenH1 games listed first in the server browser, the Kills Score, Multi-Hill and Force Team Spawns rules, and network errors in plain words.

### Performance

- Both eyes at 0.9 of SteamVR's recommended resolution. The full-detail area that follows your eyes is 0.4 of the view.
- With those defaults, firefights in the first level average 88 to 89 frames a second. No frames are dropped in play.
- In the heaviest firefight of the first level, OpenH1's resolution and foveation averaged about 77 fps. Mine average 86 to 89.
- Over a session, SteamVR used to fill in about a third of the frames. Now it is about one in ten.
- The GPU renders straight to memory instead of in tiles. On the Frame that is about 10 fps more in the same rooms.
- OpenH1 froze the world for one to two seconds each time a level moved on to its next section. Now that is a short hitch.
- The first level used to take about 17 seconds to load. Now it takes 4 to 8.

### Fixed

- SteamVR's recentre used to be ignored. The menu sat off to one side and the view could start outside your body. Now recentring works.
- The game recentres by itself on the first frame of each level start, restart and checkpoint reload.
- Menus open in front of you.
- At the start in the cryo tube, the tutorial's look tests waited for the stick. Now looking with your head counts.
- While a script holds you still, your view stays with the player's head.
- The HUD's corners used to sit partly outside the Frame's view. Now they default to positions that fit.

### Controls

- Flick the right stick up: sprint on or off, once sprint is on in VR Setup.
- Hold the right stick down for 0.8 seconds: third person on or off.
- Press the right stick in: bullet time on or off, once bullet time is on in VR Setup. Until then, and in third person, it is plain zoom.
- There is no magazine eject on the stick. Reload with X, or take a magazine from your left hip with your left hand.
- A flick down does nothing, so looking around never sets anything off.
- A push on a diagonal while you turn is ignored.
- One click of the left stick while sprinting slides you the whole way, once the slide is on. Click again to stop early.
- Each toggle shows a label on the HUD.

### VR Setup

- VR Setup is split into pages: Display, HUD, Player, Movement and comfort, Hands and weapons.
- A HUD layout page with sliders: bring the corners in, and move or resize each one.
- The refresh rate, foveation and auto recentre are in the menu.
- Left-handed play carries through: the third-person view sits over your left shoulder, and passing the weapon to the other hand saves it as your hand.

### From other people's work

These come from moistman42069's Halo VR mod, rewritten for OpenH1. Each is off until you turn it on in VR Setup (sprint, bullet time, slide and the wall moves, fists, weapon pull, prop throw, hand swap, the wrist HUD and the vignette), and they play offline. Holding the right stick down for 0.8 seconds turns the third-person view on or off, and needs no setting.

- Sprint, by flicking the stick or by swinging your arms.
- Bullet time: the world slows and your head and hands do not.
- Third person on foot (the hold on the right stick needs no setting).
- Slide, wall jump and wall run.
- Fists: punch, stagger and parry.
- Weapon pull: call a gun on the floor to your hand.
- Prop throw: pick up a loose item and throw it.
- Hand swap: pass the weapon to your other hand.
- The wrist HUD: shields, ammo and tracker on the back of your wrist.
- The comfort vignette on foot, not only in vehicles.

The mod is built for the Quest, so on the Frame it would run through Lepton. Here these features run natively inside OpenH1's renderer.

### Known issues

- Big fights with a lot of explosions can dip into the 70s for a few seconds.
- The first few seconds after a level loads run slower.
- The campaign has one save slot. Starting a new game overwrites it.
- In bullet time the sound plays at normal speed.

### Credits

- The OpenH1 team, whose game and VR mode this is built on.
- moistman42069's [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) for the features listed above. Their fork builds on astromaddie's [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR), and its own credits carry over.
- OpenCE and the decompilation of the Xbox game it starts from.
- Everyone else is in [CREDITS.md](../CREDITS.md).
