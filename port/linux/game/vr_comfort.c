/*
VR_COMFORT.C

The VR mode's sprint and bullet time (HALO_VR; vr.sprint_mode,
vr.sprint_speed, vr.bullet_time, vr.bullet_time_scale;
docs/openframe-features.md): the game's side of them, for main.c and
bipeds.c. vr.c reads the controllers (comfort_update): the right stick
flicked up toggles sprint (or ends bullet time while it is on), held up
four fifths of a second toggles bullet time, and with vr.sprint_mode =
"arm_swing" both arms pumping are a run's effort. The HUD says each:
"Sprint", "Sprint off", "Bullet time", "Bullet time off".

Both are a game of this computer alone (game_connection: the campaign),
never a network game's host or client, split screen or a game with bots
(each a network game, which every machine must step alike), nor a film's
playback.

Bullet time slows the game's clock, not the frames': main.c gives
game_time_update the frame's time times vr_comfort_frame's scale, so the
ticks (and game.sim_rate's sub-steps) are the same steps as ever, only
fewer of them a second, and the frames between them are drawn as far into
the next as that slower clock has run (render_interpolation.c, by
game_time_get_tick_fraction), while the head and hands are drawn from the
headset at the display's rate. What the frames move by their own time (the
effects, game_frame) slows with it. Nothing of it is in the game state: a
checkpoint, a revert or a saved game knows nothing of it, and a revert
(the game's time going back) turns it off. It eases in and out over about
a sixth of a second, and ends at once in a cinematic, at death or where
the game takes the player's control. The player's own body is of the
world: in bullet time it runs, falls and jumps slowed with it (OpenFrame,
headset trial 3: bullet time taken by a push held a little long, the run
at 40% of the sprint's and the jumps floating, was felt as a slow sprint). The sounds keep their pitch (their
voices play at the mixer's rate): a slowed world sounds as fast as ever.

Sprint quickens the first local player's running on foot, standing (not
crouched), as bipeds.c makes each tick's speeds: forward and sideways (not
backing), and the acceleration with them, by vr.sprint_speed at a full
effort. The view's field is unchanged.

A crouch pauses the sprint, it does not end it: halo_vr_sprint (the
effort) and vr_sprint_speed_scale go on answering for a toggled sprint
(stick forward, not backing or sideways) while the player is crouched or
sliding (vr_parkour.c reads them through it), bipeds.c just does not apply
the scale to a crouched biped; as the player stands, the speeds scale again
by themselves, the toggle never having changed. Stopping does not end it
either; only a flick, the mode, or half a second off foot does.

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
its bullet time (vr_bullet_time.c: a local time scale under the game's own
speed, eased in and out, local games only) and its sprint (vr_frame.c's
button sprint and arm run, vr_render.c's vr_player_sprint_scale), made
again for this port's clock (game.sim_rate's sub-steps and the frames
between ticks) and its VR layer.
*/

#ifdef HALO_VR

#include "cseries.h"
#include "game/game.h"
#include "game/players.h"
#include "camera/director.h"
#include "cutscene/cinematics.h"
#include "interface/hud_messaging.h"
#include "objects/objects.h"

#include "../src/vr.h"

#include <math.h>

/* the platform layer's */
void platform_log(char const *format, ...);

/* bullet time eases toward its scale with this time constant (seconds):
about a sixth of a second in all */
#define BULLET_TIME_EASE_SECONDS 0.06f

static struct
{
	/* the scale the game's clock runs at now (1: none), whether bullet time
	is on (as the HUD last said), the game's time last frame (NONE: none
	yet), and whether sprint's toggle was on last frame */
	real scale;
	boolean bullet_time;
	long game_time;
	boolean sprint;
} vr_comfort = { 1.0f, FALSE, NONE, FALSE };

/* the first local player's unit, alive, in the player's control, else NONE */
static long comfort_player_unit(void)
{
	long player_index = local_player_get_player_index(0);
	long unit_index = player_index != NONE ? player_get(player_index)->unit_index : NONE;
	struct object_datum *unit = unit_index != NONE ? object_try_and_get(unit_index) : NULL;

	if (!unit || TEST_FLAG(unit->object.damage_flags, _object_dead_bit) || cinematic_in_progress() ||
		director_inhibited_input(0))
	{
		return NONE;
	}
	return unit_index;
}

