/*
VR_STICK_GESTURE.H

(OpenFrame) The turning stick pushed one way, as vr.c's comfort_update
reads it for sprint's toggle and bullet time (vr.sprint_mode = "toggle",
vr.bullet_time): a flick or a hold. No dependencies, so that a check can
drive it frame by frame.

The stick counts as pushed past STICK_UP_PRESSED (and let go under
STICK_UP_RELEASED) with less than STICK_UP_SIDEWAYS sideways (not a turn).
Where a hold means something (hold), a push let go before
STICK_UP_HOLD_SECONDS is a flick and one held that long a hold (once, at
that moment; its release is nothing). Where it does not, a push is a flick
at once, as it crosses STICK_UP_PRESSED, and its release is nothing. A push
begun or held while the gesture is not able (menus, a vehicle) is spent: it
does nothing, and only a new push after letting go counts.
*/

#ifndef VR_STICK_GESTURE_H
#define VR_STICK_GESTURE_H

#define STICK_UP_PRESSED 0.75f
#define STICK_UP_RELEASED 0.55f
#define STICK_UP_SIDEWAYS 0.6f
#define STICK_UP_HOLD_SECONDS 0.8f

#define STICK_GESTURE_NONE 0
#define STICK_GESTURE_FLICK 1
#define STICK_GESTURE_HOLD 2

struct stick_gesture
{
	/* pushed now; for how long (seconds); this push already taken */
	int held;
	float seconds;
	int taken;
};

/* one frame of the stick: along, its push the gesture's way (up: the
stick's y); across, its sideways push (its size); hold, whether a hold
means anything; seconds, the frame's time. STICK_GESTURE_* */
static inline int stick_gesture_update(struct stick_gesture *g, int able, float along, float across, int hold,
	float seconds)
{
	int pressed = across < STICK_UP_SIDEWAYS && along > (g->held ? STICK_UP_RELEASED : STICK_UP_PRESSED);
	int result = STICK_GESTURE_NONE;

	if (!pressed)
	{
		if (able && g->held && !g->taken)
			result = STICK_GESTURE_FLICK;
		g->held = 0;
		g->seconds = 0.0f;
		g->taken = 0;
		return result;
	}
	g->seconds = g->held ? g->seconds + seconds : 0.0f;
	g->held = 1;
	if (!able || g->taken)
	{
		g->taken = 1;
		return STICK_GESTURE_NONE;
	}
	if (!hold)
		result = STICK_GESTURE_FLICK;
	else if (g->seconds >= STICK_UP_HOLD_SECONDS)
		result = STICK_GESTURE_HOLD;
	if (result != STICK_GESTURE_NONE)
		g->taken = 1;
	return result;
}

#endif
