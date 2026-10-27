# OpenFrame features

Features OpenFrame (the Steam Frame fork of OpenH1) adds to OpenH1's VR mode,
where each came from, and how to turn it on. Each is set in `config.toml`
(the `vr.*` keys below) or in VR Setup: MOVEMENT AND COMFORT..., HANDS AND
WEAPONS... (and its FISTS AND GRABBING...), HUD... (WRIST HUD..., HUD
LAYOUT...) and PLAYER....
Every one is off, or as OpenH1 had it, until you choose it.

The VR Setup menu (Settings > Controls tab > VR SETUP, also at the foot of the
player profile menu) is a tree of pages of one purpose each:

- VR SETUP: AIM, GAMEPAD VIEW, then DISPLAY..., HUD..., PLAYER..., MOVEMENT
  AND COMFORT..., HANDS AND WEAPONS...
- DISPLAY: refresh rate, resolution, foveation and its size, late latch,
  menu distance and width.
- HUD: HUD STYLE (visor, wrist, hidden), HUD SHAPE, size, distance, depth,
  wrap, lag, then VISOR LOOK... (glow, glass, reacts, helmet rim, visor
  shade, shield effects and energy), WRIST HUD... (size, three offsets) and
  HUD LAYOUT... (inset, all, ammo, shields, tracker).
- PLAYER: dominant hand (left-handed players: LEFT), swap sticks, position,
  eye height, physical crouch, world scale, auto recentre, held lean, death
  view.
- MOVEMENT AND COMFORT: turning, snap angle, smooth speed, vignette and its
  strength, sprint and its speed, vehicle view, third person and its
  distance, parkour (slide, wall jump), bullet time and its speed.
- HANDS AND WEAPONS: hands, two-handed aim, aim assist, scopes, holsters,
  reload by hand, swing to melee, grenade throw, haptics, then FISTS AND
  GRABBING... (fists, punch speed and damage, weapon pull and range, hand
  swap, prop throw and strength, grab world).

**Untested in the headset** marks a feature built and compiled but not yet
played on a Steam Frame.

## Ported from HaloCE-DecompRemix-VR

Ported from moistman42069's HaloCE-DecompRemix-VR
(<https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android>,
AGPL-3.0-or-later, as OpenH1 is), re-implemented on OpenH1's VR layer
(`port/linux/src/vr.c`, `port/linux/game/vr_comfort.c`): the two VR layers
share almost no code, so mostly the ideas and the tuning were carried over;
the few functions whose lines came across say so at their top.

### Fists: punch and parry (untested in the headset)

`port/linux/game/vr_fists.c`, once a tick (`source/game/game.c`, a marked
port change), the hands from `port/linux/src/vr.c`.

- Punch: swing the hand without the weapon (or the weapon's hand while you
  hold no weapon) past `vr.fist_speed` into an enemy. The fist's path over
  the tick, carried on a little, must pass within reach of the enemy's body
  (its pelvis to its head, as the game's aim assist has it, plus 16 cm), in
  sight. The hand's own speed counts (the room's: walking, turning or a
  turn of the body is no punch). One blow a swing: the game's melee damage
  of the weapon in hand (an empty hand's, the game's unarmed blow), half to
  one and a half times as strong as the swing is fast, times
  `vr.fist_damage`, and less on the harder difficulties (moistman42069's
  0.8, 1, 1.3, 1.6 by difficulty); the enemy is shoved along the punch by
  its mobility. With `vr.fists` and no weapon in hand, the weapon hand's
  swing is a fist's, not `vr.melee_gesture`'s B.
- Hard punch: a blow of 1.2 or more (a swing near twice `vr.fist_speed`)
  staggers the enemy for a second: lifted off its feet a moment (the game's
  airborne stumble), slowed (the game's own stun), and unable to begin a
  melee attack (`source/units/units.c`, a marked port change). Never a
  Hunter, nor an Elite whose shields hold.
- Parry: swipe a hand across an enemy's blow (at least 1.8 m/s, half of it
  across the line from your eyes to the enemy), the hand 10 to 60 cm before
  your eyes toward it, in the ticks before the blow lands (eight on Easy to
  four on Legendary). None of the blow is dealt
  (`source/objects/damage.c`, a marked port change), the enemy is staggered
  a second, the HUD says "Parry". The weapon's hand parries too; the other
  not while on the foregrip.
