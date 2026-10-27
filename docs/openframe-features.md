# OpenFrame features

Features OpenFrame (the Steam Frame fork of OpenH1) adds to OpenH1's VR mode,
where each came from, and how to turn it on. Each is set in `config.toml`
(the `vr.*` keys below) or in VR Setup > VR Display > COMFORT...
Every one is off, or as OpenH1 had it, until you choose it.

**Untested in the headset** marks a feature built and compiled but not yet
played on a Steam Frame.

## Ported from HaloCE-DecompRemix-VR

Ported from moistman42069's HaloCE-DecompRemix-VR
(<https://github.com/moistman42069/HaloCE-DecompRemix-VR-PC-Android>,
AGPL-3.0-or-later, as OpenH1 is), re-implemented on OpenH1's VR layer
(`port/linux/src/vr.c` and the game's side): the two VR layers
share almost no code, so mostly the ideas and the tuning were carried over;
the few functions whose lines came across say so at their top.

### Comfort vignette for moving and turning (untested in the headset)

OpenH1's comfort vignette dimmed the edges of the view only in vehicles
(handlebars' and wheels' turns, speeding up and braking, a seat's ride).
`"always"` also dims them as the left stick moves you on foot, as the
turning stick turns you smoothly, and for a moment at each snap turn
(`port/linux/src/vr.c`, `comfort_update`).

- Keys: `vr.vignette` (`"vehicle"`, the default and OpenH1's behaviour;
  `"always"`; `"off"`), `vr.vignette_strength` (0.25 to 1.5, default `1.0`:
  how much of the edges, and how dark). `vr.comfort_vignette = false`, the
  older switch, still turns it off.
- Input: none.
