# OpenFrame

Halo: Combat Evolved in VR, running natively on the Valve Steam Frame.

OpenFrame is the Steam Frame fork of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo: Combat Evolved. The game is the Xbox build's own code, compiled for 64-bit Arm Linux and drawn with Vulkan, with OpenXR VR through the Frame's SteamVR. There is no emulator, no Lepton, no APK and no PC. OpenFrame adds the Steam Frame packaging layer (install, launch wrapper, Steam shortcut, logs, release) and brings VR gameplay features from the Halo VR mods (sprint, bullet time, parkour, fists, a wrist HUD and more) into OpenH1's VR mode.

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
- The VR features ported from the Halo VR mods are in the game and build, but none has been played in a headset (each is marked below).
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

     `--dev` installs the build as "OpenFrame (dev)" in `app-dev`. To install a build exactly like a release (the normal `app` folder and "OpenFrame" shortcut) without making a tarball, use `steam-frame/install.sh --from-build [WORKTREE] --maps ...`; it stages the same files `release.sh` packs from `WORKTREE/build/linux_arm64` (default: this checkout).

   `--maps` takes the folder with your 24 `.map` files, or the folder that contains `maps/`. They are copied, not moved, into `~/.local/share/OpenFrame/maps`.

   The installer copies the program to `~/Games/OpenFrame/app` (`app-dev` with `--dev`; `--from-build` installs into `app`), writes `~/.local/share/OpenFrame/config.toml`, and adds **OpenFrame** to your Steam library as a non-Steam shortcut (with its OpenVR, Steam overlay and desktop configuration flags on), or updates the entry if it is there. It talks to Steam through its local debugging port, which SteamOS has on by default. It needs no sudo.

3. Start SteamVR, put the headset on and launch **OpenFrame** from your Steam library.

To update, install a newer tarball the same way. Your maps, saves and settings stay where they are. To remove it: `~/Games/OpenFrame/app/tools/uninstall.sh` (add `--purge` to delete the data folder too).

The installer changes nothing outside `~/Games/OpenFrame`, `~/.local/share/OpenFrame`, the Steam shortcut and its art, and SteamVR's settings for that shortcut (which the launch wrapper sets at each launch).

### Updates

OpenH1 updates itself from its own GitHub releases. That would replace OpenFrame with OpenH1, so the installer writes `update.auto = false` into `config.toml` and the launch wrapper exports `HALO_UPDATE_AUTO=false` on every start. As installed here the game's update check does not run at all, because it needs OpenH1's own Steam Frame launcher and package files, which OpenFrame does not use; the setting and the variable are a second lock. Update OpenFrame by installing a newer OpenFrame release.

## Controls

How the Steam Frame's controllers play Halo in OpenH1's VR mode, as the game binds them (`port/linux/host/host_vr.c`, `port/linux/src/vr.c`). The Frame's controllers are the halves of an Xbox controller: on the **right** controller are the trigger, grip, bumper, thumbstick (with its click), **A, B, X, Y** and **Menu**; on the **left** are the trigger, grip, bumper, thumbstick (with its click), the **D-pad** and **View**. Settings below say "the weapon's hand" for the right controller (the left, if you play left-handed: see [Accessibility](#accessibility-left-handed-play)).

**Controls on the Steam Frame controllers**

