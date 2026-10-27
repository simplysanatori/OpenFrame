/*
VR_GRAB.C

The VR mode's hands grabbing the world (HALO_VR; vr.grab_world,
port/linux/README.md "VR"): the game's side of what port/linux/src/vr.c sees
the hands do, for player_control.c, players.c and render_objects.c.

Halo's action button (X) does what the game offers the player where they
stand (players.c, player_examine_nearby_objects): a weapon on the ground at
the player's feet is offered to pick up (with one weapon carried) or to swap
for the one in hand (with two), and X held a moment takes it (the swap bit,
player_handle_weapon_swap). In the VR mode the right hand takes it as a hand
would: reached down to the weapon (its grip within 20 cm of it) and gripping
(vr.c, grab_update). The weapon lights up as the hand nears it, as the
holsters' weapons do (vr_holster.c), the controller ticks as the hand gets
there, and the grip presses the game's pick-up, the swap bit, through its
input and the netcode as X held does: as a network game's client, the host
picks it up as it would for X. The weapon taken goes where the game puts it:
in hand, the one held dropped for it, or the one held put in its holster.

The game offers one weapon at a time, the nearest the player's body
(player_set_action_result); with several about the feet it offers the one
the right hand reaches for instead (vr_grab_offer), so that the grip takes
what the hand is at, and the HUD's prompt names it. A network game's client
offers what the host would (the host decides pick-ups, by the body), and its
hand takes only that one. X keeps working as ever. Ammunition is picked up
by walking over it, as ever.

A control the game would let the player use (a device control: a door's
panel, a bridge's, a lift's: "Press X to activate"; in reach of the body,
able to change, facing the eye: player_examine_nearby_device) is pressed by
pushing either hand's fingers into its face (vr.c, press_update), a click
in the hand: the game's action, as X. The game offers a control only along
the player's aim, which in the VR mode is the right controller's, whatever
the hand pressing does; so as the hand presses, the game's aim turns to the
control (vr_grab_facing) until the game offers it, a tick later, and its
action is pressed, as a network game's host decides a client's use of a
control by the client's aim: the host, and the client's copy, both see the
aim on the control as the action comes. The aim is the game's alone (the
reticle, the weapon and the view follow the controller), for a few ticks;
the controls of the campaign are large (a Forerunner holo panel, 2 m
across) and the game mostly offers them at arm's length whatever the aim.
Entering a vehicle stays on X.

(OpenFrame) The hand without the weapon (docs/openframe-features.md), in a
game of this computer alone (game_connection local: the campaign; never
online, split screen or with bots), on foot: vr_fists.c's hands' tick
gives it the hands once a tick (vr_grab_hands_tick), and it tells vr.c what
that hand's grip would take (vr_grab_hands_claims), which then takes it
rather than a grenade or the foregrip.
- vr.weapon_pull: the hand pointed at a weapon lying within
  vr.weapon_pull_range (within 20 degrees of where it points, in sight of
  the eyes; it lights up), the grip locks on it (it lights up fully) and a
  flick of the hand back toward you or up (1 m/s) throws it to the hand, in
  an arc, in about half a second; still gripped as it arrives, it is
  picked up as the right hand's grab picks one up (the game's pick-up, as X
  held: it is offered first), open, it drops at your feet.
- vr.prop_throw: the hand at a loose weapon or piece of equipment (within
  15 cm of it; it lights up) holds it while the grip is closed, turned as
  the hand points, and throws it as it opens, at the hand's speed (its
  fastest of the last few ticks) times vr.prop_throw_strength, as the game
  drops a weapon (the player's body passed through, kept inside the level).
  The right hand's grab takes the one held as ever.
- vr.hand_swap (vr.c): the HUD says which hand the weapon is in.
Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later): its
WEAPON PULL (vr_weapon_pickup.c: the candidate, its scoring and sight, the
lock, the flick, the flight's time and arc, the catch) and its world grab's
loose items (vr_world_interaction.c: held at the hand, thrown at the hand's
peak speed times OBJECT THROW, at most 24 m/s). Its scenery props (their
own collision solver), vehicles and bodies are not carried over.
*/

#ifdef HALO_VR

#include "cseries.h"
#include "math/real_math.h"
#include "ai/ai_script.h"
#include "cache/cache_files.h"
#include "devices/device_controls.h"
#include "devices/devices.h"
#include "game/game.h"
#include "game/players.h"
#include "items/items.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "networking/network_game_globals.h"
#include "objects/object_definitions.h"
#include "objects/objects.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "render/render.h"
#include "tag_files/tag_files.h"
#include "units/units.h"

#include "../src/vr.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

/* port/linux/src/port_config.c */
char const *config_string(char const *name);
/* port/linux/game/vr_render.c */
boolean vr_render_world_place(real_point3d const *point, float place[3]);
/* port/linux/game/vr_holster.c */
void vr_holster_highlight_object(long unit_index, real distance, real units, struct render_model_effect *model_effect,
	struct render_lighting *lighting);

void platform_log(char const *format, ...);

/* the game offers the weapon the right hand is nearest instead of the one
nearest the body, the hand nearer its reach than this (metres: where it
begins to light up, vr_holster.c's HIGHLIGHT_DISTANCE) */
#define VR_GRAB_PREFER_DISTANCE 0.35f
/* a weapon is the line through its middle along its length, this much of
its bounding sphere's radius either way, and this thick (metres) */
#define VR_GRAB_WEAPON_LENGTH 0.8f
#define VR_GRAB_WEAPON_THICKNESS 0.04f
/* the weapon the grip took is picked up once the game offers it, within
this many ticks (the game offers the hand's weapon a tick after it is the
hand's), the pick-up pressed for this many ticks (the game takes the swap
bit's press, not its hold) */
#define VR_GRAB_WAIT_TICKS 15
#define VR_GRAB_PRESS_TICKS 3
/* a control is pressed within this radius of its face's middle (metres):
half its bounding sphere's, within these */
#define VR_GRAB_CONTROL_SMALLEST 0.06f
#define VR_GRAB_CONTROL_LARGEST 0.5f
/* a control pressed is used once the game offers it, within this many
ticks, the game's aim turned to it meanwhile (the game offers the control
its aim is on), its action pressed for this many ticks and the aim held on
it one more */
#define VR_GRAB_CONTROL_WAIT_TICKS 10
#define VR_GRAB_ACTION_TICKS 2
/* debug.vr_test_grab: placed this many seconds after the player first
aims, this many at most */
#define VR_GRAB_TEST_SECONDS 3
#define VR_GRAB_TEST_THINGS 2
/* (OpenFrame) the pull: a weapon within PULL_FACING (a cosine: 20 degrees)
of where the hand points, or PULL_KEPT for the one it pointed at, or within
PULL_NEAR of the hand, the arm reached out (PULL_ARM_OUT); the flick PULL_FLICK_SPEED (metres a second); the
flight PULL_FLIGHT_BASE seconds and its distance at PULL_FLIGHT_SPEED, at
least PULL_FLIGHT_SHORTEST and at most PULL_FLIGHT_LONGEST, arcing up a
quarter of its distance (at most PULL_ARC_HIGHEST) and PULL_ARC_LEAST; it
arrives within PULL_ARRIVED of the hand. The weapon caught is picked up
once the game offers it, within VR_GRAB_WAIT_TICKS. A prop: within
PROP_REACH of the hand; thrown at the fastest of the hand's last
PROP_HISTORY ticks times vr.prop_throw_strength, at most PROP_FASTEST
(metres a second) */
#define PULL_FACING 0.94f
#define PULL_KEPT 0.82f
#define PULL_NEAR 0.30f
#define PULL_FLICK_SPEED 1.0f
#define PULL_FLIGHT_BASE 0.25f
#define PULL_FLIGHT_SPEED 8.0f
#define PULL_FLIGHT_SHORTEST 0.30f
#define PULL_FLIGHT_LONGEST 0.70f
#define PULL_ARC_HIGHEST 0.6f
#define PULL_ARC_LEAST 0.10f
#define PULL_ARRIVED 0.22f
#define PULL_SEARCH_COUNT 32
/* (the arm reached out for a pull: the hand this far from the eyes at
least, so that a grenade taken by the body is a grenade) */
#define PULL_ARM_OUT 0.40f
#define PROP_REACH 0.15f
#define PROP_HISTORY 4
#define PROP_FASTEST 24.0f

