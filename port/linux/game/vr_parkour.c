/*
VR_PARKOUR.C

The VR mode's parkour (HALO_VR; vr.parkour, vr.slide_momentum,
vr.walljump_scale; docs/openframe-features.md), for bipeds.c: the first
local player's movement on foot, a tick at a time.

- A slide: a crouch begun (the left stick's click, or the head lowered with
  vr.physical_crouch) while sprinting (vr.sprint_mode: vr_comfort.c's
  sprint, now or in the last third of a second; a crouch only pauses a
  toggled sprint) on the ground at a run or faster slides on with the run's
  momentum (a little more, by vr.slide_momentum), crouched (the game's
  crouch: its lower height), giving it up to friction, taking a slope's
  share of gravity, steered a little by the stick. It ends as the player
  stands, jumps (the momentum carried into the jump), leaves the ground,
  meets a wall, slows to half a run, or after its time (by
  vr.slide_momentum: 1.1 seconds at 1).
- A wall jump: in the air beside a wall (probed about the body at the knee,
  the waist and the chest), Jump pressed again leaps off it where the head
  looks (turned off the wall at least 35% of the way), at least as fast as
  the body went, as high as a jump times vr.walljump_scale (more looking
  up); once a wall until the feet are on the ground again, the leap held
  against the air control for 12 ticks.
- A wall run: a jump at a wall with the stick pushed (past 60%) along it
  within a second runs along the wall, level for 60% of its 2 seconds and
  then easing into the fall; a wall jump ends it, and from it leaps where
  the head looks; steering away from the wall ends it.

Each changes the tick's movement as bipeds.c makes it ready
(biped_physics_prepare): the slide the movement the player asks for (in
every sub-step of game.sim_rate as well: vr_parkour_movement), the jumps a
velocity the tick starts from; the game's own collision moves the body.
Nothing is sent and nothing kept in the game state: a game of this
computer alone (game_connection local: the campaign; never online, split
screen or bots, network games which every machine must step alike), on
foot, alive, not in a cinematic or where the game takes the player's
control.

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
its slide (vr_slide.c, vr_slide_math.h), wall jump and wall run
(vr_parkour.c, vr_parkour_math.h), their rules and tuning carried over,
made again on this port's bipeds' physics (a tick's velocity at its start,
its sub-steps). Its slide's blows to the AI (fist combat's knockdowns) and
its grip climbing (a free hand's grip on any wall: this port's grips are
the holsters', the foregrip's, the magazines', the grenades', a vehicle's
holds, and both together recentre) are not carried over.
*/

#ifdef HALO_VR

#include "cseries.h"
#include "math/real_math.h"
#include "game/game.h"
#include "game/players.h"
#include "camera/director.h"
#include "cutscene/cinematics.h"
#include "objects/objects.h"
#include "units/units.h"
#include "units/bipeds.h"
#include "units/biped_definitions.h"
#include "physics/collision_usage.h"
#include "physics/collisions.h"
#include "physics/physics.h"

#include "../src/vr.h"

#include <math.h>
#include <string.h>

/* the platform layer's (port/linux/src/port_config.c) */
int config_boolean(char const *name);
double config_real(char const *name);
unsigned long config_changes(void);
void platform_log(char const *format, ...);
/* vr_comfort.c's: how many times the run the biped sprints at now (1: not) */
real vr_sprint_speed_scale(long biped_index);
/* vr_render.c's: where the head looked as the last eye pass drew it */
boolean vr_render_head_forward(real_vector3d *forward);

#define WORLD_UNITS_PER_METRE (1.0f / 3.048f)

