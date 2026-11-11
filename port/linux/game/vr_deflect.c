/*
VR_DEFLECT.C

The VR mode's shot deflection (HALO_VR; vr.deflect, vr.deflect_strength,
vr.deflect_assist; docs/openframe-features.md): a shot met by the gun in
the hand, or by a swung fist, turns away.

- The gun is a body: a capsule from a little behind the grip along the
  controller's aim, as long as the weapon's model. Held still in a shot's
  path it blocks it: the shot glances off its surface (mirrored about the
  surface there, a little slower). Swung past DEFLECT_SWING_SPEED it bats
  the shot along the swing, at the shot's speed or the swing's, whichever
  is faster.
- A free hand (the one without the weapon, not on the foregrip or holding
  anything; the weapon's hand while it holds none) swung past
  DEFLECT_SWING_SPEED bats a shot it meets along the swing.
- By kind, from the projectile's tag (never its name): bolts (plasma,
  needles, vehicles' bolts) by the gun or a fist; bullets only off the gun;
  grenades batted or bounced; rockets and fuel rods only by a hard swing
  (DEFLECT_HARD_SWING_SPEED), never a block; tank shells, mortars, beams and
  flame never.
- A deflected shot is the player's: its damage and kills, its owner and
  team; it can strike the one who fired it, and no longer the player. Each
  shot is deflected once.

projectiles.c asks it in each pass of a projectile's travel, after the test
for what the pass strikes and before the strike (vr_deflect_projectile);
vr_fists.c's hands' tick gives it the hands once a tick (vr_deflect_tick).
Nothing is sent and nothing kept in the game state but the shot's own
velocity and owner: a game of this computer alone (game_connection local:
the campaign; never online, split screen or with bots), the first local
player on foot, alive, in control, the controllers aiming. The hand that
met it pulses.

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
vr_projectile_parry.c's sorting of projectiles by their tags' data
(deflect_category), what may deflect each kind (deflect_allowed), the gun
as a capsule (gun_contact), the block's mirrored velocity and what it keeps
of the speed by kind, the swing's velocity, and DEFLECT AID and DEFLECT
FORCE (vr.deflect_assist, vr.deflect_strength), made again on this port's
hands (vr.c's halo_vr_hands) and projectile update.
*/

#ifdef HALO_VR

#include "cseries.h"
#include "math/real_math.h"
#include "game/game.h"
#include "game/players.h"
#include "items/projectile_definitions.h"
#include "items/projectiles.h"
#include "objects/objects.h"
#include "units/units.h"

#include "../src/vr.h"

#include <math.h>
#include <string.h>

/* the platform layer's (port/linux/src/port_config.c) */
int config_boolean(char const *name);
double config_real(char const *name);
unsigned long config_changes(void);
void platform_log(char const *format, ...);
/* vr_render.c's: a hand's place in the world */
boolean vr_render_hand_world(const float place[3], const float direction[3], real_point3d *point,
	real_vector3d *world_direction, real *units);

/* a swing (metres a second): from DEFLECT_SWING_SPEED it bats a shot along
it, from DEFLECT_HARD_SWING_SPEED an explosive too; faster than
DEFLECT_SWING_FASTEST is the tracking's jump. A hand's path over a tick
longer than DEFLECT_PATH_LONGEST (metres) is a jump too */
#define DEFLECT_SWING_SPEED 1.2f
#define DEFLECT_HARD_SWING_SPEED 2.5f
#define DEFLECT_SWING_FASTEST 15.0f
#define DEFLECT_PATH_LONGEST 0.5f
/* the fist's radius and the gun's (metres), before the shot's own radius
(at most DEFLECT_SHOT_RADIUS) and vr.deflect_assist */
#define DEFLECT_FIST_RADIUS 0.05f
#define DEFLECT_GUN_RADIUS 0.04f
#define DEFLECT_SHOT_RADIUS 0.12f
/* the gun's length (metres) behind the grip and ahead of it, from the
weapon's model's radius, and between what bounds; its radius when it has
none */
#define DEFLECT_GUN_BEHIND 0.4f
#define DEFLECT_GUN_BEHIND_LEAST 0.06f
#define DEFLECT_GUN_BEHIND_MOST 0.25f
#define DEFLECT_GUN_AHEAD 1.5f
#define DEFLECT_GUN_AHEAD_LEAST 0.25f
#define DEFLECT_GUN_AHEAD_MOST 0.75f
#define DEFLECT_GUN_SIZE 0.3f