- Keys: `vr.fists` (default `false`), `vr.fist_speed` (metres a second, 1
  to 4, default `2.0`, moistman42069's MELEE SPEED), `vr.fist_damage` (of
  the weapon's melee blow, 0.1 to 1, default `0.33`: moistman42069's ENEMY
  HEALTH of 3).
- Input: gestures only (a swing, a swipe); nothing bound. A hand's punch
  and a parry pulse it.
- Where: the first player, on foot, alive, the controllers aiming, in a game
  of this computer alone (never online, split screen or with bots); enemies
  only (no allies).
- Not carried over: moistman42069's lunges, combos, counters and their
  warnings, takedowns, blocks, combat moves, hit highlights, knockdowns
  (ragdolls on Jolt) and body contact on the bones: the body here is the
  aim assist's pill, and a strong hit's stagger the game's own stun and
  stumble.

### Weapon pull, prop throw and hand swap (untested in the headset)

`port/linux/game/vr_grab.c` (OpenH1's grab, extended), its hands from
`vr_fists.c`'s tick; the grips' sharing in `port/linux/src/vr.c`. All for
the hand without the weapon.

- Weapon pull: reach the hand out (40 cm from your eyes or more) and point
  it at a weapon lying within `vr.weapon_pull_range` (within 20 degrees of
  where it points, in sight; it lights up), grip (it lights up fully), and flick the hand back toward
  you or up: the weapon flies to the hand in an arc in about half a second.
  Still gripping as it arrives, it is picked up as the right hand's grab
  picks one up (the game's pick-up, as X held: in hand, or swapped for the
  one held); open, it drops at your feet. One like a weapon you carry is
  not offered by the game: it drops.
- Prop throw: grip at a loose weapon or piece of equipment (within 15 cm
  of the hand; it lights up) to hold it, turned as the hand points; let go
  to throw it at the hand's speed (its fastest of the last few ticks) times
  `vr.prop_throw_strength`, at most 24 m/s, as the game drops a weapon (your
  body passed through, kept inside the level). The weapon hand's grab takes
  the one held, as ever.
- Hand swap: palms together (within 10 cm), the weapon's grip open, and the
  other hand's grip takes the weapon: everything of the weapon's hand
  (trigger, aim, melee swing, holsters, pouches, the wrist HUD's side)
  moves to that hand: the swap sets `vr.dominant_hand` to that hand, for
  now and in `config.toml`, as VR Setup's DOMINANT HAND does, so the menu
  and the hands never disagree; it stays so (next start too) until you pass
  it back or choose the hand in VR Setup. It works from either hand, and
  back. The sticks stay as they were: with `vr.swap_sticks = "auto"` (as the
  hand) the swap first writes it as what it does then (`"no"` or `"yes"`).
  Both grips are nothing until both are let go. The HUD says which hand the
  weapon is in.
- The grip goes to the pull or the prop only while the hand points at a
  weapon to pull or is at a thing to hold; a pouch's magazine comes first,
  and the grenade (`vr.grenade_throw`) and the foregrip otherwise, as ever.
- Keys: `vr.weapon_pull` (default `false`), `vr.weapon_pull_range` (metres,
  1 to 5, default `3`, moistman42069's), `vr.prop_throw` (default `false`),
  `vr.prop_throw_strength` (0.5 to 3, default `2`, moistman42069's OBJECT
  THROW), `vr.hand_swap` (default `false`).
- Input: the grip of the hand without the weapon; no new binding.
- Where: on foot, the controllers aiming, in a game of this computer alone
  (never online, split screen or with bots), the hand swap's gesture too.
- Not carried over: moistman42069's scenery props (a collision solver of
  their own), lifting vehicles and grabbing bodies, PULL STYLE GRIP and
  PULL GLOW's outline (the game's own highlight lights it instead).

### Third person on foot (untested in the headset)

On foot, the view from behind and above you, your body drawn as the game
animates it for everyone else, its weapon in its hands. Only the view
moves: you still aim with the controller from where your body's hand is
(the reticle where it points), the HUD is as ever, and you hear from your
body. The view sits back along your body's heading (the head's, which it
follows past a turn and slowly, so aiming and glancing about swing
nothing), 0.3 m above the head and 0.25 m to the weapon hand's side (its
right, or its left with `vr.dominant_hand = "left"`); your head looks and
leans about from there. A wall between brings it in at once, and it eases
back out. The first-person weapon and hands (and what they hold: a
grenade, a magazine, the holstered weapon) are not drawn. It is OpenH1's
own VR view (`port/linux/game/vr_render.c`) with the eye moved, as its
vehicle third person rides the game's chase camera; the body is the
game's (`source/render/render_objects.c` and
`source/interface/first_person_weapons.c`, marked port changes).