/* port/linux/src/port_config.c */
int config_boolean(char const *name);
double config_real(char const *name);
unsigned long config_changes(void);
/* port/linux/game/vr_render.c */
boolean vr_render_hand_world(const float place[3], const float direction[3], real_point3d *point,
	real_vector3d *world_direction, real *units);

static struct
{
	/* the weapons the game offered the first player at its last tick
	(players.c), and that tick */
	long offered_tick;
	short offered_count;
	long offered[HALO_VR_GRAB_THINGS];
	/* the grab is on (vr.c, last frame), the weapon the right hand is
	nearest (NONE: none) and how far from its reach (metres), the world's
	units a metre */
	boolean enabled;
	long nearest;
	real distance;
	real units;
	/* the weapon the grip took, until the game offers it (NONE: none) or
	the tick it is given up at; the pick-up pressed until this tick */
	long taken;
	long taken_until;
	long press_until;
	/* the control a hand pressed, until the game offers it (NONE: none) or
	the tick it is given up at; its action pressed until this tick, and the
	aim on it until that one */
	long control;
	long control_until;
	long action_until;
	long facing_until;
	/* debug.vr_test_grab: when the player first aimed (NONE: not yet, on
	this map), the thing placed, and the weapons the player carried, logged */
	long test_first_aim_tick;
	boolean test_placed;
	long test_weapon_index[VR_GRAB_TEST_THINGS];
	long test_carried[2];
	char test_logged[VR_GRAB_TEST_THINGS][160];
} vr_grab = { NONE, 0, { 0 }, FALSE, NONE, 0.0f, 0.0f, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, FALSE,
	{ NONE, NONE }, { NONE, NONE } };

/* the pick-up of a weapon, as the game offers it */
static boolean vr_grab_weapon_result(short action_result)
{
	return action_result == _player_action_result_swap_for_weapon ||
		action_result == _player_action_result_add_weapon_to_inventory;
}

/* the first local player's unit on foot and alive (NULL: not) */
static struct unit_datum *vr_grab_unit(long player_index)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit;

	if (player->unit_index == NONE)
		return NULL;
	unit = (struct unit_datum *)object_try_and_get_and_verify_type(player->unit_index, _object_mask_unit);
	if (!unit || unit->object.parent_object_index != NONE || TEST_FLAG(unit->object.damage_flags, _object_dead_bit))
		return NULL;
	return unit;
}

/* debug.vr_test_grab: a weapon laid on the ground before the unit, its tag
name having name in it, this far ahead and right of it (metres), across its
way (its muzzle to the left); NONE: none made */
static long vr_grab_test_weapon(struct unit_datum *unit, char const *name, real ahead, real right)
{
	struct tag_iterator iterator;
	struct object_placement_data placement;
	struct collision_result collision;
	real_vector3d forward, down = { 0.0f, 0.0f, -6.0f / 3.048f };
	real_point3d above;
	long tag_index, weapon_index;

	tag_iterator_new(&iterator, WEAPON_DEFINITION_TAG);
	for (tag_index = tag_iterator_next(&iterator); tag_index != NONE; tag_index = tag_iterator_next(&iterator))
	{
		char const *tag_name = tag_get_name(tag_index);

		if (tag_name && strstr(tag_name, name))
			break;
	}
	forward = unit->object.forward;
	forward.k = 0.0f;
	if (tag_index == NONE || normalize3d(&forward) <= 0.0f)
	{
		platform_log("vr grab: no test weapon \"%s\" in this map", name);
		return NONE;
	}
	/* (on the ground there) */
	object_placement_data_new(&placement, tag_index, NONE);
	placement.position.x = unit->object.position.x + (forward.i * ahead + forward.j * right) / 3.048f;
	placement.position.y = unit->object.position.y + (forward.j * ahead - forward.i * right) / 3.048f;
	placement.position.z = unit->object.position.z;
	above = placement.position;
	above.z += 1.0f / 3.048f;
	if (global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH)
	{
		global_current_collision_users[global_current_collision_user_depth++] = _collision_user_ui;
		if (collision_test_vector(FLAG(_collision_test_structure_bit), &above, &down, NONE, &collision))
			placement.position.z = collision.point.z;
		--global_current_collision_user_depth;
	}
	placement.position.z += 0.05f / 3.048f;
	placement.forward.i = -forward.j;
	placement.forward.j = forward.i;
	placement.forward.k = 0.0f;
	placement.up = *global_up3d;
	weapon_index = object_new(&placement);
	platform_log("vr grab: test weapon %s, %.2f m ahead, %.2f m right%s", tag_get_name(tag_index), ahead, right,
		weapon_index == NONE ? ": not made" : "");
	return weapon_index;
}

/* debug.vr_test_grab: "control [<n> [<metres>]]": the controls the unit
could use (able to change) logged, and the unit stood before the nth
nearest (the nearest by default), so far from its face (0.6 m by default),
and (turn) the first player turned to it */
static long vr_grab_test_control(long unit_index, boolean turn, char const *test)
{
	short wanted = 0;
	real metres = 0.6f;
	struct unit_datum *unit = unit_get(unit_index);
	struct object_iterator iterator;
	long usable[32];
	real distances[32];
	short count = 0, index, rank;
	struct object_datum *control;
	struct object_marker marker;
	real_point3d face, place, above;
	real_vector3d front, down = { 0.0f, 0.0f, -3.0f / 3.048f };
	struct collision_result collision;
	real yaw;

	sscanf(test, "%hd %f", &wanted, &metres);
	object_iterator_new(&iterator, _object_mask_control, 0);
	while ((control = (struct object_datum *)object_iterator_next(&iterator)) != NULL)
	{
		real distance = distance3d(&unit->object.position, &control->object.bounding_sphere_center);
		boolean marked = object_get_marker_by_name(iterator.index, "front", &marker, 1) == 1;
		platform_log("vr grab: control %lx %s, %.1f m away, %.2f m across%s%s%s", iterator.index,
			tag_name_strip_path(tag_get_name(control->definition_index)), distance * 3.048f,
			control->object.bounding_sphere_radius * 2.0f * 3.048f,
			device_can_change_position(iterator.index) ? ", usable" : "", marked ? ", its front marked" : "",
			TEST_FLAG(control->object.flags, _object_outside_of_map_bit) ? ", outside the map" : "");
		if (device_can_change_position(iterator.index) && count < NUMBEROF(usable) &&
			!TEST_FLAG(control->object.flags, _object_outside_of_map_bit))
		{
			usable[count] = iterator.index;
			distances[count++] = distance;
		}
	}
	/* (the wanted'th nearest: as many nearer) */
	for (index = 0; index < count; index++)
	{
		short other;

		for (rank = 0, other = 0; other < count; other++)
			rank += distances[other] < distances[index] || (distances[other] == distances[index] && other < index);
		if (rank == wanted)
			break;
	}
	if (index >= count)
	{
		platform_log("vr grab: no usable control %d in this map (%d)", wanted, count);
		return NONE;
	}
	control = object_get(usable[index]);
	face = control->object.bounding_sphere_center;
	vector_from_points3d(&face, &unit->object.position, &front);
	if (object_get_marker_by_name(usable[index], "front", &marker, 1) == 1)
	{
		face = marker.matrix.position;
		front = marker.matrix.forward;
	}
	front.k = 0.0f;
	if (normalize3d(&front) <= 0.0f)
		return NONE;
	/* (on the ground before it) */
	place.x = face.x + front.i * metres / 3.048f;
	place.y = face.y + front.j * metres / 3.048f;
	place.z = face.z;
	above = place;
	if (global_current_collision_user_depth < MAXIMUM_COLLISION_USER_STACK_DEPTH)
	{
		global_current_collision_users[global_current_collision_user_depth++] = _collision_user_ui;
		if (collision_test_vector(FLAG(_collision_test_structure_bit), &above, &down, NONE, &collision))
			place.z = collision.point.z;
		--global_current_collision_user_depth;
	}
	place.z += 0.05f / 3.048f;
	object_set_position(unit_index, &place, NULL, NULL);
	yaw = (real)atan2(-front.j, -front.i);
	if (turn)
		halo_vr_test_face(yaw);
	platform_log("vr grab: unit %lx stood %.2f m before control %lx %s, its face %.2f m above the feet", unit_index,
		metres, usable[index], tag_get_name(control->definition_index), (face.z - place.z) * 3.048f);
	return usable[index];
}

