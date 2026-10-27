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

### Sprint (untested in the headset)

Runs faster than the game's run, on foot and standing (not crouched), forward
and sideways (not backing), accelerating faster with it. The view's field is
unchanged. `source/units/bipeds.c` multiplies the tick's speeds for the first
local player (`vr_sprint_speed_scale`).

- Keys: `vr.sprint_mode` (`"off"`, the default; `"toggle"`;
  `"arm_swing"`), `vr.sprint_speed` (times the run, 1.1 to 2, default `1.5`).
- `"toggle"`: flick the turning stick (the right, or the left with
  `vr.swap_sticks`) up to sprint, and again to stop. It also ends as you let the left stick go for a
  second, crouch, or leave your feet (a vehicle, a cinematic, the menus,
  death). The HUD says "Sprint". The stick's up is no game control while
  the controllers aim (OpenH1 sends the right stick to the game only in the
  menus, cinematics and with `vr.aim = "gamepad"`; its down ejects a
  magazine with `vr.manual_reload`), so no binding is taken from anything.
- `"arm_swing"`: swing both arms up and down as you run; the faster the
  slower arm moves (from 0.6 m/s, full at 1.5 m/s) the faster you go, up to
  `vr.sprint_speed`. With the left stick let go the swing walks you forward
  along your aim.
- Where: a game of this computer alone (the campaign). Never online, in split
  screen or with bots (network games, which every machine must step alike).

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