| Physical input | Action |
|---|---|
| Right trigger | Fire |
| Left trigger | Throw a grenade only if `vr.grenade_throw` is `"both"` or `"button"`. By default the left trigger does nothing on foot: you throw by hand (see the left grip). It is the vehicle's second weapon in a Banshee |
| Right grip | Draw a weapon from a holster (the right hand over the shoulder or at the hip, gripping); a softer melee swing when held. With the left grip, recentre |
| Left grip | Closes the left hand. On a long gun's foregrip: aim it with both hands. Held, drawn back, swung and let go: throw a grenade where the hand threw it. At the left hip: take a magazine (reload by hand) |
| Both grips, held | Recentre: the head's place and heading become your eye and facing |
| Left stick | Move |
| Left stick click | Crouch (you also crouch by crouching in the room: `vr.physical_crouch`) |
| Right stick, left and right | Turn, in steps (`vr.turn = "snap"`, the default) or smoothly. This is not "look": the head looks |
| Right stick, one flick down | Eject the magazine (reload by hand, `vr.manual_reload`); with BULLET TIME on it comes a third of a second after the flick, once no second flick has followed |
| Right stick click | Zoom: a magnified window about the reticle (smart-link), or a raised scope's magnification |
| Right stick, flick up | OpenFrame's sprint, if you turned it on (`vr.sprint_mode = "toggle"`): see [Features ported](#features-ported-from-haloce-decompremix-vr) |
| Right stick, two flicks down | OpenFrame's bullet time, if you turned it on (the default binding: two quick flicks, the second within a third of a second of the first's end) |
| Right stick, held down | OpenFrame's third-person view switch (a hold of 0.8 s, on or off; it needs no setting turned on first) |
| A | Jump |
| B | Melee (a hard swing of the right controller melees too: `vr.melee_gesture`) |
| X | Action and reload (hold X to pick up a weapon; X reloads even with reload by hand on) |
| Y | Switch weapon (a gun drawn from a holster switches to it) |
| Left bumper | Flashlight |
| Right bumper | Switch grenade type |
| D-pad (left controller) | The menus' d-pad (move through the menus; also the Xbox game's d-pad) |
| View (left controller) | Back: the multiplayer scoreboard, and back in the menus |
| Menu (right controller) | Start: pause, and the pause menu |
| Both grips on a Ghost's handles, turned | Steer it, its guns aiming where the bars point (`vr.ghost_steering`) |
| Both grips on a Warthog's wheel, turned | Steer it (`vr.warthog_steering`) |
| Hands off the handles or the wheel, either stick pushed sideways | Steer a Ghost or Warthog; in a Scorpion the left stick turns you and the aim. The left stick's forward and back are the throttle |
| Head | Look. Crouch in the room and the game crouches |
| Right controller | Aim: it holds the weapon and the reticle shows where it points |
| The sniper rifle's or pistol's scope raised to an eye | Look through it, magnified (`vr.scope_view = "eye"`) |
| Right hand reaching to a weapon on the ground, gripping | Pick it up, or swap the one in hand for it (`vr.grab_world`) |
| Either hand pushed into a control (a door panel, a lift) | Press it, as X does |

Notes:

- The right stick's up and down are no game control while the controllers aim, so OpenFrame's gestures use them without taking a binding from anything. With `vr.aim = "gamepad"` the controllers play as a gamepad's halves and those gestures are off.
- Every gesture and setting above is OpenH1's, from `build-734`; the full list, with each `vr.*` setting, is in the "VR" section of [port/linux/README.md](port/linux/README.md#vr). The HUD's button prompts name these buttons on the Frame (`Right Bumper`, `A`, ...).
- Controllers that SteamVR maps to the Touch layout also work, with the bumpers on the grips, X and Y on the left and Menu on the left controller; this README is written for the Frame.
- None of OpenH1's VR mode has been confirmed in a headset by OpenFrame yet (see [Status](#status)).

## Features ported from HaloCE-DecompRemix-VR

