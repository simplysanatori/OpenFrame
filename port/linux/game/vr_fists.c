/*
VR_FISTS.C

The VR mode's fists (HALO_VR; vr.fists, vr.fist_speed, vr.fist_damage;
docs/openframe-features.md), and the hands' tick, which asks
port/linux/src/vr.c for the hands once a tick (halo_vr_hands) for these and
for the grab's pull, held props and hand swap (vr_grab.c). game.c calls it
each tick after the players' update before the game; damage.c asks it of an
enemy's melee blow (a parry), units.c of an enemy beginning one (staggered,
it cannot).

- A punch: the hand without the weapon (and the weapon's hand while the
  player holds none, its swing then no longer the game's melee), swung past
  vr.fist_speed (its own speed, as the room has it: not the body's moving or
  turning, nor carried by the head), the fist's path over the tick (carried
  on a little ahead, as a swing does) passing within reach of an enemy's
  body (its pelvis to its head, as the game's aim assist has it, its width
  and the fist's), in sight of the eyes. The blow is the game's melee
  damage of the weapon in hand (an empty hand's: the game's unarmed blow),
  as hard as the swing was fast (half to one and a half times the game's
  blow, which a run's speed scales likewise), times vr.fist_damage, and
  less on the harder difficulties; the enemy is shoved along the punch.
  One blow a swing.
- A hard punch (a blow of 1.2 or more: the swing near twice vr.fist_speed)
  staggers the enemy a second, lifted off its feet (the game's airborne
  stumble), slowed (the game's stun), and unable to begin a melee; not a
  Hunter, nor an Elite whose shields hold.
- A parry: a hand swiped across an enemy's blow (at least 1.8 m/s, half of
  it across the line from the eyes to the enemy), between the eyes and the
  enemy (10 to 60 cm before the eyes, toward it), in the few ticks before
  the blow lands (eight on Easy to four on Legendary): none of the blow is
  dealt, and the enemy is staggered a second. The weapon's hand parries
  too (with the weapon), the left not on the foregrip.

Nothing is sent and nothing kept in the game state (the stagger's stun is
the game's own, on the enemy): a game of this computer alone (game_
connection local: the campaign; never online, split screen or with bots),
the first local player on foot, alive, in control, the controllers aiming.
The HUD says "Parry"; the hands pulse; no sounds.

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later): its
impact fists (vr_render.c's per-hand sweep: the blow's strength from the
swing, the sweep carried on, one blow a swing; units.c's
unit_vr_impact_melee: the weapon's melee damage for a fist, the shove along
the punch by the enemy's mobility) and its fist combat's parry and
durability (vr_extra_interaction.c: vr_combat_defend's parry window by
difficulty and the hand between the face and the attacker, the parry
swipe's 1.8 m/s, combat_durability, ENEMY HEALTH's 3 as vr.fist_damage's
third), made again on this port's hands and the game's own biped state.
Not carried over: its lunges, combos, counters, takedowns, guards, combat
moves, hit highlights and knockdowns (its ragdolls, Jolt), and its
body-contact (the bones): the body here is the aim assist's pill.
*/

#ifdef HALO_VR

#include "cseries.h"
#include "math/real_math.h"
#include "ai/actor_types.h"
#include "ai/actors.h"
#include "camera/director.h"
#include "cutscene/cinematics.h"
#include "game/game.h"
#include "game/game_allegiance.h"
#include "game/players.h"
#include "interface/hud_messaging.h"
#include "items/weapon_definitions.h"
#include "items/weapons.h"
#include "objects/damage.h"
#include "objects/object_definitions.h"
#include "objects/objects.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "units/bipeds.h"
#include "units/unit_definitions.h"
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
/* vr_grab.c's: what it has for the hands' grips, and its tick */
void vr_grab_hands_claims(int claim[2]);
void vr_grab_hands_tick(struct halo_vr_hands const *hands, long unit_index);