/* debug.vr_test_grab = "control" as a network game's client: the first
player turned, once, to the usable control whose face is within 1.5 m of
the unit across the floor (the host stands them there: vr_grab_test); TRUE
once turned */
static boolean vr_grab_test_control_turn(struct unit_datum *unit)
{
	struct object_iterator iterator;
	struct object_datum *control;
	struct object_marker marker;

	object_iterator_new(&iterator, _object_mask_control, 0);
	while ((control = (struct object_datum *)object_iterator_next(&iterator)) != NULL)
	{
		real_vector3d front, across;

		if (!device_can_change_position(iterator.index) ||
			object_get_marker_by_name(iterator.index, "front", &marker, 1) != 1)
		{
			continue;
		}
		vector_from_points3d(&unit->object.position, &marker.matrix.position, &across);
		if (across.i * across.i + across.j * across.j > (1.5f / 3.048f) * (1.5f / 3.048f) || fabs(across.k) > 1.0f)
			continue;
		front = marker.matrix.forward;
		halo_vr_test_face((real)atan2(-front.j, -front.i));
		platform_log("vr grab: turned to control %lx", iterator.index);
		vr_grab.test_weapon_index[0] = iterator.index;
		return TRUE;
	}
	return FALSE;
}

/* debug.vr_test_grab: "weapon <name>[ @ <metres ahead> <metres right>][;
weapon ...]", up to VR_GRAB_TEST_THINGS weapons whose tag names have name in
them laid on the ground before the first player (0.5 m ahead and 0.15 m to
the right by default) VR_GRAB_TEST_SECONDS after they first aim, once a map,
the level's AI erased (once a second), for automated tests of the grab; and
each second, as they change, where each is from the head, what the game
offers, how near the right hand is, and what the player carries */
static void vr_grab_test(long player_index, boolean aiming)
{
	char const *test = config_string("debug.vr_test_grab");
	long now = game_time_get();
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit;
	short thing;

	boolean host = game_connection() == _game_connection_network_server && !halo_vr_running();
	boolean client = network_game_distributed_client();

	if ((strncmp(test, "weapon ", 7) && strncmp(test, "control", 7)) || player->unit_index == NONE)
		return;
	/* (a network game's host, flat, stands the last player, on another
	machine, before a control; that machine's player turns to it) */
	if (host)
	{
		struct data_iterator players;
		struct player_datum *other, *last = NULL;

		data_iterator_new(&players, player_data);
		while ((other = (struct player_datum *)data_iterator_next(&players)) != NULL)
			last = other;
		if (!last || last == player || last->unit_index == NONE || strncmp(test, "control", 7))
			return;
		aiming = TRUE;
	}
	if (vr_grab.test_first_aim_tick != NONE && now < vr_grab.test_first_aim_tick)
	{
		/* (a new map, or a revert) */
		vr_grab.test_first_aim_tick = NONE;
		vr_grab.test_placed = FALSE;
		for (thing = 0; thing < VR_GRAB_TEST_THINGS; thing++)
			vr_grab.test_weapon_index[thing] = NONE;
	}
	if (vr_grab.test_first_aim_tick == NONE && aiming)
		vr_grab.test_first_aim_tick = now;
	if (vr_grab.test_first_aim_tick == NONE)
		return;
	unit = unit_get(player->unit_index);
	if ((now - vr_grab.test_first_aim_tick) % TICKS_PER_SECOND == 0)
	{
		long carried[2];
		short slot;

		/* (the level's own AI erased: nothing shoots the player; a
		client's are the host's) */
		if (!client)
			ai_scripting_erase_all();
		for (thing = 0; thing < VR_GRAB_TEST_THINGS; thing++)
		{
			long weapon_index = vr_grab.test_weapon_index[thing];
			struct object_datum *weapon = weapon_index != NONE ? object_try_and_get(weapon_index) : NULL;
			float place[3] = { 0.0f, 0.0f, 0.0f };
			char text[160];

			/* (a flat host's: from nowhere) */
			if (!weapon || (!vr_render_world_place(&weapon->object.bounding_sphere_center, place) && !host))
				continue;
			snprintf(text, sizeof(text), "at %.1f m ahead, %.1f m right, %.1f m up from the head%s; the game offers "
				"%s; the hand %s", place[0], -place[1], place[2],
				weapon->object.parent_object_index != NONE ? ", carried" :
				TEST_FLAG(_object_mask_control, weapon->object.type) ?
				(((struct device_datum *)weapon)->device.position > 0.5f ? ", on" : ", off") : "",
				player->action_object_index == weapon_index ?
					(player->action_result == _player_action_result_swap_for_weapon ? "a swap for it" : "it") :
					player->action_object_index != NONE ? "another" : "nothing",
				vr_grab.nearest != weapon_index ? "not nearest it" : vr_grab.distance <= 0.0f ? "at it" :
					vr_grab.distance < VR_GRAB_PREFER_DISTANCE ? "near it" : "nearest it, far");
			if (strcmp(text, vr_grab.test_logged[thing]))
			{
				platform_log("vr grab: test %s %d (%s) %s", TEST_FLAG(_object_mask_control, weapon->object.type) ?
					"control" : "weapon", thing,
					tag_name_strip_path(tag_get_name(weapon->definition_index)), text);
			}
			strncpy(vr_grab.test_logged[thing], text, sizeof(vr_grab.test_logged[thing]) - 1);
		}
		for (slot = 0; slot < 2; slot++)
			carried[slot] = unit_inventory_get_weapon(player->unit_index, slot);
		if (carried[0] != vr_grab.test_carried[0] || carried[1] != vr_grab.test_carried[1])
		{
			platform_log("vr grab: the player carries %s (in hand) and %s",
				carried[unit->unit.current_weapon_index & 1] != NONE ?
					tag_get_name(object_get(carried[unit->unit.current_weapon_index & 1])->definition_index) : "nothing",
				carried[(unit->unit.current_weapon_index + 1) & 1] != NONE ?
					tag_get_name(object_get(carried[(unit->unit.current_weapon_index + 1) & 1])->definition_index) :
					"nothing");
			vr_grab.test_carried[0] = carried[0];
			vr_grab.test_carried[1] = carried[1];
		}
	}
	if (vr_grab.test_placed || now < vr_grab.test_first_aim_tick + VR_GRAB_TEST_SECONDS * TICKS_PER_SECOND)
		return;
	if (client)
	{
		vr_grab.test_placed = !strncmp(test, "control", 7) && vr_grab_test_control_turn(unit);
		return;
	}
	vr_grab.test_placed = TRUE;
	if (!strncmp(test, "control", 7) && host)
	{
		struct data_iterator players;
		struct player_datum *other, *last = NULL;

		data_iterator_new(&players, player_data);
		while ((other = (struct player_datum *)data_iterator_next(&players)) != NULL)
			last = other;
		vr_grab.test_weapon_index[0] = vr_grab_test_control(last->unit_index, FALSE, test + 7);
		return;
	}
	if (!strncmp(test, "control", 7))
	{
		vr_grab.test_weapon_index[0] = vr_grab_test_control(player->unit_index, TRUE, test + 7);
		return;
	}
	for (thing = 0; thing < VR_GRAB_TEST_THINGS && test && !strncmp(test, "weapon ", 7); thing++)
	{
		char spec[96], *at, *end;
		real ahead = 0.5f, right = 0.15f;
		char const *next = strchr(test, ';');
		size_t length = next ? (size_t)(next - test) : strlen(test);

		if (length >= sizeof(spec))
			length = sizeof(spec) - 1;
		memcpy(spec, test + 7, length - 7);
		spec[length - 7] = 0;
		if ((at = strchr(spec, '@')) != NULL)
		{
			sscanf(at + 1, "%f %f", &ahead, &right);
			*at = 0;
		}
		for (end = spec + strlen(spec); end > spec && end[-1] == ' '; end--)
			end[-1] = 0;
		vr_grab.test_weapon_index[thing] = vr_grab_test_weapon(unit, spec, ahead, right);
		test = next ? next + 1 : NULL;
		while (test && *test == ' ')
			test++;
	}
}

