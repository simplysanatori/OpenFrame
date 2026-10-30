# Changelog

## 1.0.0 (October 2026)

The first release of OpenFrame: Halo: Combat Evolved in VR, running natively on the Valve Steam Frame. It is my Steam Frame version of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo for Arm64 Linux with an OpenXR VR mode, with VR features from other people's Halo VR work brought in and credited. If you have played OpenH1's own Frame build, this is the same game, set up for the Frame, and it runs better. You need the maps from your own Xbox disc; no game data is included.

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
- The right stick pushed down used to eject your magazine. It no longer does anything by itself: see the controls below.

### Controls

The turning stick is the right stick, or the left when you play left-handed. Flicks must be within about 34 degrees of straight up or down to count. While the stick is inside that cone, turning pauses, so a flick never turns you. A diagonal push only turns you.

- Flick the turning stick up: sprint on or off. Needs SPRINT set to TOGGLE on the MOVEMENT AND COMFORT page.
- Flick the turning stick down and let go within 0.8 seconds: bullet time on or off. Needs BULLET TIME on. It switches as you let go, and rests half a second between switches.
- Hold the turning stick down for 0.8 seconds: third person on or off. Needs no setting.
- Press the turning stick in: zoom, as in the flat game, in bullet time as well. Nothing in this release holds the click.
- Reload with X. Or squeeze your off hand's grip at the pouch on your left hip to take a magazine: the empty one drops out of the weapon by itself, and pushing the new one into the weapon loads it.
- Click the left stick while sprinting at full speed: slide. Needs PARKOUR on.
- Sprint, bullet time and the third person each show a label on the HUD when they switch.

The GESTURES page offers other bindings for bullet time: a held click, a plain click, a double flick down, a double flick up, a held push up, or off. The third person can be a double flick down instead of a hold.

### From other people's work

These come from moistman42069's Halo VR mod, rewritten for OpenH1's renderer and the Frame's controllers. The mod is an Android build, so on the Frame it would run through Lepton; here it runs natively. Sprint, bullet time, parkour, fists, weapon pull, prop throw and hand swap work in the campaign and are each off until you turn them on in VR Setup. The third person works in the campaign with no setting. The wrist HUD and the vignette are options for everywhere.

MOVEMENT AND COMFORT page:

- SPRINT: TOGGLE for the stick flick, or ARM SWING to run faster by swinging both arms. SPRINT SPEED sets how much faster, 1.5 times by default.
- BULLET TIME: the world slows to 40 % while your head and hands stay at full speed. BULLET TIME SPEED sets the slowdown. It ends by itself at a cinematic or when you die.
- THIRD PERSON and VIEW DISTANCE: the camera floats 1.7 m behind you, over the shoulder of your weapon hand, and tucks in at walls. Zooming, a vehicle or a cinematic puts you back in first person.
- PARKOUR: slide, wall jump and wall run.
  - Slide: crouch while sprinting at full speed, by the stick click or by ducking your head. A click-started slide runs its whole way with the stick let go, up to 1.1 seconds. Click again to stand early. SLIDE MOMENTUM sets its length and speed.
  - Wall jump: in the air beside a wall, press jump again. You leap where you look. WALL JUMP sets the strength.
  - Wall run: jump at a wall with the stick pushed along it. You run along the wall for up to two seconds. Steer away to drop, or jump off it.
- VIGNETTE: VEHICLES as in OpenH1, ALWAYS to also narrow the view when you move or turn on foot, or OFF. VIGNETTE STRENGTH sets how much.

HANDS AND WEAPONS page, under FISTS AND GRABBING:

- FISTS: your free hand punches. Swing it at an enemy at 2 m/s or more. A hard swing staggers them for a second. Swipe across an enemy's melee as it lands to parry it; the HUD says "Parry". PUNCH SPEED and PUNCH DAMAGE tune it.
- WEAPON PULL: hold your free hand out, point it at a weapon on the floor within PULL RANGE, 3 m by default, squeeze the grip and flick the hand back. The weapon flies to your hand. Keep the grip squeezed to catch it.
- PROP THROW: squeeze the grip with your free hand on a loose weapon or piece of equipment to pick it up, and let go to throw it. THROW STRENGTH scales the throw.
- HAND SWAP: bring your empty hand to the weapon, squeeze its grip and open the other. The weapon changes hands, and the game remembers which hand is yours. The sticks stay where they were.

HUD page:

- HUD STYLE: VISOR as in OpenH1, or WRIST to carry shields, ammo and the motion tracker on the back of your off hand's wrist. Turn the wrist toward your face to read it. WRIST HUD sets its size and place on the arm. The reticle, messages and waypoints stay on the visor, and the visor comes back in menus and in vehicles.
- HUD LAYOUT: sliders in percent to bring the corners in, and to move or resize the ammo, shields and tracker, together or one by one. Changes apply when you press OK on the page.

### VR Setup

- VR Setup is split into pages: DISPLAY, HUD, PLAYER, MOVEMENT AND COMFORT, HANDS AND WEAPONS, each with its sub-pages.
- DISPLAY: RESOLUTION, FOVEA SIZE and REFRESH RATE, 72, 90 or 120 Hz. These take effect at the next launch.
- Left-handed play carries through: the sticks swap, the pouch and holsters mirror, the third-person camera sits over your left shoulder, and a hand swap saves your hand.
- The menus show which build you are running.

### Known issues

- Big fights with many explosions fall into the 60s for as long as the fight lasts.
- The first few seconds after a level loads run slower.
- In bullet time the sound plays at normal speed.
- The launch wrapper can only set SteamVR's refresh rate and smoothing when the game starts from the Steam library.

### Credits

- The OpenH1 team, whose game and VR mode this is built on.
- moistman42069's [HaloCE-DecompRemix-VR](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android) for the features listed above. Their fork builds on astromaddie's [HaloCE-VR](https://github.com/astromaddie/HaloCE-VR), and its own credits carry over.
- OpenCE and the decompilation of the Xbox game it starts from.
- Everyone else is in [CREDITS.md](../CREDITS.md).