/* the fist (metres): its radius, and how near a body its path need pass
beyond that (an assist: the game's body is a pill about the bones); the
path carried on ahead by the swing's speed times FIST_CARRY_SECONDS, at
most FIST_CARRY_FARTHEST; a tick's path longer than FIST_PATH_LONGEST is
the tracking's jump, no swing; the bodies looked for within
FIST_SEARCH_RADIUS of the path */
#define FIST_RADIUS 0.06f
#define FIST_ASSIST 0.10f
#define FIST_CARRY_SECONDS 0.05f
#define FIST_CARRY_FARTHEST 0.15f
#define FIST_PATH_LONGEST 1.0f
#define FIST_SEARCH_RADIUS 2.0f
#define FIST_SEARCH_COUNT 16
/* the shove along the punch (metres a second at a blow of 1, times the
enemy's mobility, its acceleration scale between FIST_MOBILITY_LEAST and
1); a blow of FIST_STRONG_BLOW or more staggers: FIST_STAGGER_TICKS, lifted
FIST_STAGGER_LIFT metres a second, the game's stun at FIST_STAGGER_STUN */
#define FIST_SHOVE 1.2f
#define FIST_MOBILITY_LEAST 0.2f
#define FIST_STRONG_BLOW 1.2f
#define FIST_STAGGER_TICKS 30
#define FIST_STAGGER_LIFT 1.2f
#define FIST_STAGGER_STUN 0.6f
#define FIST_STAGGERED_COUNT 8
/* a parry: a swipe at least PARRY_SPEED (metres a second), PARRY_ACROSS of
it across the line to the enemy; the hand PARRY_NEAREST to PARRY_FARTHEST
before the eyes, toward the enemy (PARRY_FACING, a cosine); the enemy
within PARRY_ENEMY_FARTHEST; shoved back PARRY_SHOVE */
#define PARRY_SPEED 1.8f
#define PARRY_ACROSS 0.5f
#define PARRY_NEAREST 0.10f
#define PARRY_FARTHEST 0.60f
#define PARRY_FACING 0.35f
#define PARRY_ENEMY_FARTHEST 3.5f
#define PARRY_SHOVE 1.2f

static struct
{
	unsigned long changes;
	boolean on;
	real speed;
	real damage;
} fists_settings = { (unsigned long)-1, FALSE, 2.0f, 0.33f };

static struct
{
	/* the game's tick last seen (NONE: none) and the player's unit; each
	hand's place in the world at the last tick, the swing whose blow has
	landed, and the last swipe fast enough to parry (its tick, its velocity
	in the world, metres a second); the world's units a metre */
	long tick;
	long unit_index;
	boolean hand_valid[2];
	real_point3d hand[2];
	boolean landed[2];
	unsigned long landed_swing[2];
	long parry_tick[2];
	real_vector3d parry_velocity[2];
	real units;
	/* the enemies staggered, and until when */
	long staggered[FIST_STAGGERED_COUNT];
	long staggered_until[FIST_STAGGERED_COUNT];
	/* the weapon in the other hand (vr.hand_swap), as the HUD last said */
	boolean swapped;
} fists;
static boolean fists_initialized = FALSE;

static void fists_settings_read(void)
{
	if (fists_settings.changes == config_changes())
		return;
	fists_settings.changes = config_changes();
	fists_settings.on = config_boolean("vr.fists") != 0;
	fists_settings.speed = (real)config_real("vr.fist_speed");
	if (!(fists_settings.speed >= 1.0f && fists_settings.speed <= 4.0f))
		fists_settings.speed = 2.0f;
	fists_settings.damage = (real)config_real("vr.fist_damage");
	if (!(fists_settings.damage >= 0.1f && fists_settings.damage <= 1.0f))
		fists_settings.damage = 0.33f;
}

