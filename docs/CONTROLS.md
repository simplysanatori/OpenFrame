# Controls

The Frame's controllers are the two halves of an Xbox controller. The right one holds and aims the weapon. If you play left-handed (DOMINANT HAND set to LEFT), the triggers and the grips swap, the pouch and the holsters mirror, and the sticks swap with their clicks (SWAP STICKS set to AS THE HAND, the default), so "the turning stick" below means your left stick. A, B, X, Y, the bumpers, the d-pad, View and Menu stay where they are.

## The controllers

| Steam Frame | In the game |
|---|---|
| Right controller | Aim. It holds the weapon and the reticle shows where it points |
| Head | Look. Crouch in the room and the game crouches |
| Right trigger | Fire |
| Left trigger | Second weapon in a Banshee. On foot you throw grenades by hand (GRENADE THROW set to BOTH: the trigger throws too; TRIGGER: only the trigger throws) |
| Left stick | Move |
| Left stick click | Crouch while you hold it (CROUCH set to TOGGLE: a tap crouches until you press it again). You stay crouched at a full push of the stick (CROUCH WALK). While sprinting, slide (PARKOUR on) |
| Turning stick, left and right | Turn, in steps or smoothly |
| Turning stick, flick up | Sprint on or off (SPRINT set to TOGGLE) |
| Turning stick, flick down | Bullet time on or off (BULLET TIME on) |
| Turning stick, held down 0.8 seconds | Third person on or off. Needs no setting |
| Turning stick, pressed in | Zoom, as in the flat game, each time you press it, in bullet time as well |
| Turning stick, while you are dead | Move the death camera round your body (the default death view) |
| **A** | Jump |
| **B** | Melee. A hard swing of the right controller melees too |
| **X** | Action and reload. Hold to pick up a weapon |
| **Y** | Switch weapon |
| Left bumper | Flashlight |
| Right bumper | Switch grenade type |
| D-pad | Menus |
| **View** | Back, and the multiplayer scoreboard |
| **Menu** | Pause |
| Both grips, held | Recentre |
| Right grip | Draw a weapon from a holster, over the shoulder or at the hip |
| Left grip on a long gun's foregrip | Aim it with both hands |
| Left grip at the left hip | Take a magazine. The old one drops out of the weapon by itself; push the new one in to load. The shotgun takes shells one at a time; the rocket launcher's tubes are behind your left shoulder |
| Left grip held, swung and let go | Throw a grenade where your hand threw it |
| A raised scope at your eye | Look through it, magnified (SCOPES set to AT THE EYE) |
| Right grip on a weapon on the ground | Pick it up, or swap it for the one in your hand |
| Either hand pushed into a control | Press it |
| Both hands low on an overturned vehicle, lifted together | Flip it. With `vr.vehicle_flip = "button"` in config.toml, X flips it instead |
| Grip a vehicle's holds and pull | Climb in or out. X works as ever |
| Both grips on a Ghost's handles or a Warthog's wheel | Steer it |

## The turning stick's gestures

- A flick counts when the stick goes within about 34 degrees of straight up or down. A diagonal push only turns you. With SPRINT set to TOGGLE or BULLET TIME on, turning pauses while the stick is inside that cone, so a flick never turns you.
- The gestures work on foot, aiming with the controllers, with no menu open.
- Flick up: sprint on or off, at the moment of the push. With the default bindings, holding it up for 0.8 seconds takes the switch back.
- Flick down and let go within 0.8 seconds: bullet time on or off. It switches as you let go, and rests half a second between switches.
- Hold down for 0.8 seconds: third person on or off. It needs no setting, and the game remembers the view you left it in.
- Pressed in: zoom, once for each press. Nothing waits for the click to be held.
- There is no magazine eject on the stick. Reload with **X**, or take a magazine from your left hip.
- Sprint, bullet time and the third person show a label on the HUD when they switch.