- Keys: `vr.third_person` (default `false`), `vr.third_person_distance`
  (metres behind the head, 1 to 4, default `1.7`).
- Input: hold the turning stick (the right, or the left with
  `vr.swap_sticks`) pushed down for three quarters of a second to switch
  between it and the first person (it starts in the third person as you
  turn the setting on). The stick's down is no game control while the
  controllers aim; with `vr.manual_reload` (on by default) the push still
  ejects the magazine first, as ever, so switching with a magazine that can
  be ejected ejects it. The HUD says "Third person" and "First person".
- Where: on foot, alive, not zoomed (the scopes and the smart-link are the
  first person's), not in a vehicle's seat (`vr.vehicle_view` has its own
  third person), a cinematic or as you die; never in a multiplayer game
  (it would see round corners; the HUD says "Third person: not in
  multiplayer").
- After moistman42069's VR third person (`vr_third_person.c`,
  `third_person.c`), which also has the camera behind along the heading
  rather than the aim; its switching of the hands' features off (the head
  aiming) is not carried over: the controller aims as ever.

### Parkour: slide, wall jump, wall run (untested in the headset)

`port/linux/game/vr_parkour.c`, from `source/units/bipeds.c` (marked port
changes) as each tick's movement is made ready, and in each sub-step of
`game.sim_rate`. The game's own collision moves the body; nothing is sent.

- Slide: crouch (the left stick's click, or your head lowered with
  `vr.physical_crouch`) while sprinting (`vr.sprint_mode`; the sprint now,
  or a third of a second before), on the ground at a run or faster, and you
  slide on, crouched (the game's lower height), at the speed you ran and 15%
  more, losing speed to friction, faster down a slope, steered a little by
  the stick (pulled back, it brakes). It ends as you let the crouch go, jump
  (the speed carried into the jump; Jump again in the air beside a wall is
  then a wall jump), leave the ground, meet a wall, slow to half a run, or
  after 1.1 seconds. A toggled sprint is only paused by the crouch, so as
  you stand it is back by itself and the next slide needs no new flick. The
  crouch is read from the click or the head (not only the game's crouch,
  which the game drops at a full stick push), so a slide holds for as long
  as you hold the crouch. Arm swing: the swing just before the duck counts.
- stdout.log: `vr: slide start (head crouch|crouch button, sprint on|by
  arms) at N m/s, the run M`, `vr: no slide (why; speed, run)` when a crouch
  began and did not slide, `vr: slide end (why) after N s, sprint ...`,
  `vr: wall jump [from a run] at N m/s`, `vr: wall run start [(chained)]`
  and `vr: wall run end (why)`.
- Wall jump: in the air beside a wall, press Jump again to leap off it
  where your head looks (turned off the wall at least a third of the way),
  at least 5 m/s and as fast as you were going (up to half as fast again),
  as high as a jump (more looking up), both times `vr.walljump_scale`; once
  a wall until you land. The leap is held against the air control for
  0.4 s so that the stick does not turn it back.
- Wall run: jump at a wall with the stick pushed (past 60%) along it, within
  a second of the jump (or two seconds of a wall jump, chaining wall to
  wall), and you run along it at 1.5 times the speed you came with (4 to
  9 m/s), level for 1.2 s and then easing into a fall, 2 s at most. Steer
  away from the wall to drop off it; a wall jump leaps off it.
- Keys: `vr.parkour` (default `false`), `vr.slide_momentum` (0.5 to 2,
  default `1`: a slide starts `0.15 x` this faster than you ran and lasts
  at most `0.55 + 0.55 x` this seconds), `vr.walljump_scale` (0.5 to 1.5,
  default `1.5`, moistman42069's). Wall run's speed and time are fixed at
  its defaults.
- Input: the game's own (crouch, Jump, the stick); nothing taken.
- Where: the first player, on foot, in a game of this computer alone, as
  sprint (never online, split screen or with bots); not in a cinematic or
  where the game takes your control.
- Not carried over: the slide's blows to the AI (moistman42069's knockdowns
  and damage, which need its fist combat), and grip climbing (a free hand's
  grip on any wall pulls you up it): this port's grips already hold
  holsters, the foregrip, magazines, grenades and a vehicle's holds, and
  both together recentre.

### Bullet time (untested in the headset)

Slows the world while your head and hands keep the display's rate. The
game's clock runs slower (`source/main/main.c` scales the time it gives
`game_time_update`), so every tick and `game.sim_rate` sub-step is the same
step as ever, only fewer of them each second, and the frames between ticks
are drawn as far into the next tick as the slowed clock has run. Nothing of
it is saved: a checkpoint or a saved game knows nothing of it, and a revert
turns it off. It eases in and out over about a sixth of a second.