static void fists_reset(void)
{
	short index;

	csmemset(&fists, 0, sizeof(fists));
	fists.tick = NONE;
	fists.unit_index = NONE;
	fists.parry_tick[0] = fists.parry_tick[1] = NONE;
	for (index = 0; index < FIST_STAGGERED_COUNT; index++)
		fists.staggered[index] = NONE;
	fists_initialized = TRUE;
}

/* the first local player's unit on foot, alive, in its control, in a game
of this computer alone; NONE otherwise */
static long fists_player_unit(void)
{
	long player_index = local_player_get_player_index(0);
	long unit_index = player_index != NONE ? player_get(player_index)->unit_index : NONE;
	struct unit_datum *unit = unit_index != NONE ? unit_try_and_get(unit_index) : NULL;

	if (game_connection() != _game_connection_local || !unit || unit->object.parent_object_index != NONE ||
		TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || cinematic_in_progress() ||
		director_inhibited_input(0))
	{
		return NONE;
	}
	return unit_index;
}

/* an enemy of the player's: a biped the AI drives, alive, on its feet or
riding, on a team the player's is the enemy of */
static boolean fists_enemy(long unit_index, long target_index)
{
	struct unit_datum *unit = unit_try_and_get(unit_index);
	struct unit_datum *target = target_index != NONE && target_index != unit_index ?
		(struct unit_datum *)object_try_and_get_and_verify_type(target_index, _object_mask_biped) : NULL;

	return unit && target && target->unit.player_index == NONE &&
		!TEST_FLAG(target->object.damage_flags, _object_dead_bit) &&
		!TEST_FLAG(target->unit.flags, _unit_suspended_bit) &&
		game_team_is_enemy(unit->object.owner_team_index, target->object.owner_team_index);
}

/* the AI's kind of a unit (_actor_*), NONE where none drives it */
static short fists_actor_type(long target_index)
{
	struct unit_datum *target = unit_try_and_get(target_index);
	long actor_index = target ? (target->unit.swarm_actor_index != NONE ? target->unit.swarm_actor_index :
		target->unit.actor_index) : NONE;
	struct actor_datum *actor = actor_index != NONE ? actor_try_and_get(actor_index) : NULL;

	return actor ? actor->meta.type : NONE;
}

/* how readily a body moves when struck: its acceleration scale */
static real fists_mobility(long target_index)
{
	struct object_definition *definition = object_definition_get(object_get(target_index)->definition_index);
	real mobility = definition->object.acceleration_scale;

	return mobility >= FIST_MOBILITY_LEAST && mobility <= 1.0f ? mobility :
		mobility > 1.0f ? 1.0f : FIST_MOBILITY_LEAST;
}

/* the melee damage of a blow: Ported from HaloCE-DecompRemix-VR by
moistman42069 (AGPL-3.0-or-later): unit_vr_impact_melee's choice (the
weapon's melee blow, else the biped's own, else the unarmed one) and its
durability by difficulty (combat_durability, melee only) */
static long fists_damage_effect(long unit_index)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_definition *definition = unit_definition_get(unit->definition_index);
	long weapon_index = unit->unit.current_weapon_index != NONE ?
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] : NONE;
	long effect_index = NONE;

	if (weapon_index != NONE)
		effect_index = weapon_definition_get(weapon_get(weapon_index)->definition_index)->weapon.melee_attack_damage.index;
	if (effect_index == NONE)
		effect_index = definition->unit.melee_damage.index;
	if (effect_index == NONE)
		effect_index = unit_unarmed_melee_damage(unit_index);
	return effect_index;
}

static real fists_durability(void)
{
	static real const durability[] = { 0.8f, 1.0f, 1.3f, 1.6f };
	short difficulty = game_difficulty_level_get();

	return durability[difficulty < 0 ? 0 : difficulty > 3 ? 3 : difficulty];
}

/* ---------- the stagger */

static long fists_staggered_slot(long target_index)
{
	long now = game_time_get();
	short index;

	for (index = 0; index < FIST_STAGGERED_COUNT; index++)
	{
		if (fists.staggered[index] == target_index && now < fists.staggered_until[index] &&
			fists.staggered_until[index] - now <= FIST_STAGGER_TICKS)
		{
			return index;
		}
	}
	return NONE;
}