enum
{
	_deflect_never = 0,
	_deflect_bolt,
	_deflect_bullet,
	_deflect_grenade,
	_deflect_explosive,
};
static char const *const deflect_kind_names[] = { "never", "bolt", "bullet", "grenade", "explosive" };

enum
{
	_deflect_by_fist = 0,
	_deflect_by_swing,
	_deflect_by_block,
};
static char const *const deflect_way_names[] = { "fist", "gun's swing", "gun's block" };

static struct
{
	unsigned long changes;
	boolean on;
	real strength;
	real assist;
} deflect_settings = { (unsigned long)-1, FALSE, 1.0f, 0.03f };

static struct
{
	/* the tick the hands were last given at (NONE: none), the player's unit
	(NONE: no deflection), the world's units a metre */
	long tick;
	long unit_index;
	real units;
	/* each hand (1 the weapon's, 0 the other): its path over the last tick
	in the world, whether it may meet a shot as a fist, and its swing (the
	world's direction, normalized, and its speed, metres a second; zero:
	no swing) */
	boolean valid[2];
	real_point3d from[2];
	real_point3d to[2];
	boolean fist[2];
	real_vector3d stroke[2];
	real stroke_speed[2];
	/* the gun in the weapon's hand: its back and front along its aim */
	boolean gun;
	real_point3d gun_back;
	real_point3d gun_front;
	/* the shots deflected, each once: the whole index (its salt with its
	slot) at its slot */
	long deflected[MAXIMUM_OBJECTS_PER_MAP];
	long log_tick;
} deflect;
static boolean deflect_initialized = FALSE;

static void deflect_settings_read(void)
{
	if (deflect_settings.changes == config_changes())
		return;
	deflect_settings.changes = config_changes();
	deflect_settings.on = config_boolean("vr.deflect") != 0;
	deflect_settings.strength = (real)config_real("vr.deflect_strength");
	if (!(deflect_settings.strength >= 0.5f && deflect_settings.strength <= 1.5f))
		deflect_settings.strength = 1.0f;
	deflect_settings.assist = (real)config_real("vr.deflect_assist");
	if (!(deflect_settings.assist >= 0.0f && deflect_settings.assist <= 0.1f))
		deflect_settings.assist = 0.03f;
}

static void deflect_reset(void)
{
	long index;

	csmemset(&deflect, 0, sizeof(deflect));
	deflect.tick = NONE;
	deflect.unit_index = NONE;
	deflect.log_tick = NONE;
	for (index = 0; index < MAXIMUM_OBJECTS_PER_MAP; index++)
		deflect.deflected[index] = NONE;
	deflect_initialized = TRUE;
}

/* ---------- the hands */

/* vr_fists.c's hands' tick, once a tick: the hands as vr.c gave them, and
the player's unit (NONE: no deflection) */
void vr_deflect_tick(struct halo_vr_hands const *hands, long unit_index)
{
	long now = game_time_get();
	boolean following;
	struct unit_datum *unit;
	long weapon_index;
	short hand;

	if (!deflect_initialized)
		deflect_reset();
	/* (the game's time gone back: a revert, a map begun) */
	if (deflect.tick != NONE && now < deflect.tick)
		deflect_reset();
	deflect_settings_read();
	following = deflect.tick != NONE && now == deflect.tick + 1 && unit_index == deflect.unit_index;
	deflect.tick = now;
	deflect.unit_index = deflect_settings.on ? unit_index : NONE;
	deflect.gun = FALSE;
	if (deflect.unit_index == NONE)
	{
		deflect.valid[0] = deflect.valid[1] = FALSE;
		return;
	}
	unit = unit_get(unit_index);
	weapon_index = unit->unit.current_weapon_index != NONE ?
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] : NONE;
	for (hand = 0; hand < 2; hand++)
	{
		real_point3d point, unused;
		real_vector3d forward, velocity;
		real units, speed, travel;

		if (!hands->valid[hand] ||
			!vr_render_hand_world(hands->place[hand], hands->forward[hand], &point, &forward, &units) ||
			!vr_render_hand_world(hands->place[hand], hands->velocity[hand], &unused, &velocity, &units) ||
			!(units > 0.0f))
		{
			deflect.valid[hand] = FALSE;
			continue;
		}
		deflect.units = units;
		deflect.from[hand] = following && deflect.valid[hand] ? deflect.to[hand] : point;
		deflect.to[hand] = point;
		deflect.valid[hand] = TRUE;
		travel = distance3d(&deflect.from[hand], &deflect.to[hand]);
		if (travel > DEFLECT_PATH_LONGEST * units)
			deflect.from[hand] = point;
		/* (the hand's own speed, as the room has it: not the body's moving
		or turning) */
		speed = magnitude3d(&velocity);
		deflect.stroke_speed[hand] = 0.0f;
		if (speed >= DEFLECT_SWING_SPEED && speed <= DEFLECT_SWING_FASTEST && normalize3d(&velocity) > 0.0f)
		{
			deflect.stroke[hand] = velocity;
			deflect.stroke_speed[hand] = speed;
		}
		deflect.fist[hand] = hands->free[hand] != 0;
		/* the gun in the weapon's hand: from a little behind the grip along
		the aim, as long as its model */
		if (hand == 1 && weapon_index != NONE && normalize3d(&forward) > 0.0f)
		{
			struct object_datum *weapon = object_try_and_get(weapon_index);
			real size = weapon && weapon->object.bounding_sphere_radius > 0.0f ?
				weapon->object.bounding_sphere_radius / units : DEFLECT_GUN_SIZE;
			real behind = PIN(DEFLECT_GUN_BEHIND * size, DEFLECT_GUN_BEHIND_LEAST, DEFLECT_GUN_BEHIND_MOST);
			real ahead = PIN(DEFLECT_GUN_AHEAD * size, DEFLECT_GUN_AHEAD_LEAST, DEFLECT_GUN_AHEAD_MOST);

			point_from_line3d(&point, &forward, -behind * units, &deflect.gun_back);
			point_from_line3d(&point, &forward, ahead * units, &deflect.gun_front);
			deflect.gun = TRUE;
			deflect.fist[hand] = FALSE;
		}
	}
}

