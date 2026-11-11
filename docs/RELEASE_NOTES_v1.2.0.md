# OpenFrame 1.2.0

Halo: Combat Evolved in VR for the Steam Frame, running natively. OpenFrame is my Steam Frame version of [OpenH1](https://github.com/OpenH1/OpenH1), the recompiled Halo built for Arm64 Linux with an OpenXR VR mode. This release moves it to OpenH1 `build-2188`, and it runs far smoother in the headset. I also added a body shadow, a wait for SteamVR at start, punches online, smooth turning in vehicles and shot deflection. You need the maps from your own Xbox disc; no game data is included.

### Performance

I played the same Pillar of Autumn save on 1.1.0 and on this build. On 1.1.0, four out of five seconds of play had a missed frame. Now 99 seconds out of 100 run at a steady 90 Hz. Once the game has your head's position, it hands SteamVR a finished frame in about 1.4 ms. 1.1.0 took about 4.8 ms. That leaves the chip plenty of room, so the world stays locked to your head.

Most of this is OpenH1's renderer work. The game does much less each frame now. It keeps texture bindings from one frame to the next, uploads only the shader values a draw reads, draws object shadows in a few passes, and sends only the dynamic lights that changed.

### Added

- **Body shadow.** Your body casts a shadow, its head and arms following the headset and controllers. This is after moistman42069's HaloCE-DecompRemix-VR. It's on by default (`vr.body_shadow`).
- **Waits for SteamVR.** If SteamVR is still starting or the headset is asleep, the game waits up to a minute for it. It used to open as a flat window you couldn't see (`vr.runtime_wait`).
- **Punches online.** With FISTS on, a punch is a melee online, in split screen and with bots. It used to hit only in a game on your own.
- **SMOOTH TURN IN SEATS**, on the GESTURES page. With snap turning on foot, the stick turns smoothly in every vehicle seat. It's on by default.
- **Shot deflection** (`vr.deflect`, off by default). A shot that hits your gun glances off it, and a hard swing of the gun or a fist bats it away. Bullets only glance off the gun. Rockets and fuel rods need a hard swing.
- **VERSION DIFFERENCE.** Joining a game on another version now shows a message in the menus. It used to be a desktop box the headset never showed.
- Crash reports name OpenFrame and open on OpenFrame's issues.

### Changed

- Your hand keeps the angle it grabbed at on the Warthog's wheel, the Ghost's handlebars and turrets. It used to snap to one fixed pose.
- The grenade in your hand is the size it's thrown at. It used to grow as it left your hand (`vr.grenade_scale`).
- A patch of ground stops taking new blood once six fresh splats lie on it. Long fights cost less to draw (`game.blood_layers`).

### Fixed

- A slide still going when you quit a campaign kept you crouched in the next game.

### What came with OpenH1 build-2188

All of this is OpenH1's work. I merged it and set it up for the Frame.

- Sharper text everywhere.
- Pings: mark what you aim at for your allies.
- New first-person legs.
- Smoother co-op lifts and cutscene cameras.
- Bots aim and strike more like people.
- A map that fails to load sends you back to the main menu instead of stopping the game.
- Online games send less data. OpenCE players are kept out of OpenH1 games by default.
- Walls no longer cut through a camouflaged weapon.
- The master volume goes up to 20.

### Known issues

- The first half minute after loading a save can still stutter while the game prepares its shaders.