/* ---------- (OpenFrame) the hand without the weapon: the pull, props */

enum
{
	_vr_pull_idle,
	_vr_pull_locked,
	_vr_pull_flying,
};

static struct
{
	unsigned long changes;
	boolean pull;
	real pull_range;
	boolean props;
	real strength;
} vr_hands_settings = { (unsigned long)-1, FALSE, 3.0f, FALSE, 2.0f };

static struct
{
	/* the game's tick last seen, the player's unit, the world's units a
	metre; the hand's grip taken last tick */
	long tick;
	long unit_index;
	real units;
	boolean taken;
	/* the weapon to pull the hand points at, the loose thing in its reach
	(NONE: none), as the claim told vr.c */
	long pull_candidate;
	long prop_candidate;
	/* the pull: its state, its weapon, where and when it began to fly, its
	ticks and its arc's height (world units); the weapon caught, to be
	picked up once the game offers it, until this tick */
	short pull_state;
	long pull_target;
	real_point3d pull_start;
	long pull_start_tick;
	long pull_ticks;
	real pull_arc;
	long caught;
	long caught_until;
	/* the prop held (NONE: none), its middle from its origin in its own
	axes (forward, left, up), and the hand's velocities of the last ticks
	(the world's, metres a second) */
	long held;
	real held_offset[3];
	real_vector3d history[PROP_HISTORY];
	short history_count;
	short history_next;
} vr_hands = { NONE, NONE, 0.0f, FALSE, NONE, NONE, _vr_pull_idle, NONE, { 0.0f, 0.0f, 0.0f }, NONE, 0, 0.0f, NONE,
	NONE, NONE };

static void vr_hands_settings_read(void)
{
	if (vr_hands_settings.changes == config_changes())
		return;
	vr_hands_settings.changes = config_changes();
	vr_hands_settings.pull = config_boolean("vr.weapon_pull") != 0;
	vr_hands_settings.pull_range = (real)config_real("vr.weapon_pull_range");
	if (!(vr_hands_settings.pull_range >= 1.0f && vr_hands_settings.pull_range <= 5.0f))
		vr_hands_settings.pull_range = 3.0f;
	vr_hands_settings.props = config_boolean("vr.prop_throw") != 0;
	vr_hands_settings.strength = (real)config_real("vr.prop_throw_strength");
	if (!(vr_hands_settings.strength >= 0.5f && vr_hands_settings.strength <= 3.0f))
		vr_hands_settings.strength = 2.0f;
}

/* a loose thing lying in the level (a weapon, a piece of equipment; mask),
not anyone's */
static struct item_datum *vr_hands_loose(long object_index, unsigned long mask)
{
	struct item_datum *item = object_index != NONE ?
		(struct item_datum *)object_try_and_get_and_verify_type(object_index, mask) : NULL;

	if (!item || item->object.parent_object_index != NONE ||
		!TEST_FLAG(item->object.flags, _object_connected_to_map_bit))
	{
		return NULL;
	}
	return item;
}

/* the pull let go of (the weapon left where it is, falling) */
static void vr_hands_pull_end(char const *why)
{
	if (vr_hands.pull_state != _vr_pull_idle)
		platform_log("vr hands: pull: %s", why);
	vr_hands.pull_state = _vr_pull_idle;
	vr_hands.pull_target = NONE;
}

/* the prop let go of: thrown (the hand's fastest of its last ticks, times
the strength) as the game drops a weapon, or dropped (throw FALSE) */
static void vr_hands_prop_end(long unit_index, boolean throw_it)
{
	struct item_datum *item = vr_hands_loose(vr_hands.held, _object_mask_item);

	if (item && unit_index != NONE && vr_hands.units > 0.0f)
	{
		real_vector3d velocity = *global_zero_vector3d, body;
		real_point3d eye;
		real speed = 0.0f;
		short index;

		for (index = 0; throw_it && index < vr_hands.history_count; index++)
		{
			real each = magnitude3d(&vr_hands.history[index]);

			if (each > speed)
			{
				speed = each;
				velocity = vr_hands.history[index];
			}
		}
		scale_vector3d(&velocity, vr_hands_settings.strength, &velocity);
		speed *= vr_hands_settings.strength;
		if (speed > PROP_FASTEST)
			scale_vector3d(&velocity, PROP_FASTEST / speed, &velocity);
		scale_vector3d(&velocity, vr_hands.units / (real)TICKS_PER_SECOND, &velocity);
		object_get_velocities(unit_index, &body, NULL);
		add_vectors3d(&velocity, &body, &velocity);
		item->object.translational_velocity = *global_zero_vector3d;
		item->object.angular_velocity = *global_zero_vector3d;
		item->item.ignore_object_index = unit_index;
		item_accelerate(vr_hands.held, &velocity, FALSE);
		unit_get_camera_position(unit_index, &eye);
		object_force_inside_bsp(vr_hands.held, &eye);
		platform_log("vr hands: %s %s at %.1f m/s", throw_it ? "threw" : "let go of",
			tag_get_name(item->definition_index), (double)(speed > PROP_FASTEST ? PROP_FASTEST : speed));
	}
	vr_hands.held = NONE;
	vr_hands.history_count = 0;
	vr_hands.history_next = 0;
}

static void vr_hands_reset(long unit_index)
{
	vr_hands_pull_end("reset");
	vr_hands_prop_end(unit_index, FALSE);
	vr_hands.pull_candidate = NONE;
	vr_hands.prop_candidate = NONE;
	vr_hands.caught = NONE;
	vr_hands.taken = FALSE;
}

/* in sight of the eyes: aimed a little over the weapon's middle (one
lying on the ground has its middle in the ground), what the ray meets
within 15 cm of the weapon, or past nine tenths of the way, clear. Ported
from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
vr_weapon_pickup.c's pickup_in_sight */
static boolean vr_hands_in_sight(long unit_index, long object_index, real_point3d const *eye, real units)
{
	struct object_datum *object = object_get(object_index);
	real_point3d aim = object->object.bounding_sphere_center;
	struct collision_result collision;
	real_vector3d ray;
	real length;

	aim.z += 0.06f * units;
	vector_from_points3d(eye, &aim, &ray);
	length = magnitude3d(&ray);
	if (!collision_test_vector(_collision_test_for_line_of_sight_flags, eye, &ray, unit_index, &collision))
		return TRUE;
	if (collision.type == _collision_result_object && collision.object_index == object_index)
		return TRUE;
	return collision.t >= 0.92f || (1.0f - collision.t) * length <= 0.15f * units;
}