/* the slide: friction (a share of the speed each second), the share of
gravity along a slope it feels, the stick's steering (degrees a second at
a full push), gravity (metres a second squared); it starts at a run times
SLIDE_RUN_SHARE or faster, while sprinting or within SLIDE_SPRINT_TICKS of
it; it ends slower than SLIDE_END_SHARE of a run, after SLIDE_AIRBORNE_TICKS
in the air, or stalled (the body moving under SLIDE_STALL_SHARE of the
slide's speed for SLIDE_STALL_TICKS: a wall) */
#define SLIDE_FRICTION 0.8f
#define SLIDE_SLOPE_SHARE 0.55f
#define SLIDE_STEER_DEGREES 50.0f
#define SLIDE_GRAVITY 9.8f
#define SLIDE_FASTEST 14.0f
#define SLIDE_RUN_SHARE 0.85f
#define SLIDE_SPRINT_TICKS 9
#define SLIDE_END_SHARE 0.5f
#define SLIDE_AIRBORNE_TICKS 8
#define SLIDE_STALL_SHARE 0.35f
#define SLIDE_STALL_TICKS 2
/* the walls: probed this far beyond the body's radius (metres; the wall
run along farther toward its wall), at these heights of the body (knee,
waist, chest: of the pill's height) */
#define WALL_REACH 0.18f
#define WALL_RUN_REACH 0.35f
/* a wall jump's speed off the wall (metres a second, times
vr.walljump_scale), held against the air control so many ticks */
#define WALL_JUMP_SPEED 5.0f
#define WALL_LEAP_HOLD_TICKS 12
/* a wall run: begun within WALL_RUN_ENTRY_TICKS of a jump (not before
WALL_RUN_EARLIEST_TICKS: the jump's own rise), or within two seconds of a
wall jump (chained), with the stick past WALL_RUN_STICK; WALL_RUN_SPEED
times the speed it began at (at least WALL_RUN_SLOWEST, at most
WALL_RUN_FASTEST metres a second), for WALL_RUN_SECONDS; a gap in its wall
kept over for WALL_RUN_GAP_TICKS */
#define WALL_RUN_ENTRY_TICKS 30
#define WALL_RUN_EARLIEST_TICKS 6
#define WALL_RUN_STICK 0.6f
#define WALL_RUN_SPEED 1.5f
#define WALL_RUN_SLOWEST 4.0f
#define WALL_RUN_FASTEST 9.0f
#define WALL_RUN_SECONDS 2.0f
#define WALL_RUN_GAP_TICKS 6

static struct
{
	unsigned long changes;
	boolean on;
	real momentum;
	real walljump;
} parkour_settings = { (unsigned long)-1, FALSE, 1.0f, 1.5f };

static struct
{
	/* the biped and the tick it was last moved at */
	long unit_index;
	long tick;
	/* Jump: held last tick, ticks since pressed, since the last wall jump */
	boolean jump_down;
	long jump_age;
	long walljump_age;
	/* the slide: on, its velocity (metres a second, level, in the world),
	its ticks, the ticks in the air and stalled, the ticks since a sprint,
	and the crouch last tick */
	boolean sliding;
	real slide_velocity[2];
	long slide_ticks;
	long slide_airborne_ticks;
	long slide_stall_ticks;
	long sprint_recent;
	boolean crouch_was;
	/* the walls: one met (its normal and a point), jumped off since the
	feet left the ground; the run along it (its ticks, its speed, the ticks
	its wall has been lost); the leap held (ticks, its level way, speed) */
	boolean wall_valid;
	real_vector3d wall_normal;
	real_point3d wall_point;
	boolean jumped_wall;
	boolean running;
	long run_ticks;
	real run_speed;
	long lost_ticks;
	long leap_ticks;
	real leap_direction[2];
	real leap_speed;
} parkour;
static boolean parkour_initialized = FALSE;

static void parkour_settings_read(void)
{
	if (parkour_settings.changes == config_changes())
		return;
	parkour_settings.changes = config_changes();
	parkour_settings.on = config_boolean("vr.parkour") != 0;
	parkour_settings.momentum = (real)config_real("vr.slide_momentum");
	if (!(parkour_settings.momentum >= 0.5f && parkour_settings.momentum <= 2.0f))
		parkour_settings.momentum = 1.0f;
	parkour_settings.walljump = (real)config_real("vr.walljump_scale");
	if (!(parkour_settings.walljump >= 0.5f && parkour_settings.walljump <= 1.5f))
		parkour_settings.walljump = 1.5f;
}

static void parkour_reset(void)
{
	if (parkour.sliding || parkour.running)
		platform_log("vr parkour: reset");
	csmemset(&parkour, 0, sizeof(parkour));
	parkour.unit_index = NONE;
	parkour.tick = NONE;
	/* (a Jump held as it starts is no press) */
	parkour.jump_down = TRUE;
	parkour.jump_age = 1000;
	parkour.walljump_age = 1000;
}