- Keys: `vr.bullet_time` (default `false`), `vr.bullet_time_scale` (the
  world's speed, 0.2 to 0.8, default `0.4`).
- Input: hold the turning stick (the right, or the left with
  `vr.swap_sticks`) pushed up for 0.8 s; a flick up (or another hold) ends
  it. The player's own body is of the world and slows with it (the run at
  40%, jumps floating): in headset trial 3 a push held a little long (the
  hold was half a second then) took bullet time, which was felt as a slow,
  floaty sprint that no flick turned off. The stick's up is no game control
  while the controllers aim (OpenH1 sends the right stick to the game only
  in the menus, cinematics and with `vr.aim = "gamepad"`; its down ejects a
  magazine with `vr.manual_reload`), so no binding is taken from anything. On foot, with the controllers aiming.
- Where: a game of this computer alone (the campaign). Never online, in split
  screen or with bots (network games, which every machine must step alike);
  the HUD says "Bullet time: offline only" there. It ends in a cinematic, at
  death and where the game takes your control.
- The HUD says "Bullet time" and "Bullet time off"; both controllers pulse.
- Limitation: the sounds keep their pitch and speed (the mixer plays each
  voice at its own rate), so a slowed world sounds as fast as ever.

### Sprint (untested in the headset)

Runs faster than the game's run, on foot and standing (not crouched), forward
and sideways (not backing), accelerating faster with it. The view's field is
unchanged. `source/units/bipeds.c` multiplies the tick's speeds for the first
local player (`vr_sprint_speed_scale`).

- Keys: `vr.sprint_mode` (`"off"`, the default; `"toggle"`;
  `"arm_swing"`), `vr.sprint_speed` (times the run, 1.1 to 2, default `1.5`).
- `"toggle"`: flick the turning stick up (as bullet time's input, but let go
  within 0.8 s; with bullet time off it toggles at once as you push) to
  sprint, and again to stop. While bullet time is on, a flick ends bullet time
  instead. The sprint stays on as you stop or crouch (a crouch pauses it:
  `bipeds.c` sprints standing), and ends by itself only off foot (a vehicle,
  death; half a second of it, the menus aside). The HUD says "Sprint" and
  "Sprint off" at every change; the right controller pulses. The gesture is
  `port/linux/src/vr_stick_gesture.h` (`stick_gesture_update`).
- stdout.log: `vr: sprint on/off (why)` at each change, and from `bipeds.c`
  as a sprint starts and ends `vr: sprint x1.50: forward speed 2.250 ->
  3.375 world units/s (6.86 -> 10.29 m/s), acceleration ... (full speed in
  ... s)`: the run's speed and acceleration before and after the scale (the
  time to full speed stays the same).
