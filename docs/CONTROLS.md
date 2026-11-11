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
| Left stick click | Crouch while you hold it. With CROUCH set to TOGGLE, a quick tap keeps you crouched until you press it again, jump, take a seat or die; a longer press still crouches only while held. You stay crouched at a full push of the stick (CROUCH WALK). While sprinting, slide (PARKOUR on) |
| Turning stick, left and right | Turn, in steps or smoothly. In a vehicle's seat it turns smoothly even with snap turning, and snaps again once you are out and have let the stick back to the middle (VR Setup > MOVEMENT AND COMFORT > GESTURES... > SMOOTH TURN IN SEATS: OFF keeps snapping in seats) |
| Turning stick, flick up | Sprint on or off (SPRINT set to TOGGLE) |
| Turning stick, flick down | Bullet time on or off (BULLET TIME on) |
| Turning stick, held down 0.8 seconds | Third person on or off. Needs no setting |
| Turning stick, pressed in | Zoom, each time you press it, in bullet time as well. With SCOPES set to SMART-LINK (the default) the zoom opens a magnified window around the reticle. With AT THE EYE it changes the magnification of a scope held to your eye. With OFF nothing is magnified in the headset |
| Turning stick, while you are dead | Move the death camera round your body (the default death view) |
| **A** | Jump |
| **B** | Melee. A hard swing of the right controller melees too, and so does a softer one with the right grip held. With FISTS on and no weapon in your hand, the swing is a punch instead |
| **X** | Action and reload. Hold to pick up a weapon |
| **Y** | Switch weapon |
| Left bumper | Flashlight |
| Right bumper | Switch grenade type |
| D-pad | Menus |
| **View** | Back, and the multiplayer scoreboard |
| **Menu** | Pause |
| Both grips, held | Recentre. The game also recentres by itself at each level start and after a cinematic, and opens menus in front of you (AUTO RECENTRE, on by default) |
| Right grip | Draw a weapon from a holster, over the shoulder or at the hip. Drawing it also switches to it |
| Left grip on a long gun's foregrip | Aim it with both hands |
| Left grip at the left hip | Take a magazine, when the weapon has room for one. The old one drops out of the weapon by itself; push the new one in to load. The shotgun takes shells one at a time; the rocket launcher's tubes are behind your left shoulder |
| Left grip held, hand drawn back, swung forward and let go | Throw a grenade the way your hand threw it |
| The sniper rifle's or the pistol's scope raised to your eye | Look through it, magnified (SCOPES set to AT THE EYE) |
| Right grip on a weapon on the ground | Pick it up, or swap it for the one in your hand |
| Either hand pushed into a control | Press it |
| Both hands low on an overturned vehicle, lifted together | Flip it. With `vr.vehicle_flip = "button"` in config.toml, X flips it instead |
| Grip a vehicle's holds and pull | Climb in or out. X works as ever |
| Both grips on a Ghost's handles or a Warthog's wheel | Steer it |

## The turning stick's gestures

- A flick counts when the stick goes within about 34 degrees of straight up or down. A diagonal push only turns you.
- With SPRINT set to TOGGLE or BULLET TIME on, turning pauses while the stick is pushed up inside that cone. With BULLET TIME on, it also pauses while the stick is pushed down inside it. Once a hold has switched the third person, turning pauses until you let go. So a flick that does something never turns you.
- The gestures work on foot, aiming with the controllers, with no menu open.
- Flick up: sprint on or off, at the moment of the push. With the default bindings, holding it up for 0.8 seconds takes the switch back.
- Flick down and let go within 0.8 seconds: bullet time on or off. It switches as you let go, and rests half a second between switches.
- Hold down for 0.8 seconds: third person on or off. It needs no setting, and the game remembers the view you left it in.
- Pressed in: zoom, once for each press. Nothing waits for the click to be held.
- There is no magazine eject on the stick. Reload with **X**, or take a magazine from your left hip.
- Sprint, bullet time and the third person show a label on the HUD when they switch.

The GESTURES page, under MOVEMENT AND COMFORT, offers other bindings. **BULLET TIME BINDING**: **FLICK DOWN** (the default), **CLICK, HELD** (a short press zooms as you let go, a press held 0.4 seconds is bullet time), **STICK CLICK** (the click is bullet time and does not zoom), **DOUBLE FLICK DOWN**, **DOUBLE FLICK UP**, **HOLD UP** or **OFF**. **THIRD PERSON BINDING**: **HOLD** (the default), **DOUBLE FLICK DOWN** or **OFF**. The two click bindings only take the click on foot, in first person, in a game where bullet time works. Anywhere else the click zooms as usual. With the third person on a double flick down, a flick-down bullet time moves to a double flick up.