/* the biped is the first local player's, on foot and in its control, in a
game of this computer alone, with parkour on */
static struct biped_datum *parkour_biped(long biped_index)
{
	long player_index = local_player_get_player_index(0);
	struct biped_datum *biped;

	parkour_settings_read();
	if (!parkour_settings.on || !halo_vr_running() || game_connection() != _game_connection_local ||
		player_index == NONE || player_get(player_index)->unit_index != biped_index || cinematic_in_progress() ||
		director_inhibited_input(0))
	{
		return NULL;
	}
	biped = biped_try_and_get(biped_index);
	if (!biped || biped->object.parent_object_index != NONE ||
		TEST_FLAG(biped->object.damage_flags, _object_dead_bit) ||
		TEST_FLAG(biped->biped.flags, _biped_no_collision_bit) ||
		TEST_FLAG(biped->unit.flags, _unit_suspended_bit))
	{
		return NULL;
	}
	return biped;
}

/* ---------- the slide's kinematics (metres and seconds)

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
vr_slide_math.h's vr_slide_entry_speed, vr_slide_duration and
vr_slide_step. */

/* the speed a slide starts at: the run it comes from, lifted by momentum
(0.5 to 2), never past SLIDE_FASTEST */
static real slide_entry_speed(real speed, real momentum)
{
	real entry = speed * (1.0f + 0.15f * momentum);

	return entry > SLIDE_FASTEST ? SLIDE_FASTEST : entry;
}

/* how long a slide may last (seconds) */
static real slide_duration(real momentum)
{
	return 0.55f + 0.55f * momentum;
}

/* a step of a slide's velocity (v, level) over dt seconds: friction, the
slope (the ground's normal, NULL none) and the stick's steering (steer, its
way in the world; amount 0 to 1); the speed after it */
static real slide_step(real v[2], real dt, real const ground[3], real const steer[2], real amount)
{
	real keep = (real)exp(-SLIDE_FRICTION * dt);
	real speed;

	v[0] *= keep;
	v[1] *= keep;
	/* (a slope: gravity's part along the ground, down faster, up slower) */
	if (ground && ground[2] > 0.5f)
	{
		real a = SLIDE_GRAVITY * SLIDE_SLOPE_SHARE;

		v[0] += ground[0] * ground[2] * a * dt;
		v[1] += ground[1] * ground[2] * a * dt;
	}
	speed = (real)sqrt(v[0] * v[0] + v[1] * v[1]);
	if (speed > 0.001f && amount > 0.0f)
	{
		real length = (real)sqrt(steer[0] * steer[0] + steer[1] * steer[1]);

		if (length > 0.001f)
		{
			real sx = steer[0] / length, sy = steer[1] / length;
			real ux = v[0] / speed, uy = v[1] / speed;
			real cross = ux * sy - uy * sx, dot = ux * sx + uy * sy;
			real angle = (real)atan2(cross, dot);
			real limit = SLIDE_STEER_DEGREES * 0.01745329f * dt * (amount > 1.0f ? 1.0f : amount);
			real turn = angle > limit ? limit : angle < -limit ? -limit : angle;

			/* (no turning back on itself: a stick pulled back brakes) */
			if (dot > -0.5f)
			{
				real c = (real)cos(turn), s = (real)sin(turn);

				v[0] = (ux * c - uy * s) * speed;
				v[1] = (ux * s + uy * c) * speed;
			}
			else
			{
				real brake = (real)exp(-1.5f * dt * amount);

				v[0] *= brake;
				v[1] *= brake;
				speed *= brake;
			}
		}
	}
	return speed;
}

static void slide_end(char const *why)
{
	if (parkour.sliding)
	{
		platform_log("vr parkour: slide ended (%s) after %.2f s", why,
			(double)parkour.slide_ticks / TICKS_PER_SECOND);
	}
	parkour.sliding = FALSE;
	parkour.slide_ticks = parkour.slide_airborne_ticks = parkour.slide_stall_ticks = 0;
}