/* the weapon the hand (at point, pointing along forward) points at within
the pull's range: nearest the line first, the one pointed at kept a little
longer, one right by the hand too. Ported from HaloCE-DecompRemix-VR by
moistman42069 (AGPL-3.0-or-later): vr_weapon_pickup.c's pickup_candidate */
static long vr_hands_pull_candidate(long unit_index, real_point3d const *point, real_vector3d const *forward)
{
	struct unit_datum *unit = unit_get(unit_index);
	real units = vr_hands.units;
	real reach = vr_hands_settings.pull_range * units, best = 1e30f;
	long candidates[PULL_SEARCH_COUNT], choice = NONE;
	real_point3d eye;
	short count, index;

	unit_get_camera_position(unit_index, &eye);
	count = objects_in_sphere(0, _object_mask_weapon, &unit->object.location, point, reach + 0.5f * units, candidates,
		PULL_SEARCH_COUNT);
	for (index = 0; index < count; index++)
	{
		struct item_datum *weapon = vr_hands_loose(candidates[index], _object_mask_weapon);
		real_vector3d to;
		real distance, facing, score;
		boolean kept = candidates[index] == vr_hands.pull_candidate;

		if (!weapon || !unit_can_use_weapon(unit_index, candidates[index]))
			continue;
		vector_from_points3d(point, &weapon->object.bounding_sphere_center, &to);
		distance = magnitude3d(&to);
		if (distance > reach + weapon->object.bounding_sphere_radius)
			continue;
		facing = distance > 0.0001f ? dot_product3d(&to, forward) / distance : 1.0f;
		if (!(distance <= PULL_NEAR * units || facing >= PULL_FACING || (kept && facing >= PULL_KEPT)))
			continue;
		if (!vr_hands_in_sight(unit_index, candidates[index], &eye, units))
			continue;
		score = (1.0f - facing) * 4.0f + distance / reach - (kept ? 0.25f : 0.0f);
		if (score < best)
		{
			best = score;
			choice = candidates[index];
		}
	}
	return choice;
}

/* the loose thing (a weapon, a piece of equipment) within PROP_REACH of the
hand: along a weapon's length (as the grab has it), about a piece of
equipment's middle */
static long vr_hands_prop_candidate(long unit_index, real_point3d const *point)
{
	struct unit_datum *unit = unit_get(unit_index);
	real units = vr_hands.units, best = PROP_REACH * units;
	long candidates[PULL_SEARCH_COUNT], choice = NONE;
	short count, index;

	count = objects_in_sphere(0, _object_mask_weapon | _object_mask_equipment, &unit->object.location, point,
		0.5f * units, candidates, PULL_SEARCH_COUNT);
	for (index = 0; index < count; index++)
	{
		struct item_datum *item = vr_hands_loose(candidates[index], _object_mask_weapon | _object_mask_equipment);
		real distance;

		if (!item)
			continue;
		if (item->object.type == _object_type_weapon && item->object.bounding_sphere_radius > 0.0f)
		{
			real half = item->object.bounding_sphere_radius * VR_GRAB_WEAPON_LENGTH;
			real_point3d base;
			real_vector3d height;

			point_from_line3d(&item->object.bounding_sphere_center, &item->object.forward, -half, &base);
			scale_vector3d(&item->object.forward, 2.0f * half, &height);
			distance = square_root(point_to_line_distance_squared3d(point, &base, &height));
		}
		else
		{
			distance = distance3d(point, &item->object.bounding_sphere_center) -
				item->object.bounding_sphere_radius * 0.5f;
		}
		if (distance < best)
		{
			best = distance;
			choice = candidates[index];
		}
	}
	return choice;
}

/* the prop held at the hand (point, pointing along forward), its middle
there, turned as the hand points, inside the level, still */
static void vr_hands_prop_hold(long unit_index, real_point3d const *point, real_vector3d const *forward)
{
	struct item_datum *item = vr_hands_loose(vr_hands.held, _object_mask_item);
	real_vector3d f = *forward, up, left;
	real_point3d position, eye;

	if (!item)
		return;
	if (normalize3d(&f) <= 0.0f)
		f = item->object.forward;
	/* (up: the world's, square to where the hand points; straight up or
	down, the thing's own) */
	cross_product3d(global_up3d, &f, &left);
	if (normalize3d(&left) <= 0.0f)
		cross_product3d(&item->object.up, &f, &left);
	normalize3d(&left);
	cross_product3d(&f, &left, &up);
	normalize3d(&up);
	position.x = point->x - (f.i * vr_hands.held_offset[0] + left.i * vr_hands.held_offset[1] +
		up.i * vr_hands.held_offset[2]);
	position.y = point->y - (f.j * vr_hands.held_offset[0] + left.j * vr_hands.held_offset[1] +
		up.j * vr_hands.held_offset[2]);
	position.z = point->z - (f.k * vr_hands.held_offset[0] + left.k * vr_hands.held_offset[1] +
		up.k * vr_hands.held_offset[2]);
	object_set_position(vr_hands.held, &position, &f, &up);
	unit_get_camera_position(unit_index, &eye);
	object_force_inside_bsp(vr_hands.held, &eye);
	item->object.translational_velocity = *global_zero_vector3d;
	item->object.angular_velocity = *global_zero_vector3d;
	item->item.ignore_object_index = unit_index;
	SET_FLAG(item->object.flags, _object_at_rest_bit, FALSE);
}

/* the prop taken by the hand: its middle from its origin, in its own axes */
static void vr_hands_prop_take(long object_index)
{
	struct item_datum *item = vr_hands_loose(object_index, _object_mask_item);
	real_vector3d offset, left;

	if (!item)
		return;
	vector_from_points3d(&item->object.position, &item->object.bounding_sphere_center, &offset);
	cross_product3d(&item->object.up, &item->object.forward, &left);
	vr_hands.held = object_index;
	vr_hands.held_offset[0] = dot_product3d(&offset, &item->object.forward);
	vr_hands.held_offset[1] = dot_product3d(&offset, &left);
	vr_hands.held_offset[2] = dot_product3d(&offset, &item->object.up);
	vr_hands.history_count = 0;
	vr_hands.history_next = 0;
	object_activate(object_index);
	platform_log("vr hands: holding %s", tag_get_name(item->definition_index));
}

/* the pull's flight, a tick of it: along the line to the hand (point) as
it is now, eased, over it in an arc; arrived, caught (the grip closed) or
dropped. Ported from HaloCE-DecompRemix-VR by moistman42069
(AGPL-3.0-or-later): vr_weapon_pickup.c's flight and catch */
static void vr_hands_pull_fly(boolean closed, real_point3d const *point)
{
	struct item_datum *weapon = vr_hands_loose(vr_hands.pull_target, _object_mask_weapon);
	long now = game_time_get();
	real_point3d position;
	real s, eased;

	if (!weapon)
	{
		vr_hands_pull_end("the weapon in flight was taken or gone");
		return;
	}
	s = (real)(now - vr_hands.pull_start_tick) / (real)vr_hands.pull_ticks;
	if (s >= 1.0f || distance3d(&weapon->object.bounding_sphere_center, point) <= PULL_ARRIVED * vr_hands.units)
	{
		weapon->object.translational_velocity = *global_zero_vector3d;
		if (closed)
		{
			vr_hands.caught = vr_hands.pull_target;
			vr_hands.caught_until = now + VR_GRAB_WAIT_TICKS;
			halo_vr_hand_pulse(0, 0.85f, 0.07f);
			vr_hands_pull_end("caught: picked up once the game offers it");
		}
		else
		{
			vr_hands_pull_end("arrived at an open hand: it drops");
		}
		return;
	}
	s = s < 0.0f ? 0.0f : s;
	eased = s * s * (3.0f - 2.0f * s);
	position.x = vr_hands.pull_start.x + (point->x - vr_hands.pull_start.x) * eased;
	position.y = vr_hands.pull_start.y + (point->y - vr_hands.pull_start.y) * eased;
	position.z = vr_hands.pull_start.z + (point->z - vr_hands.pull_start.z) * eased +
		vr_hands.pull_arc * 4.0f * s * (1.0f - s);
	/* (the weapon's middle along the way, its origin with it) */
	position.x -= weapon->object.bounding_sphere_center.x - weapon->object.position.x;
	position.y -= weapon->object.bounding_sphere_center.y - weapon->object.position.y;
	position.z -= weapon->object.bounding_sphere_center.z - weapon->object.position.z;
	object_set_position(vr_hands.pull_target, &position, NULL, NULL);
	weapon->object.translational_velocity = *global_zero_vector3d;
	weapon->object.angular_velocity = *global_zero_vector3d;
	SET_FLAG(weapon->object.flags, _object_at_rest_bit, FALSE);
}