/* units.c, as a unit would begin a melee attack: whether a punch or a
parry staggered it, which it cannot through (the first local player's
enemies, in a game of this computer alone) */
boolean vr_fists_staggered(long unit_index)
{
	if (!fists_initialized || unit_index == NONE || game_connection() != _game_connection_local)
		return FALSE;
	return fists_staggered_slot(unit_index) != NONE;
}

/* the enemy staggered: shoved this way (the world's, level or not;
metres a second), lifted, slowed by the game's stun, its melee held off */
static void fists_stagger(long target_index, real_vector3d const *push, real lift, char const *why)
{
	struct unit_datum *target = unit_get(target_index);
	long now = game_time_get();
	real_vector3d velocity;
	short index, slot = NONE;

	for (index = 0; index < FIST_STAGGERED_COUNT; index++)
	{
		if (fists.staggered[index] == target_index || fists.staggered[index] == NONE ||
			now >= fists.staggered_until[index] || fists.staggered_until[index] - now > FIST_STAGGER_TICKS)
		{
			slot = index;
			if (fists.staggered[index] == target_index)
				break;
		}
	}
	if (slot == NONE)
	{
		/* (the one ending soonest gives way) */
		slot = 0;
		for (index = 1; index < FIST_STAGGERED_COUNT; index++)
		{
			if (fists.staggered_until[index] < fists.staggered_until[slot])
				slot = index;
		}
	}
	fists.staggered[slot] = target_index;
	fists.staggered_until[slot] = now + FIST_STAGGER_TICKS;
	if (target->unit.body_stun < FIST_STAGGER_STUN)
		target->unit.body_stun = FIST_STAGGER_STUN;
	if (target->unit.body_stun_ticks < FIST_STAGGER_TICKS)
		target->unit.body_stun_ticks = FIST_STAGGER_TICKS;
	/* (a body riding a seat stays in it) */
	if (target->object.parent_object_index == NONE && target->object.type == _object_type_biped)
	{
		real scale = fists_mobility(target_index) * fists.units / (real)TICKS_PER_SECOND;

		velocity.i = push->i * scale;
		velocity.j = push->j * scale;
		velocity.k = push->k * scale + lift * fists.units / (real)TICKS_PER_SECOND;
		biped_accelerate(target_index, &velocity);
	}
	platform_log("vr fists: %s staggers %s for %.1f s", why, tag_get_name(target->definition_index),
		(double)FIST_STAGGER_TICKS / TICKS_PER_SECOND);
}

/* ---------- the punch */

/* the nearest points of two segments (p0 p1, q0 q1) and their distance:
Ericson's closest points of two segments */
static real fists_segments(real_point3d const *p0, real_point3d const *p1, real_point3d const *q0,
	real_point3d const *q1, real_point3d *on_p, real_point3d *on_q)
{
	real_vector3d d1, d2, r;
	real a, e, f, s = 0.0f, t = 0.0f;

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
	on_p->x = p0->x + d1.i * s;
	on_p->y = p0->y + d1.j * s;
	on_p->z = p0->z + d1.k * s;
	on_q->x = q0->x + d2.i * t;
	on_q->y = q0->y + d2.j * t;
	on_q->z = q0->z + d2.k * t;
	return distance3d(on_p, on_q);
}

/* nothing of the level between the eyes and the point */
static boolean fists_in_sight(long unit_index, real_point3d const *eye, real_point3d const *point)
{
	struct collision_result collision;
	real_vector3d ray;

	vector_from_points3d(eye, point, &ray);
	return !collision_test_vector(FLAG(_collision_test_front_facing_surfaces_bit) |
		FLAG(_collision_test_structure_bit), eye, &ray, unit_index, &collision);
}