/* a tick of the slide: begun, stepped or ended (forward: the facing,
level; run: the run's speed, metres a second) */
static void slide_tick(long biped_index, struct biped_datum *biped, real_vector3d const *forward, real run)
{
	boolean crouch = TEST_FLAG(biped->unit.control_flags, _unit_control_crouch_modifier_bit);
	boolean crouch_begun = crouch && !parkour.crouch_was;
	boolean airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
	real_vector3d horizontal = biped->object.translational_velocity;
	real actual, speed;

	parkour.crouch_was = crouch;
	horizontal.k = 0.0f;
	/* (metres a second) */
	actual = magnitude3d(&horizontal) * TICKS_PER_SECOND / WORLD_UNITS_PER_METRE;
	if (halo_vr_sprint() >= 0.5f && vr_sprint_speed_scale(biped_index) > 1.0f)
		parkour.sprint_recent = SLIDE_SPRINT_TICKS;
	else if (parkour.sprint_recent > 0)
		parkour.sprint_recent--;

	if (!parkour.sliding)
	{
		/* (a crouch begun while sprinting, on the ground, at a run or faster) */
		if (!crouch_begun || parkour.sprint_recent <= 0 || airborne || !(actual >= SLIDE_RUN_SHARE * run) ||
			normalize3d(&horizontal) <= 0.0f)
		{
			return;
		}
		speed = slide_entry_speed(actual, parkour_settings.momentum);
		parkour.slide_velocity[0] = horizontal.i * speed;
		parkour.slide_velocity[1] = horizontal.j * speed;
		parkour.sliding = TRUE;
		parkour.slide_ticks = parkour.slide_airborne_ticks = parkour.slide_stall_ticks = 0;
		halo_vr_pulse(0.35f, 0.08f);
		platform_log("vr parkour: slide at %.1f m/s (the run %.1f)", speed, run);
	}

	if (!crouch)
	{
		slide_end("stood");
		return;
	}
	if (TEST_FLAG(biped->unit.control_flags, _unit_control_jump_bit))
	{
		/* (the jump keeps the slide's momentum: the body has it) */
		slide_end("jumped");
		return;
	}
	parkour.slide_airborne_ticks = airborne ? parkour.slide_airborne_ticks + 1 : 0;
	if (parkour.slide_airborne_ticks > SLIDE_AIRBORNE_TICKS)
	{
		slide_end("left the ground");
		return;
	}
	if (parkour.slide_ticks > (long)(slide_duration(parkour_settings.momentum) * TICKS_PER_SECOND))
	{
		slide_end("its time");
		return;
	}
	speed = (real)sqrt(parkour.slide_velocity[0] * parkour.slide_velocity[0] +
		parkour.slide_velocity[1] * parkour.slide_velocity[1]);
	/* (a wall: the body barely moving where the slide would take it) */
	if (parkour.slide_ticks > 3 && actual < SLIDE_STALL_SHARE * speed)
	{
		if (++parkour.slide_stall_ticks >= SLIDE_STALL_TICKS)
		{
			slide_end("a wall");
			return;
		}
	}
	else
	{
		parkour.slide_stall_ticks = 0;
	}
	{
		real ground[3];
		real steer[2] = { 0.0f, 0.0f };
		real amount = (real)sqrt(biped->unit.throttle.i * biped->unit.throttle.i +
			biped->unit.throttle.j * biped->unit.throttle.j);

		ground[0] = biped->biped.ground_plane.n.i;
		ground[1] = biped->biped.ground_plane.n.j;
		ground[2] = biped->biped.ground_plane.n.k;
		/* (the stick in the world: forward along the facing, sideways to
		its left) */
		steer[0] = forward->i * biped->unit.throttle.i - forward->j * biped->unit.throttle.j;
		steer[1] = forward->j * biped->unit.throttle.i + forward->i * biped->unit.throttle.j;
		speed = slide_step(parkour.slide_velocity, 1.0f / TICKS_PER_SECOND, airborne ? NULL : ground, steer,
			amount);
	}
	if (speed < SLIDE_END_SHARE * run)
	{
		slide_end("slowed");
		return;
	}
	parkour.slide_ticks++;
}

/* ---------- the walls

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
vr_parkour_math.h's vr_pk_runnable, vr_pk_blend_normal, vr_pk_wall_leap,
vr_pk_leap_hold and vr_pk_leap_start, and vr_parkour.c's probes. */