These come from [moistman42069's HaloCE-DecompRemix-VR-PC-Android](https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android), a Quest-focused fork of OpenCE, re-implemented on OpenH1's VR layer with credit (AGPL-3.0-or-later, as OpenH1 is). The two VR layers share almost no code, so mostly the ideas and the tuning were carried over. **They are built and compile, and every one of them is not yet tested in a headset.** Every one is off, or as OpenH1 had it, until you choose it.

All of them are set in the headset under **Settings > Controls tab > VR SETUP** (the last row; it appears only while the game plays in VR, and also sits at the foot of the player profile menu). Its first page, "VR SETUP", holds the aim and opens one page for each purpose: **DISPLAY...**, **HUD...**, **PLAYER...** (the dominant hand, your height), **MOVEMENT AND COMFORT...** and **HANDS AND WEAPONS...**; the HUD page opens **VISOR LOOK...**, **WRIST HUD...** and **HUD LAYOUT...**, and the hands page opens **FISTS AND GRABBING...**. A setting applies at once. Each is also a `config.toml` key under `[vr]` (and an environment variable `HALO_VR_*`). The full list of tunables, every range and the reasons for them are in [docs/openframe-features.md](docs/openframe-features.md); the short form follows.

Nearly all of these are **offline only**: in a game of this computer alone (the campaign: never online, in split screen or with bots), on foot, with the controllers aiming. Where a feature says so (bullet time, the third person), the HUD tells you when it is not available.

### Comfort vignette (not yet tested in a headset)

- What it does: dims the edges of your view while you move, turn or sprint, not only in vehicles (OpenH1's vignette covers vehicles only).
- Turn it on: VR Setup > MOVEMENT AND COMFORT... > **VIGNETTE** = ALWAYS (`vr.vignette`, default `"vehicle"`; `"off"` too), and **VIGNETTE STRENGTH** (`vr.vignette_strength`, 25 to 150%, default 100%).
- In play: automatic. It dims as the left stick moves you (more while sprinting), as the right stick turns you smoothly, and for a moment at each snap turn.
- Caveats: none known beyond being untested. The older `vr.comfort_vignette = false` still turns it off.

### Sprint, by toggle or arm swing (not yet tested in a headset)

- What it does: runs faster than the game's run (1.5 times by default), on foot, standing, forward and sideways but not backing.
- Turn it on: MOVEMENT AND COMFORT... > **SPRINT** = TOGGLE or ARM SWING (`vr.sprint_mode`, default `"off"`), **SPRINT SPEED** (`vr.sprint_speed`, 1.25 to 2 times, default 1.5).
- In play, TOGGLE: push the right (turning) stick up and let go ("flick it up") to start sprinting; flick again to stop. The HUD says "Sprint" and "Sprint off" each time, and the right controller pulses. A sprint stays on as you stop, look around or crouch (crouching pauses it; standing up you sprint again), and ends by itself only in a vehicle or at death. With bullet time on too, a double flick (two within a third of a second) starts bullet time and takes back the first flick's sprint toggle; sprint toggles the same with bullet time on.
- In play, ARM SWING: swing both arms up and down as you run; the faster the slower arm moves (from 0.6 m/s, full at 1.5 m/s) the faster you go, up to the sprint speed. With the left stick let go, the swing walks you forward along your aim.
- Caveats: offline only.

### Bullet time (not yet tested in a headset)

- What it does: slows the world to 40% (default) while your head and hands keep the display's rate. It eases in and out over about a sixth of a second.
- Turn it on: MOVEMENT AND COMFORT... > **BULLET TIME** = ON (`vr.bullet_time`, default off), **BULLET TIME SPEED** (`vr.bullet_time_scale`, 25 to 80%, default 40%); GESTURES... > **BULLET TIME BINDING** (`vr.bullet_time_binding`: DOUBLE FLICK DOWN, the default, FLICK DOWN, DOUBLE FLICK UP, HOLD UP, or OFF).
- In play: **flick** the right (turning) stick **down twice** (two quick pushes, each let go within 0.3 s, the second begun within a third of a second of the first's end; each within about 34 degrees of straight down, so a diagonal push while looking around does nothing; the stick does not turn you there) to toggle bullet time, on or off; it acts as you let go of the second, and rests for half a second after. **One** flick down on its own is the magazine's eject (reload by hand), a third of a second after it; a **hold** down (0.8 s) is the third person's. A flick **up** is sprint's. Sprint and bullet time are independent: a flick up toggles sprint with bullet time on or off, and they multiply (sprint x2 in a world at 40% runs you at 80% of a normal run: the world is slow and you feel fast). A long hold up does nothing, so a Steam Input script that holds the stick for a second is not pre-empted. With BULLET TIME BINDING = FLICK DOWN (one flick, which takes the stick's eject away: take a new magazine from your left hip and the old one ejects itself), DOUBLE FLICK UP (two quick flicks up; the first toggles sprint and the second takes that back and toggles bullet time) or HOLD UP (0.8 s), see the settings. Each push is logged in stdout.log with how long you held it and what it came to (`vr: stick up: held 0.12 s -> flick`, `vr: stick down: held 0.12 s -> flick (...)`, and `nothing (diagonal; ...)` for a push that was too sideways). Both controllers pulse, and the HUD says "Bullet time" and "Bullet time off". It also ends in a cinematic, at death and where the game takes your control. **Your own body slows with the world**: you run at 40% speed and jumps float, so bullet time feels like slow, floaty movement; that is bullet time, not the sprint.
- Caveats: **offline only** (network games must step alike; the HUD says "Bullet time: offline only"). **The sounds keep their normal pitch and speed**, so a slowed world sounds as fast as ever. Nothing of it is saved in a checkpoint, and a revert turns it off.

### Third person on foot (not yet tested in a headset)

- What it does: the view from behind and above your body, drawn as the game animates it for everyone else, its weapon in its hands. Only the view moves: you still aim with the controller (the reticle where it points) and the HUD is as ever. The view sits 1.7 m behind your head by default, 0.3 m above it and 0.25 m to the weapon hand's side; a wall brings it in.
- Turn it on: hold the right stick down (below), or MOVEMENT AND COMFORT... > **THIRD PERSON** = ON (`vr.third_person`, default off; the hold writes it, so the view stays as you left it), **VIEW DISTANCE** (`vr.third_person_distance`, 1 to 4 m, default 1.7).
- In play: hold the right (turning) stick pushed **down** for 0.8 s to switch between third and first person, whichever you are in and whatever the setting was; a thumb that drifts a little while it holds does not matter, and a push that began on a diagonal and settled down still counts. The HUD says "Third person" and "First person". If that hold clashes with a Steam Input script, GESTURES... > **THIRD PERSON BINDING** = DOUBLE FLICK DOWN (`vr.third_person_binding`, default HOLD) switches with two quick flicks down instead, or OFF.
- Caveats: the push down is also reload-by-hand's magazine eject (`vr.manual_reload`, on by default), so with BULLET TIME off, switching with a magazine that can be ejected **ejects it first** (they share the input); with BULLET TIME on (the default double flick down) the eject waits to see whether the push is a flick on its own, so a hold ejects nothing. Not zoomed, not in a vehicle's seat (`vr.vehicle_view` has its own third person), not in a cinematic; never in multiplayer (the HUD says "Third person: not in multiplayer").

### Parkour: slide, wall jump, wall run (not yet tested in a headset)

- What it does: slide, leap off walls, and run along them. The game's own collision moves you; nothing is sent over the network.
- Turn it on: MOVEMENT AND COMFORT... > **PARKOUR** = ON (`vr.parkour`, default off), **SLIDE MOMENTUM** (`vr.slide_momentum`, 0.5 to 2 times, default 1), **WALL JUMP** (`vr.walljump_scale`, 0.5 to 1.5 times, default 1.5). Sprint must be on for the slide.
- In play, slide: **crouch while sprinting** (the left stick's click, or lower your head with physical crouch) on the ground at a run or faster. **Click once** to slide about a second (the stick's click is a tap; `vr.slide_latch`, default on) and **click again to stop early**. You slide, crouched, at the speed you ran and 15% more, losing speed to friction; it ends as you jump, leave the ground, meet a wall, click again, or after 1.1 seconds (with `vr.slide_latch` off, or a head crouch, as you stand).
- In play, wall jump: in the air beside a wall, **press A (jump) again** to leap off it where your head looks (at least 5 m/s). Once per wall until you land.
- In play, wall run: **jump at a wall with the left stick pushed along it**, within a second of the jump (two seconds after a wall jump, chaining wall to wall), and you run along it for up to 2 seconds. Steer away from the wall to drop off it.
- Caveats: offline only. Grip climbing from the mod is not carried over (this port's grips are already holsters, the foregrip, magazines and vehicle holds).

### Fists: punch, stagger, parry (not yet tested in a headset)

- What it does: punch enemies with your hands, stagger them with a hard punch, and deflect a melee blow.
- Turn it on: VR Setup > HANDS AND WEAPONS... > FISTS AND GRABBING... > **FISTS** = ON (`vr.fists`, default off), **PUNCH SPEED** (`vr.fist_speed`, 1.5 to 4 m/s, default 2.0), **PUNCH DAMAGE** (`vr.fist_damage`, 15 to 100% of the weapon's melee blow, default 33%).
- In play, punch: **swing the empty hand faster than 2 m/s** into an enemy in reach and sight. The free hand always punches; the weapon's hand punches while you hold no weapon. One blow per swing, stronger the faster you swing, less on harder difficulties.
- In play, stagger: a **hard punch** (a swing near twice the punch speed) staggers the enemy for a second: lifted off its feet a moment, slowed, and unable to start a melee attack. Never a Hunter, nor an Elite whose shields hold.
- In play, parry: **swipe a hand across an enemy's blow** (at least 1.8 m/s, half of it across the line from your eyes to the enemy), the hand 10 to 60 cm before your eyes toward it, in the moments before the blow lands. None of the blow is dealt, the enemy is staggered a second, the HUD says "Parry". The weapon's hand parries too; the foregrip hand does not while it holds the foregrip.
- Caveats: offline only; enemies only (no allies). A punch or parry pulses the hand. The mod's lunges, combos, takedowns and ragdoll knockdowns are not carried over.

### Weapon pull (not yet tested in a headset)

- What it does: pulls a weapon lying on the ground to your free hand.
- Turn it on: HANDS AND WEAPONS... > FISTS AND GRABBING... > **WEAPON PULL** = ON (`vr.weapon_pull`, default off), **PULL RANGE** (`vr.weapon_pull_range`, 1 to 5 m, default 3).
- In play: with the free hand, reach out (40 cm from your eyes or more) and **point it at a weapon** within range (it lights up), **grip**, and **flick the hand back toward you or up**. The weapon flies to the hand in about half a second; still gripping as it arrives, you pick it up (as X held does); open, it drops at your feet.
- Caveats: offline only. A weapon like one you already carry is not offered by the game: it drops.

### Prop throw (not yet tested in a headset)

- What it does: lets you hold a loose weapon or piece of equipment in your free hand and throw it.
- Turn it on: HANDS AND WEAPONS... > FISTS AND GRABBING... > **PROP THROW** = ON (`vr.prop_throw`, default off), **THROW STRENGTH** (`vr.prop_throw_strength`, 0.5 to 3 times, default 2).
- In play: **grip a loose item within 15 cm of the free hand** (it lights up) to hold it, turned as the hand points; **let go to throw it** at the hand's speed times the strength, at most 24 m/s.
- Caveats: offline only. Scenery props, lifting vehicles and grabbing bodies are not carried over.

### Hand swap (not yet tested in a headset)

- What it does: passes the weapon to the other hand, for left-handed play on the fly (and back).
- Turn it on: HANDS AND WEAPONS... > FISTS AND GRABBING... > **HAND SWAP** = ON (`vr.hand_swap`, default off).
- In play: with the weapon's grip open, **bring your palms within 10 cm of each other, then squeeze the other hand's grip**. The weapon, trigger, aim, melee swing, holsters, pouches and the wrist HUD's side all move to that hand; the HUD says "Weapon in the left hand" or "Weapon in the right hand". It works from either hand, and back.
- Caveats: offline only. It **saves**: the swap sets `vr.dominant_hand` for now and in `config.toml`, as VR Setup's DOMINANT HAND does, so the menu and the hands never disagree and the next start keeps the hand. The sticks stay where they were (`vr.swap_sticks = "auto"` is first written as what it does then). Both grips do nothing until both are let go.

### Wrist HUD and HUD modes (not yet tested in a headset)

- What it does: moves the shields and health, the ammunition and grenades and the motion tracker from the visor to a small panel on the back of your other hand's wrist, read like a watch. The reticle, messages, waypoints and players' names stay on the visor.
- Turn it on: VR Setup > HUD... > **HUD STYLE** = WRIST (`vr.hud_mode`: `"visor"` default, `"wrist"`, `"hidden"` shows none of the corners). Also, under HUD... > WRIST HUD..., **WRIST HUD SIZE** (`vr.wrist_hud_scale`, 50 to 200%, default 100% = 11 cm across) and **ALONG THE ARM**, **ACROSS THE WRIST**, **OFF THE WRIST** (`vr.wrist_hud_offset_along`, `_across`, `_out`, -10 to +10 cm, default 0).
- In play: **turn the back of the wrist toward your face**; it fades in as you do and out as you turn it away. It stays away while that hand holds a long gun's foregrip.
- Caveats: the wrist HUD needs `vr.hud = "visor"` (the default; a flat HUD keeps the whole HUD on its screen). In a vehicle's seat, with the wrist untracked, and in the menus the corners go back to the visor. The panels are drawn plain, without the visor's glass and glow. The mod's HUD head tap and menu placement are not carried over.

## Accessibility: left-handed play

OpenFrame is playable left-handed, and a few comfort options help with other needs. All are in VR Setup (Settings > Controls tab > VR SETUP).

**Make the left hand the weapon's hand.** VR Setup > PLAYER... > **DOMINANT HAND** = LEFT (`vr.dominant_hand = "left"`, default `"right"`). OpenH1 swaps the two controllers as their input comes in, so everything that said "the weapon's hand" is now the left controller, and "the other hand" the right one. The left trigger fires and the left controller aims, melees, and draws from the holsters on your left; the weapon and both hands are drawn mirrored; the haptics move with the hands. The right hand holds the foregrip, takes magazines from your right hip and throws grenades.

**Swap the sticks.** On the same page, **SWAP STICKS** (`vr.swap_sticks`): AS THE HAND (`"auto"`, the default: swapped when DOMINANT HAND is LEFT), NO (the left stick keeps moving, whatever the hand) or YES (swapped for a right hand too). Swapped, the right stick moves (its click crouches) and the left turns (its click zooms; pushed down it ejects the magazine). Not with `vr.aim = "gamepad"`. The buttons (A, B, X, Y, Menu, D-pad, View, bumpers) stay where they are on the controllers, as the game's prompts name them.

**Hand swap while playing.** With HANDS AND WEAPONS... > FISTS AND GRABBING... > HAND SWAP on, palms together and the free hand's grip passes the weapon across (see above). It now saves `vr.dominant_hand` (and fixes the sticks as they were), so a swap in play is also your new setting.

**How each feature behaves left-handed**

- Weapon, aim, trigger, melee swing, smart-link zoom: the left hand; foregrip, magazines and the hand grenade: the right hand (the grenade's prompt says "Right Hand Throw"). Holsters: over the left shoulder and at the left hip; the magazine pouches at the right hip (and right shoulder).
- Turning stick: whichever stick turns you. Sprint's flick up, bullet time's double flick down and the third person's hold down are all on it, as is the magazine's eject (one flick down) and the zoom (its click).
- Third person: the view sits over the left shoulder. Wrist HUD: on the right wrist, read the same way up.
- Fists: both hands punch as for a right-hander, the free (right) hand always, the left while it holds no weapon; either hand parries, the right not while it holds the foregrip. Weapon pull and prop throw: the free (right) hand.
- Hand swap: from a left-handed start, palms together and the right grip takes the weapon; DOMINANT HAND becomes RIGHT. The sticks do not move.
- Parkour: the game's own crouch, jump and moving stick; no hand. Vehicles: each hand takes the handle on its own side.

**Comfort and seated play**

| Option | Where in VR Setup | What |
|---|---|---|
| Seated mode | PLAYER... > **POSITION** = SEATED (`vr.height`, default `"seated"`; needs a restart) | Your eyes are wherever you recentre (hold both grips); no floor or height to get right. **EYE HEIGHT** (`vr.player_height`) is for standing |
| Physical crouch off | PLAYER... > **PHYSICAL CROUCH** = OFF (`vr.physical_crouch`, default on) | The game crouches only on the left (move) stick's click, so lowering your head does nothing |
| Snap turn | MOVEMENT AND COMFORT... > **TURNING** = SNAP, **SNAP TURN ANGLE** 15 to 90 degrees (`vr.turn`, `vr.snap_turn_angle`, default snap, 30) | Turns in steps, not continuously. SMOOTH and **SMOOTH TURN SPEED** are the alternative |
| Vignette always | MOVEMENT AND COMFORT... > **VIGNETTE** = ALWAYS | Dims the edges whenever you move or turn; **VIGNETTE STRENGTH** adjusts it |
| World scale | PLAYER... > **WORLD SCALE** 0.75 to 1.5 times (`vr.world_scale`, default 1) | More than 1 makes the world look larger and you smaller in it |
| HUD and menu size and distance | HUD... > **HUD DISTANCE**, **HUD SIZE**; DISPLAY... > **MENU DISTANCE**, **MENU WIDTH** | Place and size the HUD and menus where you read them |
| HUD layout: corners in, moved, resized (not yet tested in a headset) | HUD... > **HUD LAYOUT...**: **HUD INSET** (0 to 50%), **ALL LEFT/RIGHT**, **ALL UP/DOWN**, **ALL SIZE**, then **AMMO**, **SHIELDS** and **TRACKER** each **LEFT/RIGHT**, **UP/DOWN** (sliders, -50% to +50% of the HUD's width or height) and **SIZE** (50 to 150%); left and right step a slider by 1% (`vr.hud_inset`, `vr.hud_offset_x`, `_y`, `vr.hud_scale`, `vr.hud_weapon_x`, `_y`, `_scale`, `vr.hud_status_*`, `vr.hud_tracker_*`) | Brings the visor HUD's corners in toward the middle of your view, or moves or resizes each one; the reticle, messages and waypoints stay. Applies when the page's OK is pressed (Cancel forgets the changes) |

## Settings

`~/.local/share/OpenFrame/config.toml` holds the settings. The game writes its own settings into it with a comment on each; a setting commented out follows the default. Every setting also has a `HALO_*` environment variable, which wins over the file. Put `NAME=value` lines in `~/.local/share/OpenFrame/session.env` for one launch, or in front of the command in the shortcut's Steam launch options (an install resets those). The wrapper's own settings:

| Variable | Default | What |
|---|---|---|
| `OPENFRAME_REFRESH_HZ` | unset | 72, 90 or 120. When given, it is asked of SteamVR (per-app `preferredRefreshRate`) and of the game (`HALO_VR_REFRESH_RATE`, which wins over `config.toml`). Unset, the game uses `vr.refresh_rate` from `config.toml` (VR Setup's REFRESH RATE; 90 if absent) and SteamVR is asked for the same rate |
| `OPENFRAME_SYSMEM` | on | `TU_DEBUG=sysmem`: the GPU renders straight to memory instead of in tiles. `0` turns it off |
| `OPENFRAME_PACING` | unset | Reserved for the game's pacing modes; passed through, ignored today |
| `OPENFRAME_MOTION_SMOOTHING` | 2 (off) | SteamVR `motionSmoothingOverride` for the app |
| `OPENFRAME_VRTIMING` | on | `0`: no SteamVR compositor timing log |

At each launch the wrapper also sets SteamVR's per-app `framesToThrottle=0` and `additionalFramesToPredict=0`.

### Steam Frame defaults

OpenFrame keeps OpenH1's VR defaults, which were chosen and measured on a Frame: 1728x1728 eyes, eye-tracked foveation, 90 Hz, snap turn at 30 degrees, a seated position, physical crouch on, and the wrapper's `TU_DEBUG=sysmem`. [docs/steam-frame-defaults.md](docs/steam-frame-defaults.md) lists each setting, why it is where it is, and what to measure before changing it. To try something for one launch, put `NAME=value` lines in `~/.local/share/OpenFrame/session.env` (for example `OPENFRAME_REFRESH_HZ=72` for a heavy level, or `OPENFRAME_SYSMEM=0` to leave Turnip's tiled rendering on); it is read once and renamed `session.env.used`. The refresh rate you pick in VR Setup > DISPLAY... (REFRESH RATE: 72, 90 or 120 Hz, applied from the next start) is what the game and SteamVR use, unless `OPENFRAME_REFRESH_HZ` is set.

## Features

### OpenH1's VR mode (in the game, from `build-734`)

- OpenXR through SteamVR with Vulkan, single-pass multiview stereo, eye-tracked foveated rendering, and a late latch of the head and controller poses.
- Hands and arms that follow the controllers, with the weapon in the right hand; left-handed mode.
- Two-handed aiming of long guns, smart-link and at-the-eye scopes, holsters at the shoulder and hip, reloading by hand, grenades thrown by the left hand, melee by swinging.
- Picking up weapons by hand, pressing controls by hand, flipping vehicles, climbing into vehicles, steering a Ghost by its handlebars and a Warthog by its wheel, driving a Scorpion from its cockpit, flying a Banshee.
- The HUD on the helmet's visor, with depth, wrap, glow and the shields' effects on the glass; snap or smooth turning; physical crouch; a comfort vignette in vehicles; head-in-wall handling; death view; 3D sound from your head (HRTF).
- Everything OpenH1 has outside VR: the Vulkan renderer with SMAA, anisotropic filtering and per-pixel lights, up to 128 players online, mods and Custom Edition maps.

### Not carried over from the mods

The mods' ragdolls (they need a physics library), co-op pose sync, grabbing the world with a free hand (climbing), lunges and combos, takedowns and the menus' placement are not in OpenFrame. [docs/openframe-features.md](docs/openframe-features.md) lists what each ported feature leaves out.

## Files

| What | Where |
|---|---|
| Program | `~/Games/OpenFrame/app` (`halo`, `libSDL3.so.0`, `openframe.sh`, `tools/`) |
| Settings, maps, saves, internet play's `brokers.txt` | `~/.local/share/OpenFrame` (`config.toml`, `maps/`, `saves/`, `brokers.txt`) |
| Campaign save (what CONTINUE loads) | `saves/u/UDATA/<profile id>/savegame.bin`: ONE slot per profile. **Starting a NEW GAME (or any level) overwrites it**; there is no second slot and no backup. The profile (name, levels finished) is `blam.sav` beside it; the running mission's checkpoint is `saves/z/savegame.bin`, copied into the slot as the level is left. Copy `saves/` aside before starting something new if you want to come back |
| Logs | `~/.local/share/OpenFrame/sessions/latest/` (`launch.log`, `stdout.log`, `debug.txt`, `vrtiming.log`, `perfcriteria.txt`) |
| One-launch test settings | `~/.local/share/OpenFrame/session.env` (`NAME=value` lines; `session.env.next` for the launch after) |

## Internet play

Internet play is OpenH1's and is on by default. Invites and the server browser find each other through public MQTT brokers listed in `brokers.txt` (`network.brokers_file`) and OpenH1's master servers, so using it contacts them. `network.online = false` in `config.toml` turns internet play off and leaves system link on the local network, which needs no brokers.

## Troubleshooting

- **`install.sh` says Steam's debugging port is not reachable.** Start Steam and run it again, or set `OPENFRAME_SKIP_STEAM=1`.
- **The game asks for the disc image at start.** It did not find `maps/` in `~/.local/share/OpenFrame`. Run `install.sh --maps` with your maps folder.
- **It plays flat in a window.** SteamVR was not running, or the headset was not connected, when the game started. Start SteamVR first. `HALO_VR=0` forces flat play.
- **The menu is off to the side, or the view is not where the player is (outside the cryo tube).** Hold both grips for a moment: that recentres (the menu goes in front of you, the eye into the player). SteamVR's own recentre works too. The game also recentres by itself on the first frame of every level start (new game, CONTINUE, mission restart, checkpoint), after a cutscene, and opens each menu in front of where you look (`vr.auto_recentre`, on by default); it does none of its own within 5 s of yours. Sit as you mean to play when the loading picture fades. While the game holds you still (the cryo tube, scripted seats) and for 2 s after a level start, the view stays within 15 cm of the player's eye in any direction (`vr.held_lean`). `stdout.log`'s `vr: head offset:` lines say how far off the head was each time.
- **HUD corners cut off** (the ammo top left or the motion tracker bottom left past the edge of your view, for example on a Frame with a glasses spacer). VR Setup > HUD... > HUD LAYOUT... > **HUD INSET** 15 to 25% (`vr.hud_inset = 0.2`), then **AMMO**, **SHIELDS** or **TRACKER** LEFT/RIGHT and UP/DOWN (a few percent) for one corner alone. [docs/steam-frame-defaults.md](docs/steam-frame-defaults.md) has a preset.
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