/* vr_fists.c, before it asks vr.c for the hands each tick: what the hand
without the weapon's grip would take (the pull's candidate, a prop) */
void vr_grab_hands_claims(int claim[2])
{
	claim[0] = vr_hands.pull_state == _vr_pull_idle && vr_hands.held == NONE &&
		(vr_hands.pull_candidate != NONE || vr_hands.prop_candidate != NONE);
	claim[1] = 0;
}

/* vr_fists.c, once a tick: the hands as vr.c has them, and the player's
unit (on foot, alive, in control, in a game of this computer alone; NONE
otherwise) */
void vr_grab_hands_tick(struct halo_vr_hands const *hands, long unit_index)
{
	long now = game_time_get();
	boolean taken = hands->taken[0] != 0;
	boolean pressing = taken && !vr_hands.taken;
	real_point3d point;
	real_vector3d forward, velocity = *global_zero_vector3d;
	real units = 0.0f;
	boolean valid;

	vr_hands_settings_read();
	if (vr_hands.tick != NONE && now < vr_hands.tick)
		vr_hands_reset(NONE);
	vr_hands.tick = now;
	if (unit_index != vr_hands.unit_index)
	{
		vr_hands_reset(vr_hands.unit_index != NONE && unit_try_and_get(vr_hands.unit_index) ? vr_hands.unit_index : NONE);
		vr_hands.unit_index = unit_index;
	}
	vr_hands.taken = taken;
	vr_hands.pull_candidate = NONE;
	vr_hands.prop_candidate = NONE;
	valid = unit_index != NONE && hands->valid[0] &&
		vr_render_hand_world(hands->place[0], hands->forward[0], &point, &forward, &units);
	if (valid)
	{
		vr_hands.units = units;
		vr_render_hand_world(hands->place[0], hands->velocity[0], &point, &velocity, &units);
		normalize3d(&forward);
	}
	if (!valid || (!vr_hands_settings.pull && !vr_hands_settings.props))
	{
		/* (a flight goes on to where the hand was) */
		if (vr_hands.pull_state == _vr_pull_locked || (!valid && vr_hands.pull_state == _vr_pull_flying))
			vr_hands_pull_end(valid ? "turned off" : "the hand lost, or not in play");
		if (vr_hands.held != NONE)
			vr_hands_prop_end(unit_index, FALSE);
		if (!valid)
			return;
	}

	/* the prop held: at the hand while the grip is closed, thrown as it
	opens */
	if (vr_hands.held != NONE)
	{
		if (!vr_hands_loose(vr_hands.held, _object_mask_item))
		{
			platform_log("vr hands: the thing held was taken");
			vr_hands.held = NONE;
		}
		else if (!taken || !vr_hands_settings.props)
		{
			vr_hands_prop_end(unit_index, vr_hands_settings.props);
		}
		else
		{
			vr_hands.history[vr_hands.history_next] = velocity;
			vr_hands.history_next = (short)((vr_hands.history_next + 1) % PROP_HISTORY);
			if (vr_hands.history_count < PROP_HISTORY)
				vr_hands.history_count++;
			vr_hands_prop_hold(unit_index, &point, &forward);
			return;
		}
	}

	/* the pull: locked on, a flick throws it to the hand; flying */
	if (vr_hands.pull_state == _vr_pull_flying)
	{
		vr_hands_pull_fly(taken, &point);
		return;
	}
	if (vr_hands.pull_state == _vr_pull_locked)
	{
		struct item_datum *weapon = vr_hands_loose(vr_hands.pull_target, _object_mask_weapon);
		real_vector3d away;
		real speed, distance;

		if (!weapon)
		{
			vr_hands_pull_end("the weapon was taken or gone");
		}
		else if (!taken)
		{
			vr_hands_pull_end("the grip let go before the pull");
		}
		else
		{
			/* (a flick: the hand yanked away from the weapon, toward you,
			or up) */
			vector_from_points3d(&weapon->object.bounding_sphere_center, &point, &away);
			if (normalize3d(&away) <= 0.0f)
				away = *global_up3d;
			speed = dot_product3d(&velocity, &away);
			if (velocity.k > speed)
				speed = velocity.k;
			if (speed >= PULL_FLICK_SPEED)
			{
				vr_hands.pull_state = _vr_pull_flying;
				vr_hands.pull_start = weapon->object.bounding_sphere_center;
				vr_hands.pull_start_tick = now;
				distance = distance3d(&vr_hands.pull_start, &point) / units;
				vr_hands.pull_ticks = (long)((PIN(PULL_FLIGHT_BASE + distance / PULL_FLIGHT_SPEED,
					PULL_FLIGHT_SHORTEST, PULL_FLIGHT_LONGEST)) * TICKS_PER_SECOND + 0.999f);
				if (vr_hands.pull_ticks < 1)
					vr_hands.pull_ticks = 1;
				vr_hands.pull_arc = ((distance * 0.25f < PULL_ARC_HIGHEST ? distance * 0.25f : PULL_ARC_HIGHEST) +
					PULL_ARC_LEAST) * units;
				object_activate(vr_hands.pull_target);
				halo_vr_hand_pulse(0, 0.55f, 0.05f);
				platform_log("vr hands: pull: %s flies to the hand (%.1f m, %.2f s)",
					tag_get_name(weapon->definition_index), (double)distance,
					(double)vr_hands.pull_ticks / TICKS_PER_SECOND);
			}
		}
		return;
	}

	/* the grip closed on what the hand was claimed for (vr.c took it so):
	a prop held, else a weapon locked on */
	if (pressing && vr_hands_settings.props && vr_hands.held == NONE)
	{
		long prop = vr_hands_prop_candidate(unit_index, &point);

		if (prop != NONE)
		{
			vr_hands_prop_take(prop);
			vr_hands_prop_hold(unit_index, &point, &forward);
			return;
		}
	}
	if (pressing && vr_hands_settings.pull)
	{
		long weapon_index = vr_hands_pull_candidate(unit_index, &point, &forward);

		if (weapon_index != NONE)
		{
			vr_hands.pull_state = _vr_pull_locked;
			vr_hands.pull_target = weapon_index;
			platform_log("vr hands: pull: locked on %s (%.1f m away): flick the hand back",
				tag_get_name(object_get(weapon_index)->definition_index),
				(double)(distance3d(&point, &object_get(weapon_index)->object.bounding_sphere_center) / units));
			return;
		}
	}
	/* (what the hand's grip would take next: vr_grab_hands_claims) */
	if (vr_hands_settings.props && !taken)
		vr_hands.prop_candidate = vr_hands_prop_candidate(unit_index, &point);
	if (vr_hands_settings.pull && !taken && vr_hands.prop_candidate == NONE)
	{
		real_point3d eye;

		unit_get_camera_position(unit_index, &eye);
		if (distance3d(&eye, &point) >= PULL_ARM_OUT * units)
			vr_hands.pull_candidate = vr_hands_pull_candidate(unit_index, &point, &forward);
	}
}