/* a face to run along or jump off: steeper than about 53 degrees (an
overhang as much); at the knee, a wall (steeper than about 70) */
static boolean wall_runnable(real_vector3d const *normal, boolean low)
{
	return low ? fabs(normal->k) < 0.35f : fabs(normal->k) < 0.6f;
}

/* the wall run along eases round a bend or over a seam (a sharp corner,
past about 80 degrees, taken whole) */
static void wall_blend_normal(real_vector3d *normal, real_vector3d const *met)
{
	if (!(dot_product3d(normal, met) > 0.17f))
	{
		*normal = *met;
		return;
	}
	normal->i = normal->i * 0.6f + met->i * 0.4f;
	normal->j = normal->j * 0.6f + met->j * 0.4f;
	normal->k = normal->k * 0.6f + met->k * 0.4f;
	if (normalize3d(normal) <= 0.001f)
		*normal = *met;
}

/* the leap off a wall where the head looks (look), turned off the wall
(normal) at least 35% of the way; as fast as the body went (velocity,
level) or at least speed, at most 1.5 times it; up as a jump does
(upward), more looking up (all per tick) */
static void wall_leap(real_vector3d const *look, real_vector3d const *normal, real_vector3d const *velocity,
	real speed, real upward, real_vector3d *out)
{
	real h[2], n[2], length, away, carried;

	n[0] = normal->i;
	n[1] = normal->j;
	length = (real)sqrt(n[0] * n[0] + n[1] * n[1]);
	if (!(length > 0.1f))
	{
		n[0] = 1.0f;
		n[1] = 0.0f;
	}
	else
	{
		n[0] /= length;
		n[1] /= length;
	}
	h[0] = look->i;
	h[1] = look->j;
	length = (real)sqrt(h[0] * h[0] + h[1] * h[1]);
	/* (looking straight up or down: away from the wall) */
	if (!(length > 0.15f))
	{
		h[0] = n[0];
		h[1] = n[1];
	}
	else
	{
		h[0] /= length;
		h[1] /= length;
	}
	away = h[0] * n[0] + h[1] * n[1];
	if (away < 0.35f)
	{
		/* (the look's way along the wall kept, 35% of it away from the
		wall; looking straight at the wall, straight off it) */
		real t[2];

		t[0] = h[0] - away * n[0];
		t[1] = h[1] - away * n[1];
		length = (real)sqrt(t[0] * t[0] + t[1] * t[1]);
		if (length > 0.001f)
		{
			h[0] = 0.35f * n[0] + 0.9367497f * t[0] / length;
			h[1] = 0.35f * n[1] + 0.9367497f * t[1] / length;
		}
		else
		{
			h[0] = n[0];
			h[1] = n[1];
		}
	}
	carried = (real)sqrt(velocity->i * velocity->i + velocity->j * velocity->j);
	carried = PIN(carried, speed, speed * 1.5f);
	out->i = h[0] * carried;
	out->j = h[1] * carried;
	out->k = upward + (look->k > 0.0f ? look->k * speed * 0.5f : 0.0f);
}

/* the leap held against the air control: across its way taken out, along
it no slower than its speed */
static void wall_leap_hold(real_vector3d *velocity)
{
	real along = velocity->i * parkour.leap_direction[0] + velocity->j * parkour.leap_direction[1];

	if (!(along >= parkour.leap_speed))
		along = parkour.leap_speed;
	velocity->i = parkour.leap_direction[0] * along;
	velocity->j = parkour.leap_direction[1] * along;
}