/* ---------- the shots */

/* what a projectile is, from its tag's own data. Ported from
HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
vr_projectile_parry.c's deflect_category */
static short deflect_kind(struct projectile_definition const *definition)
{
	real speed = definition->projectile.initial_velocity;
	real gravity = definition->projectile.air_gravity_scale;
	real range = definition->projectile.maximum_range;
	boolean impact = definition->projectile.impact_damage.index != NONE;
	boolean detonates = definition->projectile.effect.index != NONE;

	if (!isfinite(speed) || !isfinite(gravity) || !isfinite(range))
		return _deflect_never;
	/* (flame and cinematic effects live a few units only) */
	if (range > 0.0f && range < 10.0f)
		return _deflect_never;
	if (definition->projectile.timer_upper_bound > 0.0f && gravity > 0.0f && !impact)
		return _deflect_grenade;
	if (impact)
		return speed >= 3.0f ? _deflect_bullet : speed > 0.01f ? _deflect_bolt : _deflect_never;
	if (detonates)
	{
		/* (shells, mortars, beams) */
		if (speed >= 2.0f || definition->projectile.collision_radius >= 0.3f)
			return _deflect_never;
		return speed > 0.01f || gravity > 0.0f ? _deflect_explosive : _deflect_never;
	}
	return _deflect_never;
}

/* whether a kind is deflected that way, by a swing of that speed (metres a
second). Ported from HaloCE-DecompRemix-VR by moistman42069
(AGPL-3.0-or-later): vr_projectile_parry.c's deflect_allowed */
static boolean deflect_allowed(short kind, short way, real speed)
{
	switch (kind)
	{
	case _deflect_bolt:
	case _deflect_grenade:
		return TRUE;
	case _deflect_bullet:
		return way != _deflect_by_fist;
	case _deflect_explosive:
		return way != _deflect_by_block && speed >= DEFLECT_HARD_SWING_SPEED;
	default:
		return FALSE;
	}
}

