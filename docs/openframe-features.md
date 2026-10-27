# OpenFrame features

Features OpenFrame (the Steam Frame fork of OpenH1) adds to OpenH1's VR mode,
where each came from, and how to turn it on. Each is set in `config.toml`
(the `vr.*` keys below) or in VR Setup > VR Display > COMFORT, SPRINT...
Every one is off, or as OpenH1 had it, until you choose it.

**Untested in the headset** marks a feature built and compiled but not yet
played on a Steam Frame.

## Ported from HaloCE-DecompRemix-VR

Ported from moistman42069's HaloCE-DecompRemix-VR
(<https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android>,
AGPL-3.0-or-later, as OpenH1 is), re-implemented on OpenH1's VR layer
(`port/linux/src/vr.c`, `port/linux/game/vr_comfort.c`): the two VR layers
share almost no code, so mostly the ideas and the tuning were carried over;
the few functions whose lines came across say so at their top.

### Third person on foot (untested in the headset)

On foot, the view from behind and above you, your body drawn as the game
animates it for everyone else, its weapon in its hands. Only the view
moves: you still aim with the controller from where your body's hand is
(the reticle where it points), the HUD is as ever, and you hear from your
body. The view sits back along your body's heading (the head's, which it
follows past a turn and slowly, so aiming and glancing about swing
nothing), 0.3 m above the head and 0.25 m to its right; your head looks and
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
  `vr.swap_sticks`) pushed up for half a second; again to end it. The stick's
  up is no game control while the controllers aim (OpenH1 sends the right
  stick to the game only in the menus, cinematics and with `vr.aim =
  "gamepad"`; its down ejects a magazine with `vr.manual_reload`), so no
  binding is taken from anything. On foot, with the controllers aiming.
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
  within half a second; with bullet time off it toggles at once as you push)
  to sprint, and again to stop. It also ends as you let the left stick go for
  a second, crouch, or leave your feet (a vehicle, a cinematic, the menus,
  death). The HUD says "Sprint".
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