/* the nearest wall about the body (at the knee, the waist and the chest,
eight ways round, and toward the wall run along); FALSE, none */
static boolean wall_probe(long biped_index, real_point3d const *position, real height, real width,
	real_vector3d const *forward, struct collision_result *wall)
{
	static real const levels[3] = { 0.55f, 0.85f, 0.30f };
	real_vector3d left;
	boolean found = FALSE;
	short level, direction;

	left.i = -forward->j;
	left.j = forward->i;
	left.k = 0.0f;
	if (global_current_collision_user_depth >= MAXIMUM_COLLISION_USER_STACK_DEPTH)
		return FALSE;
	global_current_collision_users[global_current_collision_user_depth++] = _collision_user_bipeds;
	for (level = 0; level < 3; level++)
	{
		for (direction = 0; direction < 9; direction++)
		{
			real_vector3d ray;
			real_point3d from = *position;
			real reach = width + WALL_REACH * WORLD_UNITS_PER_METRE;
			struct collision_result hit;

			from.z += height * levels[level];
			if (direction < 8)
			{
				real angle = (real)direction * 0.7853982f;
				real c = (real)cos(angle), s = (real)sin(angle);

				ray.i = forward->i * c + left.i * s;
				ray.j = forward->j * c + left.j * s;
			}
			else
			{
				if (!parkour.running || !parkour.wall_valid)
					continue;
				ray.i = -parkour.wall_normal.i;
				ray.j = -parkour.wall_normal.j;
				reach = width + WALL_RUN_REACH * WORLD_UNITS_PER_METRE;
			}
			ray.k = 0.0f;
			if (normalize3d(&ray) < 0.001f)
				continue;
			scale_vector3d(&ray, reach, &ray);
			if (collision_test_vector(FLAG(_collision_test_front_facing_surfaces_bit) |
				FLAG(_collision_test_structure_bit) | FLAG(_collision_test_objects_bit) |
				FLAG(_collision_test_objects_scenery_bit), &from, &ray, biped_index, &hit) &&
				hit.t >= 0.0f && hit.t <= 1.0f && wall_runnable(&hit.plane.n, level == 2) &&
				(!found || hit.t < wall->t))
			{
				*wall = hit;
				found = TRUE;
			}
		}
	}
	--global_current_collision_user_depth;
	return found;
}