/* a blow (blow: its strength, 0.5 to 1.5) on the enemy, at the point, along
the way given (the world's, normalized): Ported from HaloCE-DecompRemix-VR
by moistman42069 (AGPL-3.0-or-later): unit_vr_impact_melee's damage and
the shove after it (vr_combat_hit, without ragdolls) */
static void fists_blow(long unit_index, long target_index, real_point3d const *point, real_vector3d const *along,
	real blow)
{
	struct unit_datum *unit = unit_get(unit_index);
	struct unit_datum *target = unit_get(target_index);
	long effect_index = fists_damage_effect(unit_index);
	short type = fists_actor_type(target_index);
	struct damage_data damage;
	real_vector3d normal, push;
	boolean strong;

	if (effect_index == NONE)
	{
		platform_log("vr fists: a punch on %s, no melee damage to deal", tag_get_name(target->definition_index));
		return;
	}
	strong = blow >= FIST_STRONG_BLOW && type != _actor_hunter &&
		(type != _actor_elite || target->object.shield_vitality <= 0.0f);
	damage_data_new(&damage, effect_index);
	damage.location = unit->object.location;
	damage.owner_object_index = unit_index;
	damage.owner_team_index = unit->object.owner_team_index;
	damage.owner_player_index = unit->unit.player_index;
	damage.origin = *point;
	damage.epicenter = *point;
	damage.direction = *along;
	damage.scale = blow;
	damage.multiplier = fists_settings.damage / fists_durability();
	negate_vector3d(along, &normal);
	object_cause_damage(&damage, target_index, NONE, NONE, NONE, &normal);
	platform_log("vr fists: a punch (blow %.2f) on %s", blow, tag_get_name(target->definition_index));
	/* (still there: a blow may have killed it, and its body goes on) */
	if (!unit_try_and_get(target_index))
		return;
	push = *along;
	push.k = 0.0f;
	if (normalize3d(&push) <= 0.0f)
		return;
	if (strong && !TEST_FLAG(target->object.damage_flags, _object_dead_bit))
	{
		scale_vector3d(&push, FIST_SHOVE * blow, &push);
		fists_stagger(target_index, &push, FIST_STAGGER_LIFT, "a hard punch");
	}
	else if (target->object.parent_object_index == NONE)
	{
		scale_vector3d(&push, FIST_SHOVE * PIN(blow, 0.5f, 1.5f) * fists_mobility(target_index) * fists.units /
			(real)TICKS_PER_SECOND, &push);
		biped_accelerate(target_index, &push);
	}
}

/* a hand's punch this tick, its fist from where it was at the last tick
to where it is now (hand: 1 the weapon's, 0 the other); TRUE, it landed */
static boolean fists_punch(long unit_index, short hand, real_point3d const *from, real_point3d const *to, real speed)
{
	struct unit_datum *unit = unit_get(unit_index);
	real units = fists.units;
	real_vector3d path, along;
	real_point3d end, eye, middle, best_point;
	real length, carry, blow, best = 1e30f;
	long candidates[FIST_SEARCH_COUNT], best_index = NONE;
	short count, index;

	best_point = *to;
	vector_from_points3d(from, to, &path);
	length = magnitude3d(&path);
	if (length <= 0.001f * units || length > FIST_PATH_LONGEST * units)
		return FALSE;
	along = path;
	normalize3d(&along);
	/* (the swing carried on ahead) */
	carry = speed * FIST_CARRY_SECONDS;
	carry = (carry > FIST_CARRY_FARTHEST ? FIST_CARRY_FARTHEST : carry) * units;
	point_from_line3d(to, &along, carry, &end);
	blow = PIN(speed / (2.0f * fists_settings.speed) + 0.25f, 0.5f, 1.5f);
	unit_get_camera_position(unit_index, &eye);
	midpoint3d(from, &end, &middle);
	count = objects_in_sphere(0, _object_mask_biped, &unit->object.location, &middle,
		FIST_SEARCH_RADIUS * units + length, candidates, FIST_SEARCH_COUNT);
	for (index = 0; index < count; index++)
	{
		real_point3d base, top, on_fist, on_body;
		real_vector3d height;
		real width, distance;

		if (!fists_enemy(unit_index, candidates[index]))
			continue;
		biped_get_autoaim_pill(candidates[index], &base, &height, &width);
		top.x = base.x + height.i;
		top.y = base.y + height.j;
		top.z = base.z + height.k;
		distance = fists_segments(from, &end, &base, &top, &on_fist, &on_body) - width;
		if (distance > (FIST_RADIUS + FIST_ASSIST) * units || distance >= best ||
			!fists_in_sight(unit_index, &eye, &on_fist))
		{
			continue;
		}
		best = distance;
		best_index = candidates[index];
		best_point = on_fist;
	}
	if (best_index == NONE)
		return FALSE;
	fists_blow(unit_index, best_index, &best_point, &along, blow);
	halo_vr_hand_pulse(hand, 1.0f, 0.08f);
	return TRUE;
}