The GESTURES page, under MOVEMENT AND COMFORT, offers other bindings. **BULLET TIME BINDING**: **FLICK DOWN** (the default), **CLICK, HELD** (a short press still zooms, a press held 0.4 seconds is bullet time), **STICK CLICK** (the click is bullet time and no longer zooms on foot), **DOUBLE FLICK DOWN**, **DOUBLE FLICK UP**, **HOLD UP** or **OFF**. **THIRD PERSON BINDING**: **HOLD** (the default), **DOUBLE FLICK DOWN** or **OFF**. With the third person on a double flick down, a flick-down bullet time moves to a double flick up.

## The added features

These come from moistman42069's Halo VR mod, rewritten for OpenH1's renderer and the Frame's controllers. Sprint, bullet time, parkour, fists, weapon pull, prop throw and hand swap work in the campaign and are each off until you turn them on in VR Setup. The third person works in the campaign with no setting. The wrist HUD and the vignette are options for everywhere.

### MOVEMENT AND COMFORT

- **SPRINT**: TOGGLE for the stick flick, or ARM SWING to run faster by swinging both arms. **SPRINT SPEED** sets how much faster, 1.5 times by default. It speeds you forward and sideways, not backing. Crouching pauses a sprint. It ends in a vehicle or when you die.
- **BULLET TIME**: the world slows to 40% while your head and hands stay at full speed. Your own body slows with the world, so jumps float. **BULLET TIME SPEED** sets the slowdown. It ends by itself at a cinematic, when you die, or when the game goes back to a checkpoint.
- **THIRD PERSON** and **VIEW DISTANCE**: the camera floats 1.7 m behind your head, over the shoulder of your weapon hand, and tucks in at walls. You still aim with the controller. While you zoom, ride a vehicle, climb into one, watch a cinematic or die, you see in first person. Not in multiplayer: the HUD says so.
- **PARKOUR**: slide, wall jump and wall run.
  - Slide: crouch while sprinting at a run, by the stick click or by ducking your head. A click-started slide runs its whole way with the click let go, up to 1.1 seconds. Click again to stand early. A slide by your head ends as you stand up. **SLIDE MOMENTUM** sets its length and speed.
  - Wall jump: in the air beside a wall, press **A** again. You leap where you look. **WALL JUMP** sets the strength.
  - Wall run: jump at a wall with the left stick pushed along it. You run along the wall for up to two seconds. Steer away to drop, or jump off it.
- **VIGNETTE**: VEHICLES as in OpenH1, ALWAYS to also narrow the view when you move or turn on foot, or OFF. **VIGNETTE STRENGTH** sets how much.

### HANDS AND WEAPONS, under FISTS AND GRABBING

- **FISTS**: your free hand punches, and so does your weapon hand when it holds no weapon. Swing it at an enemy at 2 m/s or more. A hard swing staggers them for a second. Swipe across an enemy's melee as it lands to parry it; the HUD says "Parry". **PUNCH SPEED** and **PUNCH DAMAGE** tune it.
- **WEAPON PULL**: hold your free hand out, point it at a weapon on the floor within **PULL RANGE**, 3 m by default, squeeze the grip and flick the hand back. The weapon flies to your hand. Keep the grip squeezed to catch it.
- **PROP THROW**: squeeze the grip with your free hand on a loose weapon or piece of equipment to pick it up, and let go to throw it. **THROW STRENGTH** scales the throw.
- **HAND SWAP**: palms together, squeeze your free hand's grip and open the other. The weapon changes hands, and the game remembers which hand is yours. The sticks stay where they were.

### HUD

- **HUD STYLE**: VISOR as in OpenH1, WRIST to carry shields, ammo and the motion tracker on the back of your off hand's wrist, or HIDDEN. Turn the wrist toward your face to read it. **WRIST HUD** sets its size and place on the arm. The reticle, messages and waypoints stay on the visor, and the visor comes back in menus and in vehicles.
- **HUD LAYOUT**: sliders in percent to bring the corners in, and to move or resize the ammo, shields and tracker, together or one by one. Changes apply when you press OK on the page.

[openframe-features.md](openframe-features.md) has every setting of each feature.