/* a tick in the air: the wall met, a wall jump, a wall run (velocity: the
tick's starting velocity, per tick, changed); TRUE, it was changed */
static boolean wall_tick(long biped_index, struct biped_datum *biped, real_point3d const *position, real height,
	real width, real_vector3d const *forward, boolean jump_pressed, real_vector3d *velocity)
{
	real per_second = WORLD_UNITS_PER_METRE / TICKS_PER_SECOND;
	real throttle = (real)sqrt(biped->unit.throttle.i * biped->unit.throttle.i +
		biped->unit.throttle.j * biped->unit.throttle.j);
	real_vector3d left, stick;
	struct collision_result wall;
	boolean found = wall_probe(biped_index, position, height, width, forward, &wall);

	left.i = -forward->j;
	left.j = forward->i;
	left.k = 0.0f;
	stick.i = forward->i * biped->unit.throttle.i + left.i * biped->unit.throttle.j;
	stick.j = forward->j * biped->unit.throttle.i + left.j * biped->unit.throttle.j;
	stick.k = 0.0f;
	if (found)
	{
		boolean changed = FALSE;

		if (parkour.wall_valid && dot_product3d(&parkour.wall_normal, &wall.plane.n) < 0.5f)
		{
			real_vector3d separation;

			/* (a run goes on round a bend: only a new wall off the ground
			starts again) */
			vector_from_points3d(&parkour.wall_point, &wall.point, &separation);
			if (magnitude3d(&separation) > 0.5f * WORLD_UNITS_PER_METRE && !parkour.running)
			{
				parkour.run_ticks = 0;
				parkour.jumped_wall = FALSE;
				changed = TRUE;
			}
		}
		if (!parkour.wall_valid || changed)
		{
			parkour.wall_normal = wall.plane.n;
			parkour.wall_point = wall.point;
			parkour.wall_valid = TRUE;
		}
		else if (parkour.running)
		{
			wall_blend_normal(&parkour.wall_normal, &wall.plane.n);
			parkour.wall_point = wall.point;
		}
		parkour.lost_ticks = 0;
	}
	else if (parkour.running && parkour.wall_valid && parkour.lost_ticks < WALL_RUN_GAP_TICKS)
	{
		/* (a run keeps over a gap in its wall, on the facing it had) */
		csmemset(&wall, 0, sizeof(wall));
		wall.point = parkour.wall_point;
		wall.plane.n = parkour.wall_normal;
		found = TRUE;
		parkour.lost_ticks++;
	}

	/* the wall jump: where the head looks, off the wall run along or the
	nearest met */
	if (jump_pressed && found && !parkour.jumped_wall)
	{
		real scale = parkour_settings.walljump;
		real upward = biped_definition_get(biped->definition_index)->biped.jump_velocity * scale;
		real_vector3d look, from_wall = parkour.running && parkour.wall_valid ? parkour.wall_normal : wall.plane.n;
		real_vector3d carried = *velocity;
		real speed;

		if (!vr_render_head_forward(&look))
			unit_get_aiming_vector(biped_index, &look);
		wall_leap(&look, &from_wall, &carried, WALL_JUMP_SPEED * per_second * scale, upward, velocity);
		speed = (real)sqrt(velocity->i * velocity->i + velocity->j * velocity->j);
		parkour.leap_ticks = 0;
		if (speed > 0.001f)
		{
			parkour.leap_direction[0] = velocity->i / speed;
			parkour.leap_direction[1] = velocity->j / speed;
			parkour.leap_speed = speed;
			parkour.leap_ticks = WALL_LEAP_HOLD_TICKS;
		}
		platform_log("vr parkour: wall jump%s at %.1f m/s", parkour.running ? " from a run" : "",
			(double)(magnitude3d(velocity) / per_second));
		parkour.jumped_wall = TRUE;
		parkour.running = FALSE;
		parkour.run_ticks = (long)(WALL_RUN_SECONDS * TICKS_PER_SECOND);
		parkour.walljump_age = 0;
		halo_vr_pulse(0.3f, 0.04f);
		return TRUE;
	}

	/* the wall run: steered away from its wall, it ends */
	if (parkour.running && found && throttle > 0.5f)
	{
		real_vector3d way = stick;

		if (normalize3d(&way) > 0.001f && dot_product3d(&way, &wall.plane.n) > 0.6f)
		{
			platform_log("vr parkour: wall run ended (steered away)");
			parkour.running = FALSE;
			parkour.run_ticks = (long)(WALL_RUN_SECONDS * TICKS_PER_SECOND);
			return FALSE;
		}
	}
	if (found && (parkour.running || throttle > WALL_RUN_STICK) &&
		(parkour.running || (parkour.jump_age <= WALL_RUN_ENTRY_TICKS && parkour.jump_age >= WALL_RUN_EARLIEST_TICKS) ||
		parkour.walljump_age <= 2 * TICKS_PER_SECOND) &&
		parkour.run_ticks < (long)(WALL_RUN_SECONDS * TICKS_PER_SECOND))
	{
		real speed = (real)sqrt(velocity->i * velocity->i + velocity->j * velocity->j);
		real_vector3d way;
		real into;

		/* (the stick let go: on the way the body already goes) */
		way = throttle > 0.3f ? stick : *velocity;
		way.k = 0.0f;
		into = dot_product3d(&way, &wall.plane.n);
		way.i -= into * wall.plane.n.i;
		way.j -= into * wall.plane.n.j;
		way.k = 0.0f;
		if (normalize3d(&way) > (throttle > 0.3f ? 0.15f : 0.001f) && speed > 1.0f * per_second)
		{
			real progress = (real)parkour.run_ticks / (WALL_RUN_SECONDS * TICKS_PER_SECOND);

			if (!parkour.running)
			{
				parkour.run_speed = speed;
				platform_log("vr parkour: wall run%s", parkour.walljump_age <= 2 * TICKS_PER_SECOND ? " (chained)" : "");
			}
			speed = PIN(MAX(parkour.run_speed, WALL_RUN_SLOWEST * per_second) * WALL_RUN_SPEED,
				WALL_RUN_SLOWEST * per_second, WALL_RUN_FASTEST * per_second);
			velocity->i = velocity->i * 0.65f + way.i * speed * 0.35f;
			velocity->j = velocity->j * 0.65f + way.j * speed * 0.35f;
			/* (a light hold toward the wall keeps the feet on it) */
			velocity->i -= wall.plane.n.i * 0.3f * per_second;
			velocity->j -= wall.plane.n.j * 0.3f * per_second;
			/* (level for the first 60% of its time, the tick's gravity
			taken back as it starts, then easing into the fall) */
			if (velocity->k < 0.0f)
			{
				if (progress < 0.6f)
					velocity->k = global_gravity * 0.5f;
				else
					velocity->k *= PIN((progress - 0.6f) / 0.4f, 0.0f, 1.0f);
			}
			parkour.running = TRUE;
			parkour.run_ticks++;
			parkour.leap_ticks = 0;
			return TRUE;
		}
	}
	if (parkour.running)
		platform_log("vr parkour: wall run ended");
	parkour.running = FALSE;
	return FALSE;
}