/* ---------- the parry */

/* damage.c, as an enemy's melee blow is about to be dealt (victim, its
owner): parried (TRUE: none of it dealt) by a hand swiped across it just
before. Ported from HaloCE-DecompRemix-VR by moistman42069
(AGPL-3.0-or-later): vr_combat_defend's parry (its window by difficulty,
the hand between the face and the attacker, the attacker's stagger) */
boolean vr_fists_parry(long victim_index, long attacker_index)
{
	static short const window[] = { 8, 6, 5, 4 };
	long now = game_time_get();
	short difficulty = game_difficulty_level_get();
	short ticks = window[difficulty < 0 ? 0 : difficulty > 3 ? 3 : difficulty];
	real units = fists.units;
	struct object_datum *attacker;
	real_point3d eye;
	real_vector3d to;
	short hand, parried = NONE;

	if (!fists_initialized || victim_index == NONE || victim_index != fists.unit_index ||
		attacker_index == NONE || units <= 0.0f || game_connection() != _game_connection_local)
	{
		return FALSE;
	}
	fists_settings_read();
	if (!fists_settings.on || fists_player_unit() != victim_index || !fists_enemy(victim_index, attacker_index))
		return FALSE;
	attacker = object_get(attacker_index);
	unit_get_camera_position(victim_index, &eye);
	vector_from_points3d(&eye, &attacker->object.bounding_sphere_center, &to);
	if (magnitude3d(&to) > PARRY_ENEMY_FARTHEST * units || normalize3d(&to) <= 0.0f)
		return FALSE;
	for (hand = 0; hand < 2; hand++)
	{
		real_vector3d toward, velocity = fists.parry_velocity[hand], across;
		real distance, ahead;

		if (fists.parry_tick[hand] == NONE || now < fists.parry_tick[hand] || now - fists.parry_tick[hand] > ticks ||
			!fists.hand_valid[hand])
		{
			continue;
		}
		vector_from_points3d(&eye, &fists.hand[hand], &toward);
		distance = normalize3d(&toward);
		if (distance < PARRY_NEAREST * units || distance > PARRY_FARTHEST * units ||
			dot_product3d(&toward, &to) < PARRY_FACING)
		{
			continue;
		}
		ahead = dot_product3d(&velocity, &to);
		across.i = velocity.i - to.i * ahead;
		across.j = velocity.j - to.j * ahead;
		across.k = velocity.k - to.k * ahead;
		if (magnitude3d(&across) < PARRY_ACROSS * PARRY_SPEED)
			continue;
		parried = hand;
		break;
	}
	if (parried == NONE)
		return FALSE;
	fists.parry_tick[0] = fists.parry_tick[1] = NONE;
	halo_vr_hand_pulse(parried, 1.0f, 0.07f);
	halo_vr_hand_pulse(!parried, 0.5f, 0.04f);
	hud_print_message(0, L"Parry");
	{
		real_vector3d back = to;

		back.k = 0.0f;
		if (normalize3d(&back) > 0.0f)
		{
			scale_vector3d(&back, PARRY_SHOVE, &back);
			fists_stagger(attacker_index, &back, 0.0f, "a parry");
		}
	}
	platform_log("vr fists: parried %s's blow with the %s hand", tag_get_name(attacker->definition_index),
		parried ? "weapon's" : "other");
	return TRUE;
}