/* the thing the hand without the weapon is at or points at, lit up as the
grab's is (lighting set; FALSE, not one) */
static boolean vr_hands_highlight(long object_index, struct render_lighting const *given,
	struct render_lighting *lighting, struct render_model_effect *model_effect)
{
	long unit_index = vr_hands.unit_index;
	real distance;

	if (object_index == NONE || !given || unit_index == NONE || vr_hands.units <= 0.0f ||
		!object_try_and_get_and_verify_type(unit_index, _object_mask_unit))
	{
		return FALSE;
	}
	if (object_index == vr_hands.pull_target)
		distance = 0.0f;
	else if (object_index == vr_hands.pull_candidate || object_index == vr_hands.prop_candidate)
		distance = 0.15f;
	else
		return FALSE;
	*lighting = *given;
	vr_holster_highlight_object(unit_index, distance, vr_hands.units, model_effect, lighting);
	return TRUE;
}

/* players.c, as the game offers a player something to do with the action
button (player_set_action_result), and would offer it (set_action): the
weapons offered to the first local player kept, and among them the one its
right hand reaches for offered first (TRUE), whatever is nearer the body.
Not as a network game's client, whose host decides pick-ups by the body. */
boolean vr_grab_offer(long player_index, short action_result, long object_index, boolean set_action)
{
	struct player_datum *player = player_get(player_index);
	long now = game_time_get();

	if (!vr_grab_weapon_result(action_result) || player_index != local_player_get_player_index(0) ||
		network_game_distributed_client())
	{
		return set_action;
	}
	if (vr_grab.offered_tick != now)
	{
		vr_grab.offered_tick = now;
		vr_grab.offered_count = 0;
	}
	if (vr_grab.offered_count < HALO_VR_GRAB_THINGS)
		vr_grab.offered[vr_grab.offered_count++] = object_index;
	/* (OpenFrame: the weapon the pull caught, offered first) */
	if (vr_hands.caught != NONE && action_result == player->action_result && player->action_object_index != NONE)
	{
		if (object_index == vr_hands.caught)
			return TRUE;
		if (player->action_object_index == vr_hands.caught)
			return FALSE;
	}
	if (!vr_grab.enabled || vr_grab.nearest == NONE || vr_grab.distance >= VR_GRAB_PREFER_DISTANCE ||
		action_result != player->action_result || player->action_object_index == NONE)
	{
		return set_action;
	}
	if (object_index == vr_grab.nearest)
		return TRUE;
	if (player->action_object_index == vr_grab.nearest)
		return FALSE;
	return set_action;
}

/* a weapon on the ground as a thing to grab: the line along it, through
its middle */
static boolean vr_grab_weapon_thing(long weapon_index, struct halo_vr_grab_thing *thing)
{
	struct object_datum *weapon = object_try_and_get(weapon_index);
	real_point3d ends[2];
	real half;
	short end;

	if (!weapon || weapon->object.parent_object_index != NONE)
		return FALSE;
	half = weapon->object.bounding_sphere_radius * VR_GRAB_WEAPON_LENGTH;
	for (end = 0; end < 2; end++)
	{
		real along = end ? half : -half;

		ends[end].x = weapon->object.bounding_sphere_center.x + weapon->object.forward.i * along;
		ends[end].y = weapon->object.bounding_sphere_center.y + weapon->object.forward.j * along;
		ends[end].z = weapon->object.bounding_sphere_center.z + weapon->object.forward.k * along;
		if (!vr_render_world_place(&ends[end], thing->ends[end]))
			return FALSE;
	}
	thing->kind = HALO_VR_GRAB_WEAPON;
	thing->object = weapon_index;
	thing->radius = VR_GRAB_WEAPON_THICKNESS * 0.5f;
	return TRUE;
}

/* a control the game would let the player use (in reach of the body, able
to change, facing the eye) as a thing to press: its face (its "front"
marker, and which way it faces: toward the eye, for one used from either
side), else its middle */
static boolean vr_grab_control_thing(long unit_index, long control_index, struct halo_vr_grab_thing *thing)
{
	struct control_datum *control = control_try_and_get(control_index);
	real units = vr_grab.units > 0.0f ? vr_grab.units : 1.0f / 3.048f;
	struct object_marker marker;
	real_point3d camera, face;
	real_vector3d toward;
	real radius;
	short axis;

	if (!control || !device_can_change_position(control_index))
		return FALSE;
	unit_get_camera_position(unit_index, &camera);
	vector_from_points3d(&camera, &control->object.bounding_sphere_center, &toward);
	if (normalize3d(&toward) <= 0.0f || !device_frontfacing(control_index, &camera, &toward))
		return FALSE;
	face = control->object.bounding_sphere_center;
	thing->front[0] = thing->front[1] = thing->front[2] = 0.0f;
	if (object_get_marker_by_name(control_index, "front", &marker, 1) == 1)
	{
		real_vector3d front = marker.matrix.forward;
		real_point3d ahead;
		float place[3];

		face = marker.matrix.position;
		if (dot_product3d(&front, &toward) > 0.0f)
			scale_vector3d(&front, -1.0f, &front);
		ahead.x = face.x + front.i * units;
		ahead.y = face.y + front.j * units;
		ahead.z = face.z + front.k * units;
		if (!vr_render_world_place(&ahead, place))
			return FALSE;
		for (axis = 0; axis < 3; axis++)
			thing->front[axis] = place[axis];
	}
	if (!vr_render_world_place(&face, thing->ends[0]))
		return FALSE;
	for (axis = 0; axis < 3; axis++)
	{
		thing->ends[1][axis] = thing->ends[0][axis];
		if (thing->front[0] != 0.0f || thing->front[1] != 0.0f || thing->front[2] != 0.0f)
			thing->front[axis] -= thing->ends[0][axis];
	}
	radius = control->object.bounding_sphere_radius / units * 0.5f;
	thing->kind = HALO_VR_GRAB_CONTROL;
	thing->object = control_index;
	thing->radius = PIN(radius, VR_GRAB_CONTROL_SMALLEST, VR_GRAB_CONTROL_LARGEST);
	return TRUE;
}

/* player_control.c, each frame, for the first local player's unit: while a
control a hand pressed waits for the game to offer it, and as its action is
pressed, the facing (TRUE, yaw and pitch) at it from the eye, as X's aim
would be, so that the game offers it, as a network game's host does for a
client (the host decides the controls a player uses, by the player's aim) */
boolean vr_grab_facing(long unit_index, real *yaw, real *pitch)
{
	long now = game_time_get();
	struct object_datum *control;
	real_point3d camera;
	real_vector3d toward;

	if (vr_grab.facing_until == NONE || now >= vr_grab.facing_until ||
		now < vr_grab.facing_until - VR_GRAB_CONTROL_WAIT_TICKS - VR_GRAB_ACTION_TICKS - 1 || unit_index == NONE ||
		vr_grab.control == NONE)
	{
		return FALSE;
	}
	control = (struct object_datum *)object_try_and_get_and_verify_type(vr_grab.control, _object_mask_control);
	if (!control || !object_try_and_get_and_verify_type(unit_index, _object_mask_unit))
		return FALSE;
	unit_get_camera_position(unit_index, &camera);
	vector_from_points3d(&camera, &control->object.bounding_sphere_center, &toward);
	if (normalize3d(&toward) <= 0.0f)
		return FALSE;
	*yaw = (real)atan2(toward.j, toward.i);
	*pitch = (real)asin(PIN(toward.k, -1.0f, 1.0f));
	return TRUE;
}