/* ---------- public code */

/* bipeds.c, as it makes a player's movement ready for a tick on foot
(biped_physics_prepare, the player's physics): the pill (its base, its
height and radius), the facing, the run's speed (world units a second) and
the velocity the tick starts from (per tick); TRUE, parkour changed that
velocity (a wall jump, a wall run). Nothing for any but the first local
player's biped, or without vr.parkour */
boolean vr_parkour_tick(
	long biped_index,
	real_point3d const *position,
	real height,
	real width,
	real_vector3d const *facing,
	real run_speed,
	real_vector3d *velocity)
{
	struct biped_datum *biped = parkour_biped(biped_index);
	long now = game_time_get();
	real_vector3d forward = *facing;
	boolean jump, jump_pressed, airborne, changed = FALSE;

	if (!parkour_initialized)
	{
		parkour_initialized = TRUE;
		parkour_reset();
	}
	if (!biped)
	{
		if (biped_index == parkour.unit_index)
		{
			slide_end("unavailable");
			parkour_reset();
		}
		return FALSE;
	}
	if (biped_index != parkour.unit_index || now < parkour.tick || now > parkour.tick + 1)
	{
		slide_end("another body or time");
		parkour_reset();
	}
	if (now == parkour.tick)
		return FALSE;
	parkour.unit_index = biped_index;
	parkour.tick = now;
	forward.k = 0.0f;
	if (normalize3d(&forward) <= 0.0f)
		forward = *global_forward3d;

	jump = TEST_FLAG(biped->unit.control_flags, _unit_control_jump_bit);
	jump_pressed = jump && !parkour.jump_down;
	parkour.jump_down = jump;
	parkour.jump_age = jump_pressed ? 0 : MIN(parkour.jump_age + 1, 1000);
	parkour.walljump_age = MIN(parkour.walljump_age + 1, 1000);
	airborne = TEST_FLAG(biped->biped.flags, _biped_airborne_bit);
	if (!airborne)
	{
		parkour.run_ticks = 0;
		parkour.jumped_wall = FALSE;
		parkour.wall_valid = FALSE;
		parkour.leap_ticks = 0;
		parkour.running = FALSE;
	}
	/* (the wall jump's leap goes where the head looked, not back along the
	stick) */
	if (parkour.leap_ticks > 0)
	{
		wall_leap_hold(velocity);
		parkour.leap_ticks--;
		changed = TRUE;
	}
	slide_tick(biped_index, biped, &forward, run_speed / WORLD_UNITS_PER_METRE);
	if (airborne && wall_tick(biped_index, biped, position, height, width, &forward, jump_pressed, velocity))
		changed = TRUE;
	return changed;
}

/* bipeds.c, the movement the first local player asks for in a tick (and in
each sub-step of game.sim_rate, its facing then): while sliding, the
slide's, reached at once (movement in the facing's frame, per tick; the
acceleration's maximum per tick) */
void vr_parkour_movement(
	long biped_index,
	real_vector3d const *facing,
	real_vector3d *movement_desired,
	real *acceleration_maximum)
{
	real_vector3d forward = *facing;
	real vx, vy, speed;

	if (!parkour.sliding || biped_index != parkour.unit_index)
		return;
	forward.k = 0.0f;
	if (normalize3d(&forward) <= 0.0f)
		return;
	vx = parkour.slide_velocity[0] * WORLD_UNITS_PER_METRE / TICKS_PER_SECOND;
	vy = parkour.slide_velocity[1] * WORLD_UNITS_PER_METRE / TICKS_PER_SECOND;
	movement_desired->i = vx * forward.i + vy * forward.j;
	movement_desired->j = -vx * forward.j + vy * forward.i;
	movement_desired->k = 0.0f;
	speed = (real)sqrt(vx * vx + vy * vy);
	if (*acceleration_maximum < speed)
		*acceleration_maximum = speed;
}

#endif