- `"arm_swing"`: swing both arms up and down as you run; the faster the
  slower arm moves (from 0.6 m/s, full at 1.5 m/s) the faster you go, up to
  `vr.sprint_speed`. With the left stick let go the swing walks you forward
  along your aim.
- Where: a game of this computer alone, as bullet time.

### Comfort vignette for moving and turning (untested in the headset)

OpenH1's comfort vignette dimmed the edges of the view only in vehicles
(handlebars' and wheels' turns, speeding up and braking, a seat's ride).
`"always"` also dims them as the left stick moves you on foot (more while
sprinting), as the turning stick turns you smoothly, and for a moment at each
snap turn.

- Keys: `vr.vignette` (`"vehicle"`, the default and OpenH1's behaviour;
  `"always"`; `"off"`), `vr.vignette_strength` (0.25 to 1.5, default `1.0`:
  how much of the edges, and how dark). `vr.comfort_vignette = false`, the
  older switch, still turns it off.
- Input: none.

### Wrist HUD and HUD modes (untested in the headset)

`vr.hud_mode` says where the HUD's corners are: the shields and health, the
weapon's ammunition and grenades, and the motion tracker. `"wrist"` puts them
on a small panel on the back of your other hand's wrist (the left, or the
right with `vr.dominant_hand = "left"`), read as a watch is: it fades in as
you turn the back of the wrist toward your face and out as you turn it away,
and it stays away while that hand holds the long gun's foregrip. `"hidden"`
shows no corners at all. The rest of the HUD (the reticle, the messages, the
waypoints and the players' names) stays on the visor in every mode.

It uses OpenH1's own HUD panels (`port/linux/src/vr_visor.c`, the per-group
images that `vr.hud_depth` draws at their own depths): with the wrist HUD the
three corner panels are composed as their own OpenXR quad layers on the
wrist, in the play space, over the HUD's layer, rather than on the visor.
Nothing is drawn into the eyes' images. Each corner's panel shows only the
part of the 640x480 HUD its elements fill, laid out as moistman42069's panel
is: the shields across the top, the tracker under them on the left and the
ammunition beside it. On the wrist they are drawn plain, without the visor's
glass, glow and reaction to the world (`port/linux/vulkan/host_vulkan_vr.c`).

- Keys: `vr.hud_mode` (`"visor"`, the default and OpenH1's behaviour;
  `"wrist"`; `"hidden"`), `vr.wrist_hud_scale` (0.5 to 2, default `1`: 11 cm
  across), `vr.wrist_hud_offset_along`, `vr.wrist_hud_offset_across`,
  `vr.wrist_hud_offset_out` (metres from its place, -0.2 to 0.2, default
  `0`: along the forearm toward the elbow, across toward the thumb, and up
  off the wrist). VR Setup > HUD... (**HUD STYLE**) and WRIST HUD...
- Where: on foot, with `vr.hud = "visor"` (the default; `"flat"` keeps the
  whole HUD on its flat screen). In a vehicle's seat, with the wrist
  untracked, and in the menus the corners go back to the visor. It works at
  `vr.hud_depth` 0 as well (the corners are then on the visor at the HUD's
  distance when not on the wrist). The HUD's layer is placed in the play
  space while the wrist HUD is on (it was locked to the view space at
  `vr.hud_lag` 0).
- Its place and its test of facing the head come from moistman42069's
  (`vr_frame.c`, `place_wrist`: 10 cm behind the grip along the arm, 4.5 cm
  out of the back of the wrist, shown while the way out of the wrist is
  within about 63 degrees of the way to the head, a cosine of 0.45). The fade
  (a 0.06 s time constant) is OpenFrame's; theirs shows and hides at once.
  Their crops of the HUD are kept, a little wider (each group's image holds
  its own elements alone). Their dark backing behind the panel is not
  carried over.