/* the nearest points of two segments (p0 p1, q0 q1): where along the
first (0 to 1) and the point on the second, and their distance: Ericson's
closest points of two segments */
static real deflect_segments(real_point3d const *p0, real_point3d const *p1, real_point3d const *q0,
	real_point3d const *q1, real *along_p, real_point3d *on_q)
{
	real_vector3d d1, d2, r;
	real a, e, f, s = 0.0f, t = 0.0f;
	real_point3d on_p;

	vector_from_points3d(p0, p1, &d1);
	vector_from_points3d(q0, q1, &d2);
	vector_from_points3d(q0, p0, &r);
	a = dot_product3d(&d1, &d1);
	e = dot_product3d(&d2, &d2);
	f = dot_product3d(&d2, &r);
	if (a <= 1e-9f && e <= 1e-9f)
	{
		s = t = 0.0f;
	}
	else if (a <= 1e-9f)
	{
		t = PIN(f / e, 0.0f, 1.0f);
	}
	else
	{
		real c = dot_product3d(&d1, &r);

		if (e <= 1e-9f)
		{
			s = PIN(-c / a, 0.0f, 1.0f);
		}
		else
		{
			real b = dot_product3d(&d1, &d2);
			real denominator = a * e - b * b;

			s = denominator != 0.0f ? PIN((b * f - c * e) / denominator, 0.0f, 1.0f) : 0.0f;
			t = (b * s + f) / e;
			if (t < 0.0f)
			{
				t = 0.0f;
				s = PIN(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = PIN((b - c) / a, 0.0f, 1.0f);
			}
		}
	}
	point_from_line3d(p0, &d1, s, &on_p);
	point_from_line3d(q0, &d2, t, on_q);
	*along_p = s;
	return distance3d(&on_p, on_q);
}

/* where along a shot's path (start to end, 0 to 1) it first meets a body
about a segment (q0 q1) `radius` round, and the body's point nearest it;
FALSE: it does not */
static boolean deflect_contact(real_point3d const *start, real_point3d const *end, real_point3d const *q0,
	real_point3d const *q1, real radius, real *at, real_point3d *on_body)
{
	real along, distance, length = distance3d(start, end);

	distance = deflect_segments(start, end, q0, q1, &along, on_body);
	if (!(distance <= radius) || !(length > 0.0f))
		return FALSE;
	/* (back along the path to where its edge first touched) */
	along -= (real)sqrt(MAX(radius * radius - distance * distance, 0.0f)) / length;
	*at = PIN(along, 0.0f, 1.0f);
	return TRUE;
}

/* projectiles.c, in a pass of a projectile's travel that may still deflect
(flying on its own, not yet acting: the projectiles' flags are its): its
path this pass (from its place to end), its velocity at the pass's start
and end (the world's units a tick), and how far along the path it strikes
what it strikes (1: nothing). TRUE: deflected, where along the path
(*fraction), its velocity, owner and orientation set */
boolean vr_deflect_projectile(long projectile_index, real_point3d const *end, real_vector3d const *end_velocity,
	real solid_fraction, real *fraction)
{
	struct projectile_datum *projectile;
	struct projectile_definition const *definition;
	struct unit_datum *unit;
	long unit_index = deflect.unit_index;
	long now = game_time_get();
	long slot = DATUM_INDEX_TO_ABSOLUTE_INDEX(projectile_index);
	real units = deflect.units;
	real best = 2.0f, shot_radius, assist, speed;
	real_point3d best_on_body, contact;
	real_vector3d incoming, reflected, direction;
	short kind, hand, best_hand = NONE, best_way = _deflect_by_fist;

	if (!deflect_initialized || unit_index == NONE || deflect.tick == NONE || now - deflect.tick > 1 ||
		now < deflect.tick || !(units > 0.0f) || game_connection() != _game_connection_local ||
		slot < 0 || slot >= MAXIMUM_OBJECTS_PER_MAP || deflect.deflected[slot] == projectile_index)
	{
		return FALSE;
	}
	unit = unit_try_and_get(unit_index);
	projectile = (struct projectile_datum *)object_try_and_get_and_verify_type(projectile_index,
		_object_mask_projectile);
	if (!unit || !projectile || projectile->object.owner_object_index == unit_index ||
		projectile->object.parent_object_index != NONE || !(projectile->projectile.detonation_timer < 1.0f))
	{
		return FALSE;
	}
	definition = projectile_definition_get(projectile->definition_index);
	kind = deflect_kind(definition);
	if (kind == _deflect_never)
		return FALSE;
	shot_radius = definition->projectile.collision_radius;
	if (!(shot_radius >= 0.0f))
		return FALSE;
	shot_radius = MIN(shot_radius, DEFLECT_SHOT_RADIUS * units);
	assist = deflect_settings.assist * units;
	for (hand = 0; hand < 2; hand++)
	{
		real at;
		real_point3d on_body;
		short way;

		if (!deflect.valid[hand])
			continue;
		if (hand == 1 && deflect.gun)
		{
			way = deflect.stroke_speed[hand] > 0.0f ? _deflect_by_swing : _deflect_by_block;
			if (!deflect_contact(&projectile->object.position, end, &deflect.gun_back, &deflect.gun_front,
				DEFLECT_GUN_RADIUS * units + shot_radius + assist, &at, &on_body))
			{
				continue;
			}
		}
		else
		{
			way = _deflect_by_fist;
			if (!deflect.fist[hand] || !(deflect.stroke_speed[hand] > 0.0f) ||
				!deflect_contact(&projectile->object.position, end, &deflect.from[hand], &deflect.to[hand],
					DEFLECT_FIST_RADIUS * units + shot_radius + assist, &at, &on_body))
			{
				continue;
			}
		}
		if (!deflect_allowed(kind, way, deflect.stroke_speed[hand]) || at >= solid_fraction || at >= best)
			continue;
		best = at;
		best_hand = hand;
		best_way = way;
		best_on_body = on_body;
	}
	if (best_hand == NONE)
		return FALSE;

	/* its velocity where it met the hand, and where that was */
	incoming.i = projectile->object.translational_velocity.i +
		(end_velocity->i - projectile->object.translational_velocity.i) * best;
	incoming.j = projectile->object.translational_velocity.j +
		(end_velocity->j - projectile->object.translational_velocity.j) * best;
	incoming.k = projectile->object.translational_velocity.k +
		(end_velocity->k - projectile->object.translational_velocity.k) * best;
	contact.x = projectile->object.position.x + (end->x - projectile->object.position.x) * best;
	contact.y = projectile->object.position.y + (end->y - projectile->object.position.y) * best;
	contact.z = projectile->object.position.z + (end->z - projectile->object.position.z) * best;
	speed = magnitude3d(&incoming);
	if (!isfinite(speed) || !valid_real_point3d(&contact))
		return FALSE;
	if (best_way == _deflect_by_block)
	{
		/* glanced off the gun's surface: mirrored about its normal there
		(from the gun's axis to the contact), a grenade bouncing slower than
		a bolt or a round. Ported from HaloCE-DecompRemix-VR by moistman42069
		(AGPL-3.0-or-later): vr_projectile_parry.c's block */
		real_vector3d normal;
		real along;
		real keep = kind == _deflect_grenade ? 0.45f : kind == _deflect_bullet ? 0.70f : 0.85f;

		vector_from_points3d(&best_on_body, &contact, &normal);
		if (normalize3d(&normal) <= 0.0001f)
		{
			negate_vector3d(&incoming, &normal);
			normalize3d(&normal);
		}
		along = dot_product3d(&incoming, &normal);
		if (along > 0.0f)
		{
			negate_vector3d(&normal, &normal);
			along = -along;
		}
		reflected.i = (incoming.i - 2.0f * along * normal.i) * keep * deflect_settings.strength;
		reflected.j = (incoming.j - 2.0f * along * normal.j) * keep * deflect_settings.strength;
		reflected.k = (incoming.k - 2.0f * along * normal.k) * keep * deflect_settings.strength;
	}
	else
	{
		/* batted along the swing, at the shot's speed or the swing's,
		whichever is faster (no aim at anything) */
		real swing = deflect.stroke_speed[best_hand] * units / (real)TICKS_PER_SECOND;

		scale_vector3d(&deflect.stroke[best_hand], MAX(speed, swing) * deflect_settings.strength, &reflected);
	}
	direction = reflected;
	if (!valid_real_vector3d(&reflected) || normalize3d(&direction) <= 0.0001f)
		return FALSE;

	projectile->object.translational_velocity = reflected;
	projectile->object.forward = direction;
	{
		real_vector3d left;

		cross_product3d(&projectile->object.up, &projectile->object.forward, &left);
		if (normalize3d(cross_product3d(&projectile->object.forward, &left, &projectile->object.up)) == 0.0f)
			normalize3d(perpendicular3d(&projectile->object.forward, &projectile->object.up));
	}
	projectile->object.owner_object_index = unit_index;
	projectile->object.owner_player_index = unit->unit.player_index;
	projectile->object.owner_team_index = unit->object.owner_team_index;
	projectile->projectile.ignore_object_index = unit_index;
	projectile_set_target_object_index(projectile_index, NONE);
	deflect.deflected[slot] = projectile_index;
	*fraction = best;
	if (best_way == _deflect_by_fist)
	{
		halo_vr_hand_pulse(best_hand, 1.0f, 0.06f);
	}
	else
	{
		halo_vr_hand_pulse(1, best_way == _deflect_by_swing ? 1.0f : 0.7f, 0.05f);
		if (!deflect.fist[0])
			halo_vr_hand_pulse(0, 0.4f, 0.04f);
	}
	if (deflect.log_tick == NONE || now - deflect.log_tick >= TICKS_PER_SECOND / 2 || now < deflect.log_tick)
	{
		deflect.log_tick = now;
		platform_log("vr deflect: a %s (%s) deflected by the %s, %.0f m/s in, %.0f m/s out",
			deflect_kind_names[kind], tag_get_name(projectile->definition_index), deflect_way_names[best_way],
			(double)(speed * TICKS_PER_SECOND / units),
			(double)(magnitude3d(&reflected) * TICKS_PER_SECOND / units));
	}
	return TRUE;
}

#endif