/* ---------- the hands' tick */

/* game.c, once a tick, after the players' update before the game: the
hands told and asked (halo_vr_hands), the punches and the parry's swipes,
then the grab's pull, props and hand swap (vr_grab.c) */
void vr_hands_tick(void)
{
	struct halo_vr_hands hands;
	long now = game_time_get();
	long unit_index;
	struct unit_datum *unit;
	boolean allowed;
	real_point3d points[2];
	boolean valid[2];
	short hand;

	if (!fists_initialized)
		fists_reset();
	/* (the game's time gone back: a revert, a map begun) */
	if (fists.tick != NONE && now < fists.tick)
		fists_reset();
	fists.tick = now;
	fists_settings_read();
	unit_index = fists_player_unit();
	unit = unit_index != NONE ? unit_get(unit_index) : NULL;
	csmemset(&hands, 0, sizeof(hands));
	vr_grab_hands_claims(hands.claim);
	hands.unarmed = unit && (unit->unit.current_weapon_index == NONE ||
		unit->unit.weapon_object_indices[unit->unit.current_weapon_index] == NONE);
	hands.local_game = game_connection() == _game_connection_local;
	if (!halo_vr_running() || !halo_vr_hands(&hands))
		return;
	/* (the weapon passed to the other hand: said) */
	if ((hands.swapped != 0) != fists.swapped)
	{
		fists.swapped = hands.swapped != 0;
		if (local_player_get_player_index(0) != NONE)
			hud_print_message(0, halo_vr_left_handed() ? L"Weapon in the left hand" : L"Weapon in the right hand");
	}
	if (unit_index != fists.unit_index)
	{
		fists.unit_index = unit_index;
		fists.hand_valid[0] = fists.hand_valid[1] = FALSE;
		fists.parry_tick[0] = fists.parry_tick[1] = NONE;
	}
	allowed = unit_index != NONE && hands.enabled;
	for (hand = 0; hand < 2; hand++)
	{
		real units;

		valid[hand] = allowed && hands.valid[hand] &&
			vr_render_hand_world(hands.place[hand], NULL, &points[hand], NULL, &units);
		if (valid[hand])
			fists.units = units;
	}
	if (allowed && fists_settings.on)
	{
		for (hand = 0; hand < 2; hand++)
		{
			real_point3d unused;
			real units;

			if (!valid[hand])
				continue;
			/* (a new swing lands again) */
			if (fists.landed[hand] && fists.landed_swing[hand] != hands.swing[hand])
				fists.landed[hand] = FALSE;
			/* (a blow landed spends the swing) */
			if (hands.punch[hand] && fists.hand_valid[hand] && !fists.landed[hand] &&
				fists_punch(unit_index, hand, &fists.hand[hand], &points[hand], hands.peak[hand]))
			{
				fists.landed[hand] = TRUE;
				fists.landed_swing[hand] = hands.swing[hand];
			}
			if (hands.peak[hand] >= PARRY_SPEED &&
				vr_render_hand_world(hands.place[hand], hands.peak_velocity[hand], &unused,
					&fists.parry_velocity[hand], &units))
			{
				fists.parry_tick[hand] = now;
			}
		}
	}
	for (hand = 0; hand < 2; hand++)
	{
		fists.hand_valid[hand] = valid[hand];
		if (valid[hand])
			fists.hand[hand] = points[hand];
	}
	vr_grab_hands_tick(&hands, allowed ? unit_index : NONE);
}

#endif