/* player_control.c, as the first local player's input is read (the
player, and the flags the input has set): the things to grab told to vr.c,
and the grip's weapon picked up, the game's pick-up pressed for it (the swap
bit, as X held) once the game offers it */
void vr_grab_input(long player_index, unsigned long *unit_control_flags)
{
	struct player_datum *player = player_get(player_index);
	struct unit_datum *unit = vr_grab_unit(player_index);
	boolean weapon_offered = unit && vr_grab_weapon_result(player->action_result) &&
		player->action_object_index != NONE;
	long now = game_time_get();
	struct halo_vr_grab grab;
	short index;

	vr_grab_test(player_index, halo_vr_aiming() || vr_grab.enabled);
	memset(&grab, 0, sizeof(grab));
	if (unit)
	{
		/* (the weapons the game offered at its last tick, else the one it
		offers: a network game's client's, the host's) */
		if (!network_game_distributed_client() && vr_grab.offered_tick != NONE && now - vr_grab.offered_tick <= 1 &&
			now >= vr_grab.offered_tick)
		{
			for (index = 0; index < vr_grab.offered_count && grab.count < HALO_VR_GRAB_THINGS; index++)
			{
				if (vr_grab_weapon_thing(vr_grab.offered[index], &grab.things[grab.count]))
					grab.count++;
			}
		}
		else if (weapon_offered && vr_grab_weapon_thing(player->action_object_index, &grab.things[0]))
		{
			grab.count = 1;
		}
		/* (the controls in reach of the body, as the game finds them:
		player_examine_nearby_objects) */
		{
			long controls[HALO_VR_GRAB_THINGS];
			short count = objects_in_sphere(0, _object_mask_control, &unit->object.location,
				&unit->object.bounding_sphere_center, unit->object.bounding_sphere_radius, controls, NUMBEROF(controls));

			for (index = 0; index < count && grab.count < HALO_VR_GRAB_THINGS; index++)
			{
				if (vr_grab_control_thing(player->unit_index, controls[index], &grab.things[grab.count]))
					grab.count++;
			}
		}
	}
	if (!halo_vr_grab(&grab))
	{
		vr_grab.enabled = FALSE;
		vr_grab.nearest = NONE;
		vr_grab.control = NONE;
		return;
	}
	vr_grab.enabled = grab.enabled && unit;
	vr_grab.nearest = vr_grab.enabled && grab.nearest != -1 ? grab.nearest : NONE;
	vr_grab.distance = grab.distance;
	vr_grab.units = grab.world_units_per_metre;
	if (grab.taken != -1 && vr_grab.enabled)
	{
		vr_grab.taken = grab.taken;
		vr_grab.taken_until = now + VR_GRAB_WAIT_TICKS;
	}
	/* (the grip's weapon, once the game offers it: picked up) */
	if (vr_grab.taken != NONE)
	{
		if (!vr_grab.enabled || now > vr_grab.taken_until || now < vr_grab.taken_until - VR_GRAB_WAIT_TICKS)
		{
			platform_log("vr grab: the weapon gripped was not offered: nothing taken");
			vr_grab.taken = NONE;
		}
		else if (weapon_offered && player->action_object_index == vr_grab.taken)
		{
			platform_log("vr grab: %s %s", player->action_result == _player_action_result_swap_for_weapon ?
				"swapped for" : "picked up", tag_get_name(object_get(vr_grab.taken)->definition_index));
			vr_grab.press_until = now + VR_GRAB_PRESS_TICKS;
			vr_grab.taken = NONE;
		}
	}
	/* (OpenFrame: the weapon the pull caught, once the game offers it:
	picked up as the grip's is) */
	if (vr_hands.caught != NONE)
	{
		if (!unit || now > vr_hands.caught_until || now < vr_hands.caught_until - VR_GRAB_WAIT_TICKS)
		{
			platform_log("vr hands: pull: the weapon caught was not offered (one like it carried?): it drops");
			vr_hands.caught = NONE;
		}
		else if (weapon_offered && player->action_object_index == vr_hands.caught)
		{
			platform_log("vr hands: pull: %s %s", player->action_result == _player_action_result_swap_for_weapon ?
				"swapped for" : "picked up", tag_get_name(object_get(vr_hands.caught)->definition_index));
			vr_grab.press_until = now + VR_GRAB_PRESS_TICKS;
			vr_hands.caught = NONE;
		}
	}
	if (vr_grab.press_until != NONE && (now >= vr_grab.press_until || now < vr_grab.press_until - VR_GRAB_PRESS_TICKS))
		vr_grab.press_until = NONE;
	if (vr_grab.press_until != NONE)
		SET_FLAG(*unit_control_flags, _unit_control_swap_weapons_bit, TRUE);

	/* (the control a hand pressed: the aim turned to it, vr_grab_facing,
	until the game offers it, then its action) */
	if (grab.pressed != -1 && vr_grab.enabled && vr_grab.action_until == NONE)
	{
		if (player->action_result != _player_action_result_touch_device || player->action_object_index != grab.pressed)
			platform_log("vr grab: the control pressed is not on offer: the aim turned to it");
		vr_grab.control = grab.pressed;
		vr_grab.control_until = now + VR_GRAB_CONTROL_WAIT_TICKS;
		vr_grab.facing_until = vr_grab.control_until + VR_GRAB_ACTION_TICKS + 1;
	}
	if (vr_grab.action_until != NONE && (now >= vr_grab.action_until ||
		now < vr_grab.action_until - VR_GRAB_ACTION_TICKS))
	{
		vr_grab.action_until = NONE;
		vr_grab.control = NONE;
	}
	if (vr_grab.control != NONE && vr_grab.action_until == NONE)
	{
		if (!vr_grab.enabled || now > vr_grab.control_until || now < vr_grab.control_until - VR_GRAB_CONTROL_WAIT_TICKS)
		{
			platform_log("vr grab: the control pressed was not offered: nothing done");
			vr_grab.control = NONE;
			vr_grab.facing_until = NONE;
		}
		else if (unit && player->action_result == _player_action_result_touch_device &&
			player->action_object_index == vr_grab.control)
		{
			platform_log("vr grab: pressed %s", tag_get_name(object_get(vr_grab.control)->definition_index));
			vr_grab.action_until = now + VR_GRAB_ACTION_TICKS;
			vr_grab.facing_until = vr_grab.action_until + 1;
		}
	}
	if (vr_grab.action_until != NONE && player->action_result == _player_action_result_touch_device &&
		player->action_object_index == vr_grab.control)
	{
		SET_FLAG(*unit_control_flags, _unit_control_action_bit, TRUE);
	}
}

/* render_objects.c, as an object is drawn in the world (its light given):
the weapon the right hand reaches for lit up as the holsters' weapons are
(lighting set to the light to draw it with; FALSE, as it is) */
boolean vr_grab_highlight(long object_index, struct render_lighting const *given, struct render_lighting *lighting,
	struct render_model_effect *model_effect)
{
	long player_index = local_player_get_player_index(0);
	long unit_index;
	extern boolean vr_climb_highlight(long object_index, struct render_lighting const *given,
		struct render_lighting *lighting, struct render_model_effect *model_effect);

	/* (a vehicle a hand comes to a hold of: vr_climb.c) */
	if (halo_vr_running() && vr_climb_highlight(object_index, given, lighting, model_effect))
		return TRUE;
	/* (OpenFrame: a weapon to pull, a thing to hold) */
	if (halo_vr_running() && vr_hands_highlight(object_index, given, lighting, model_effect))
		return TRUE;
	if (object_index == NONE || object_index != vr_grab.nearest || !vr_grab.enabled || !given || player_index == NONE ||
		!halo_vr_running())
	{
		return FALSE;
	}
	unit_index = player_get(player_index)->unit_index;
	if (unit_index == NONE || !object_try_and_get_and_verify_type(unit_index, _object_mask_unit))
		return FALSE;
	*lighting = *given;
	vr_holster_highlight_object(unit_index, vr_grab.distance, vr_grab.units, model_effect, lighting);
	return TRUE;
}

#endif