static void comfort_message(wchar_t const *text)
{
	if (local_player_get_player_index(0) != NONE)
		hud_print_message(0, text);
}

/* main.c, once a frame of a game in progress, before the game's clock
runs (game_time_update): the frame's time (seconds); the scale of it the
game's clock runs, 1 but in bullet time */
real vr_comfort_frame(
	real seconds)
{
	boolean local = game_connection() == _game_connection_local;
	boolean allowed = local && halo_vr_running() && comfort_player_unit() != NONE;
	long now = game_time_get();
	real target;

	/* (the game's time gone back: a revert, a saved game loaded, a map
	begun) */
	if (vr_comfort.game_time != NONE && now < vr_comfort.game_time)
		allowed = FALSE;
	vr_comfort.game_time = now;

	/* sprint's toggle, said on the HUD as it comes on and as it goes off
	(a flick, off foot, its mode changed) */
	if (halo_vr_sprint_toggled() != vr_comfort.sprint)
	{
		vr_comfort.sprint = !vr_comfort.sprint;
		comfort_message(!vr_comfort.sprint ? L"Sprint off" : local ? L"Sprint" : L"Sprint: offline only");
	}

	target = halo_vr_bullet_time();
	if (target < 1.0f && !allowed)
	{
		halo_vr_bullet_time_off();
		target = 1.0f;
		if (!local && !vr_comfort.bullet_time)
			comfort_message(L"Bullet time: offline only");
	}
	if ((target < 1.0f) != vr_comfort.bullet_time)
	{
		vr_comfort.bullet_time = target < 1.0f;
		comfort_message(vr_comfort.bullet_time ? L"Bullet time" : L"Bullet time off");
	}
	if (!allowed)
	{
		vr_comfort.scale = 1.0f;
	}
	else if (seconds > 0.0f)
	{
		vr_comfort.scale += (target - vr_comfort.scale) * (1.0f - expf(-seconds / BULLET_TIME_EASE_SECONDS));
		if (fabsf(vr_comfort.scale - target) < 0.01f)
			vr_comfort.scale = target;
	}
	return vr_comfort.scale;
}

/* bipeds.c, as it makes a player's speeds for a tick (on foot, standing):
how many times faster than the run this biped's sprint runs it, 1 for any
but the first local player's, or not now */
real vr_sprint_speed_scale(
	long biped_index)
{
	long player_index = local_player_get_player_index(0);
	real effort;

	if (game_connection() != _game_connection_local || player_index == NONE ||
		player_get(player_index)->unit_index != biped_index || comfort_player_unit() == NONE)
	{
		return 1.0f;
	}
	effort = halo_vr_sprint();
	if (!(effort > 0.0f))
		return 1.0f;
	if (effort > 1.0f)
		effort = 1.0f;
	return 1.0f + (halo_vr_sprint_speed() - 1.0f) * effort;
}

/* bipeds.c, after its sprint's scaling of a player's speeds for a tick:
the scale, the forward run's speed (world units a second) and the
acceleration (bipeds.c's: a tick's velocity change is a 30th of it) before
and after it. The first local player's, logged as a sprint starts and ends,
so that a trial's stdout.log shows what the sprint did */
void vr_sprint_speeds_log(
	long biped_index,
	real scale,
	real forward_speed,
	real scaled_forward_speed,
	real acceleration,
	real scaled_acceleration)
{
	static boolean sprinting = FALSE;
	long player_index = local_player_get_player_index(0);

	if (player_index == NONE || player_get(player_index)->unit_index != biped_index ||
		(scale > 1.0f) == sprinting)
	{
		return;
	}
	sprinting = scale > 1.0f;
	platform_log("vr: sprint x%.2f: forward speed %.3f -> %.3f world units/s (%.2f -> %.2f m/s), "
		"acceleration %.3f -> %.3f (full speed in %.2f -> %.2f s)",
		(double)scale, (double)forward_speed, (double)scaled_forward_speed,
		(double)(forward_speed * 3.048f), (double)(scaled_forward_speed * 3.048f),
		(double)acceleration, (double)scaled_acceleration,
		acceleration > 0.0f ? (double)(forward_speed / acceleration / 30.0f) : 0.0,
		scaled_acceleration > 0.0f ? (double)(scaled_forward_speed / scaled_acceleration / 30.0f) : 0.0);
}

#endif