- Not carried over: moistman42069's HUD head tap (the weapon hand held by
  the temple hides the HUD) and the menus' placement (`vr.menu_anchor`
  head, world, follow). A lag for the HUD following the head needs nothing
  new: OpenH1 has `vr.hud_lag`.

## Recentring and Pillar of Autumn's tutorial (after the first headset trial)

These fix what the first trial found (the player's eye outside the cryo
tube, the tutorial not going on as the head looked about, the menu off to
the side). Unlike the features above, `vr.auto_recentre` is on by default.

- SteamVR's own recentre (its dashboard's, or the system button's) moves
  the play space; the game now recentres with it (the runtime's
  `XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING`). Before, the game
  kept its recentred origin from before, so the eye was as far from the
  player's as the head had been from where it was then, and the menus were
  as far round.
- `vr.auto_recentre` (default `true`): the game recentres as play comes
  back after a load, a cinematic or the scripts holding the facing (half a
  second or more), and places each menu in front of where you look as it
  opens (VR Setup > PLAYER... > AUTO RECENTRE). `false`: only both grips held (or a gamepad's Back), SteamVR's
  recentre and the headset going on recentre, and the menus are where the
  last recentre put them, as OpenH1 had it.
- `vr.held_lean` (default `0.15` metres): while the scripts hold the
  player still (the cryo tube), the head moves the view at most this far
  across the floor from the player's eye. `10` lets it go as far as the
  head does (VR Setup > PLAYER... > HELD LEAN).
- The scripts' gaze tests (`unit_can_see_object`, `objects_can_see_flag`:
  the tutorial's lights) pass when the head looks at the thing, as well as
  when the player's facing (the controller, with `vr.aim = "controller"`)
  does; and the look tests (`player_action_test_look_relative_*`: look up,
  down, left, right) count the head's turn and the VR aim's, about 5
  degrees each time, as well as the stick's. HaloCE-DecompRemix-VR does the
  same with its head (`vr_head_look_actions`, `vr_script_can_see_point`);
  this is written for OpenH1's VR mode, not ported.

## HUD placement (after the second headset trial; untested in the headset)

With a glasses spacer the Frame's comfortable view is narrower, and the
visor HUD's corners (the ammunition and grenades at the top left, the motion
tracker at the bottom left) were partly past its edge. These keys bring the
corners in, move or resize each one (VR Setup > HUD... > **HUD
LAYOUT...**). Every one defaults to 0
(sizes to 1): the corners where OpenH1 puts them.

| Key (`HALO_*` variable) | VR Setup row | Range | What |
| --- | --- | --- | --- |
| `vr.hud_inset` (`HALO_VR_HUD_INSET`) | HUD INSET | 0 to 0.5 | Every corner turned in toward the middle of your view by that fraction of its angle from it |
| `vr.hud_offset_x`, `_y` (`HALO_VR_HUD_OFFSET_X`, `_Y`) | ALL LEFT/RIGHT, ALL UP/DOWN | -0.3 to 0.3 | The three corners moved together, right and up (less than 0: left, down), in fractions of the HUD's width and height |
| `vr.hud_scale` (`HALO_VR_HUD_SCALE`) | ALL SIZE | 0.5 to 1.5 | The corners' size, each about its outer corner |
| `vr.hud_weapon_x`, `_y`, `_scale` (`HALO_VR_HUD_WEAPON_X`, ...) | AMMO LEFT/RIGHT, UP/DOWN, SIZE | -0.3 to 0.3; 0.5 to 1.5 | The ammunition and grenades (top left) alone |
| `vr.hud_status_x`, `_y`, `_scale` (`HALO_VR_HUD_STATUS_X`, ...) | SHIELDS LEFT/RIGHT, UP/DOWN, SIZE | the same | The shields and health (top right) alone |
| `vr.hud_tracker_x`, `_y`, `_scale` (`HALO_VR_HUD_TRACKER_X`, ...) | TRACKER LEFT/RIGHT, UP/DOWN, SIZE | the same | The motion tracker (bottom left) alone |

