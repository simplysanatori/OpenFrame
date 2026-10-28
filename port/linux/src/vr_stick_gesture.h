/*
VR_STICK_GESTURE.H

(OpenFrame) The turning stick pushed one way, as vr.c's comfort_update
reads it for sprint's toggle and bullet time (vr.sprint_mode = "toggle",
vr.bullet_time): a flick or a hold. No dependencies, so that a check can
drive it frame by frame.

A push is the stick inside a cone about the way: pushed past
STICK_UP_PRESSED with more along than across (within 45 degrees of it) to
begin, and kept while past STICK_UP_RELEASED with STICK_UP_STAY_ACROSS or
less across for each along (a thumb that holds a stick drifts, and a hold
of 0.8 s that a drift ended as a flick was never a hold: headset trial 7).
A dip out of the cone shorter than STICK_UP_GRACE_SECONDS is part of the
push, and so is a moment of the gesture not being able
(STICK_UP_ABLE_SECONDS: the game's frames that did not say the player is on
foot). The push's time is the real seconds from its first frame.

Where a hold means something (hold), a push let go before
STICK_UP_HOLD_SECONDS is a flick (when the grace after its release has
passed) and one held that long a hold (once, at that moment; its release is
nothing). Where it does not, a push is a flick at once, as it begins, and
its release is nothing. A push begun while the gesture is not able (menus, a
vehicle), or kept from it longer than STICK_UP_ABLE_SECONDS, is spent: it
does nothing, and only a new push after letting go counts.

When a push is over, ended is set for that call: ended_seconds is how long
it was held, ended_result what it came to (STICK_GESTURE_*, none: spent),
and ended_across and ended_along the most sideways and the least forward it
went (for the log).
*/

#ifndef VR_STICK_GESTURE_H
#define VR_STICK_GESTURE_H

/* (the cone's: the push's start, its keeping, and the ratio of across to
along it keeps; the third person's stick down uses the first three, plain) */
#define STICK_UP_PRESSED 0.75f
#define STICK_UP_RELEASED 0.55f
#define STICK_UP_SIDEWAYS 0.6f
#define STICK_UP_BEGIN 0.6f
#define STICK_UP_KEEP 0.4f
#define STICK_UP_STAY_ACROSS 1.4f
#define STICK_UP_HOLD_SECONDS 0.8f
#define STICK_UP_GRACE_SECONDS 0.15f
#define STICK_UP_ABLE_SECONDS 0.3f
#define STICK_UP_FRAME_MAX 0.05f

#define STICK_GESTURE_NONE 0
#define STICK_GESTURE_FLICK 1
#define STICK_GESTURE_HOLD 2

struct stick_gesture
{
	/* pushed now; for how long (seconds); this push already taken; the
	seconds out of the cone, and not able, in a row */
	int held;
	float seconds;
	int taken;
	float gap;
	float unable;
	/* the push's extremes, and the one just over */
	float across;
	float along;
	int ended;
	float ended_seconds;
	int ended_result;
	float ended_across;
	float ended_along;
	/* what the push has come to (a flick, a hold) */
	int result;
};

/* whether the stick is in the cone: along, the push the gesture's way (up:
the stick's y); across, its sideways push (its size) */
static inline int stick_gesture_inside(const struct stick_gesture *g, float along, float across)
{
	if (!g->held)
		return along >= STICK_UP_BEGIN && along > across;
	return along >= STICK_UP_KEEP && across <= along * STICK_UP_STAY_ACROSS;
}

/* one frame of the stick: able, whether the gesture means anything now;
hold, whether a hold means anything; seconds, the frame's real time.
STICK_GESTURE_* */
static inline int stick_gesture_update(struct stick_gesture *g, int able, float along, float across, int hold,
	float seconds)
{
	int result = STICK_GESTURE_NONE;
	int pressed = stick_gesture_inside(g, along, across);

	/* (a hitch of the game is no hold) */
	if (seconds > STICK_UP_FRAME_MAX)
		seconds = STICK_UP_FRAME_MAX;
	g->ended = 0;
	if (!pressed && g->held && g->gap + seconds < STICK_UP_GRACE_SECONDS)
	{
		/* (a dip: part of the push) */
		g->gap += seconds;
		g->seconds += seconds;
		if (!able)
			g->unable += seconds;
		return STICK_GESTURE_NONE;
	}
	if (!pressed)
	{
		if (g->held)
		{
			if (g->unable < STICK_UP_ABLE_SECONDS && !g->taken)
			{
				result = STICK_GESTURE_FLICK;
				g->result = result;
			}
			g->ended = 1;
			g->ended_seconds = g->seconds - g->gap;
			g->ended_result = g->result;
			g->ended_across = g->across;
			g->ended_along = g->along;
		}
		g->held = 0;
		g->seconds = 0.0f;
		g->taken = 0;
		g->gap = 0.0f;
		g->unable = 0.0f;
		g->result = STICK_GESTURE_NONE;
		return result;
	}
	if (!g->held)
	{
		g->seconds = 0.0f;
		g->unable = 0.0f;
		g->across = across;
		g->along = along;
		g->result = STICK_GESTURE_NONE;
		/* (begun not able: spent) */
		g->taken = !able;
	}
	else
	{
		g->seconds += seconds;
		if (across > g->across)
			g->across = across;
		if (along < g->along)
			g->along = along;
	}
	g->held = 1;
	g->gap = 0.0f;
	g->unable = able ? 0.0f : g->unable + seconds;
	if (g->unable >= STICK_UP_ABLE_SECONDS)
		g->taken = 1;
	if (g->taken || !able)
		return STICK_GESTURE_NONE;
	if (!hold)
		result = STICK_GESTURE_FLICK;
	else if (g->seconds >= STICK_UP_HOLD_SECONDS)
		result = STICK_GESTURE_HOLD;
	if (result != STICK_GESTURE_NONE)
	{
		g->taken = 1;
		g->result = result;
	}
	return result;
}

#endif
