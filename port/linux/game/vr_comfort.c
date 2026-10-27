/*
VR_COMFORT.C

The VR mode's sprint (HALO_VR; vr.sprint_mode, vr.sprint_speed;
docs/openframe-features.md): the game's side of it, for bipeds.c. vr.c
reads the controllers (comfort_update): the right stick flicked up toggles
sprint, and with vr.sprint_mode = "arm_swing" both arms pumping are a run's
effort.

It is a game of this computer alone (game_connection: the campaign),
never a network game's host or client, split screen or a game with bots
(each a network game, which every machine must step alike), nor a film's
playback.

Sprint quickens the first local player's running on foot, standing (not
crouched), as bipeds.c makes each tick's speeds: forward and sideways (not
backing), and the acceleration with them, by vr.sprint_speed at a full
effort. The view's field is unchanged.

Ported from HaloCE-DecompRemix-VR by moistman42069 (AGPL-3.0-or-later):
its sprint (vr_frame.c's button sprint and arm run, vr_render.c's
vr_player_sprint_scale), made again for this port's VR layer.
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

static struct
{
	/* whether sprint's toggle was on last frame */
	boolean sprint;
} vr_comfort = { FALSE };

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

/* main.c, once a frame of a game in progress */
void vr_comfort_frame(
	void)
{
	boolean local = game_connection() == _game_connection_local;

	/* sprint's toggle, said on the HUD as it comes on (not as it ends,
	with every stop) */
	if (halo_vr_sprint_toggled() != vr_comfort.sprint)
	{
		vr_comfort.sprint = !vr_comfort.sprint;
		if (vr_comfort.sprint)
			comfort_message(local ? L"Sprint" : L"Sprint: offline only");
	}
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

#endif