- Order: each corner's own move first, then the inset (which scales that
  move too), then the move of all three; sizes multiply (`vr.hud_scale`
  times the corner's own). The inset alone, or each corner's own move
  alone, is enough; they add up if both are set.
- Moves are angles: 0.1 of the width is 6 degrees at `vr.hud_size` 60, the
  same at any `vr.hud_distance`. Sizes grow or shrink each corner about its
  outer corner (where the game anchors its elements), so they stay put.
- The reticle and the messages (the HUD's middle group, on the HUD's own
  layer) and the waypoints and players' names stay where they are; the
  messages cannot be moved alone (they share the reticle's image). So do
  the visor's curve, glass and glow, each panel's depth (`vr.hud_depth_*`)
  and wrap (`vr.hud_wrap`). With `vr.hud_mode = "wrist"` the corners on the
  wrist are not moved (they are where they go back to the visor, as in a
  vehicle's seat). With `vr.hud = "flat"` none of this applies.
- Live: every value is read again as it changes (no restart), and the
  corners are at their new place the moment the menu closes. While a menu
  is open OpenH1 shows no corner panels (`vr_visor_panels` gives none with
  the menus up), so they cannot be watched moving from the menu itself.
- How: `port/linux/src/vr_visor.c`, `panel_placement`: each corner's quad
  layer is turned about the middle of the head (across about the HUD's up,
  then up about its own across), so it still faces the eyes, upright, its
  image as ever, and scaled about its outer corner. At `vr.hud_depth` 0 the
  corners are drawn on panels of their own (at the HUD's distance) while
  they are moved, as the wrist HUD has them.
- `docs/steam-frame-defaults.md` has suggested values for a Frame with a
  spacer (`vr.hud_inset` 0.15 to 0.25, or the ammo's and tracker's own moves).

## Left-handed

Set VR Setup > PLAYER... > DOMINANT HAND to LEFT
(`vr.dominant_hand = "left"`). OpenH1 swaps the two controllers as their
input comes in (`port/linux/src/vr.c`, `hands_assign_views` and
`hands_assign_input`): from there on everything that says "the weapon's
hand" is the left controller and "the other hand" the right one, so every
feature below follows without a check of its own. `vr.swap_sticks`
(SWAP STICKS, default AS THE HAND) then swaps the sticks too: the right
stick moves, the left turns; NO keeps them where they are. Not with
`vr.aim = "gamepad"`. The buttons stay where they are on the controllers
(A, B, X, Y and Menu on the right, the d-pad and View on the left), as the
game's prompts name them.

How each feature behaves, left-handed:

- Weapon, aim, trigger, melee swing (`vr.melee_gesture`), scopes'
  smart-link zoom: the left hand; the weapon and the hands drawn mirrored.
- Foregrip (`vr.two_handed`), magazines (`vr.manual_reload`) and the hand
  grenade (`vr.grenade_throw`): the right hand. The grenade's prompt says
  "Right Hand Throw".
- Holsters (`vr.holsters`): over the left shoulder and at the left hip; the
  magazines' pouches at the right hip (and the right shoulder).
- Turning stick: whichever stick turns you (the left with SWAP STICKS as
  the hand or YES; the right with NO). Sprint's flick up, bullet time's
  hold up and the third person's hold down are all on it, as is the
  magazine's eject (pushed down) and the smart-link's zoom (its click).
- Third person on foot: the view sits over the left shoulder instead of
  the right.
- Wrist HUD (`vr.hud_mode = "wrist"`): on the right wrist, read the same
  way up (its top toward the little finger, the shields across the top).
- Fists (`vr.fists`): both hands punch, as for a right-hander: the free
  (right) hand always, the left (weapon) hand while it holds no weapon;
  either hand parries, the right not while it holds the foregrip.
- Weapon pull and prop throw: the free (right) hand.
- Hand swap (`vr.hand_swap`): from a left-handed start, palms together and
  the right grip takes the weapon; DOMINANT HAND becomes RIGHT. Again with
  the left grip, and it is LEFT again. The sticks do not move.
- Parkour: the game's own crouch, jump and moving stick; no hand.
- Vehicles: each hand takes the handle on its own side, as ever.
- HUD messages: "Weapon in the left hand" / "Weapon in the right hand"
  after a hand swap.