## The added features

These come from moistman42069's Halo VR mod, rewritten for OpenH1's renderer and the Frame's controllers. Sprint, bullet time, parkour, fists, shot deflection, weapon pull, prop throw and hand swap are each off until you turn them on in VR Setup. They work only in a game on your own Frame, such as the campaign. Online, in split screen and in games with bots they do nothing, and the HUD says "offline only" for sprint and bullet time. The VR Tutorial has bots, so they are off there too. The third person works in the campaign with no setting. The wrist HUD and the vignette are options for everywhere.

### MOVEMENT AND COMFORT

- **SPRINT**: TOGGLE for the stick flick, or ARM SWING to run faster by swinging both arms. **SPRINT SPEED** sets how much faster, 1.5 times by default. It speeds you up going forward or on a diagonal. Pushing the stick back, or mostly sideways, does not sprint. Crouching pauses a sprint. It ends in a vehicle or when you die.
- **BULLET TIME**: the world slows to 40% while your head and hands stay at full speed. Your own body slows with the world, so jumps float. **BULLET TIME SPEED** sets the slowdown. It ends by itself at a cinematic, when you die, or when the game goes back to a checkpoint.
- **THIRD PERSON** and **VIEW DISTANCE**: the camera floats 1.7 m behind your head, over the shoulder of your weapon hand, and tucks in at walls. You still aim with the controller. While you zoom, ride a vehicle, climb into one, watch a cinematic or die, you see in first person. Not in multiplayer: the HUD says so.
- **PARKOUR**: slide, wall jump and wall run.
  - Slide: crouch while sprinting at a run, by the stick click or by ducking your head. A click-started slide runs its whole way with the click let go, up to 1.1 seconds. Click again to stand early. A slide by your head ends as you stand up. **SLIDE MOMENTUM** sets its length and speed.
  - Wall jump: in the air beside a wall, press **A** again. You leap where you look, always at least partly away from the wall. You get one wall jump off each wall until you land. **WALL JUMP** sets the strength.
  - Wall run: jump at a wall with the left stick pushed along it. You run along the wall for up to two seconds. Steer away to drop, or jump off it.
- **VIGNETTE**: VEHICLES as in OpenH1, ALWAYS to also narrow the view when you move or turn on foot, or OFF. **VIGNETTE STRENGTH** sets how much.

### HANDS AND WEAPONS, under FISTS AND GRABBING

- **FISTS**: your free hand punches, and so does your weapon hand when it holds no weapon. Swing it at an enemy at 2 m/s or more. A hard swing staggers them for a second. Hunters, and Elites whose shields still hold, are never staggered. Swipe across an enemy's melee as it lands to parry it; the HUD says "Parry". **PUNCH SPEED** and **PUNCH DAMAGE** tune it.
- **DEFLECT SHOTS**: hold your gun in the path of an incoming shot and it glances off. Swing the gun, or a free fist, at 1.2 m/s or more to bat a shot away along your swing. Plasma and needles deflect off either; bullets only off the gun; grenades bounce or are batted; rockets and fuel rods need a hard swing of 2.5 m/s; tank shells and flame never. A deflected shot is yours, and its kills count for you. Your hand buzzes when it deflects one. `vr.deflect_strength` and `vr.deflect_assist` in `config.toml` set its speed and how near it must pass.
- **WEAPON PULL**: hold your free hand out, point it at a weapon on the floor within **PULL RANGE**, 3 m by default, squeeze the grip and flick the hand back. The weapon flies to your hand. Keep the grip squeezed to catch it.
- **PROP THROW**: squeeze the grip with your free hand on a loose weapon or piece of equipment to pick it up, and let go to throw it. **THROW STRENGTH** scales the throw.
- **HAND SWAP**: palms together, squeeze your free hand's grip and open the other. The weapon changes hands, and the game remembers which hand is yours. The sticks stay where they were.

### HUD

- **HUD STYLE**: VISOR as in OpenH1, WRIST to carry shields, ammo and the motion tracker on the back of your off hand's wrist, or HIDDEN. Turn the wrist toward your face to read it. It hides while that hand holds a long gun's foregrip. **WRIST HUD** sets its size and place on the arm. The reticle, messages and waypoints stay on the visor, and the visor comes back in menus and in vehicles.
- **HUD LAYOUT**: sliders in percent to bring the corners in, and to move or resize the ammo, shields and tracker, together or one by one. Changes apply when you press OK on the page.

[openframe-features.md](openframe-features.md) has every setting of each feature.
