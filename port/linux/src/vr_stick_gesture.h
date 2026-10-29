/*
VR_STICK_GESTURE.H

(OpenFrame) The turning stick pushed one way, as vr.c's comfort_update
reads it for sprint's toggle and bullet time (vr.sprint_mode = "toggle",
vr.bullet_time): a flick or a hold. No dependencies, so that a check can
drive it frame by frame.

A push is the stick inside a cone about the way: pushed past
STICK_UP_BEGIN with STICK_UP_BEGIN_RATIO times as much along as across
(within about 34 degrees of it: a diagonal push of a look around never
begins one, headset trial 9) to begin, and kept while past STICK_UP_RELEASED with STICK_UP_STAY_ACROSS or
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

With hold 2 (vr.bullet_time_binding = "double_flick" or "off", and the
third person's double flick down): a push is a flick at once, as it begins,
and it goes on to be a hold as well if held STICK_UP_HOLD_SECONDS (the
result then is the hold, the flick before it already given): whoever acts
on the flick undoes it at the hold, so that a held stick, a camera script's
second of it, does nothing in the end. stick_double (below) reads two
flicks quickly in a row.

The stick pushed down (stick_down_update below; it is the third person's hold,
and the flicks of vr.bullet_time_binding = "double_flick_down", which is not
the default: the default, "stick_click", is the stick's click, so the push
down is no bullet time's, and a flick up is sprint's whatever the binding): a
flick is told as the push is let go (hold 1: a push let go before
STICK_UP_HOLD_SECONDS), so that a hold, which is the third person's, never
is one. The third person's hold is a gesture of its own in vr.c with
loose set (it begins as the cone used to, down more than sideways, so a push
that began on a diagonal and settled still counts, and its keeping cone is
the generous one: trial 9's holds drifted to 0.5 sideways and 0.5 down, which
the old, tighter hold cone ended unheard), so a push it has taken is no flick
either, whichever of the two ends first.

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
/* (to begin a push: along must be this many times across; a loose gesture,
the third person's hold, begins with just more along than across) */
#define STICK_UP_BEGIN_RATIO 1.5f
#define STICK_UP_KEEP 0.4f
#define STICK_UP_STAY_ACROSS 1.4f
#define STICK_UP_HOLD_SECONDS 0.8f
#define STICK_UP_GRACE_SECONDS 0.15f
#define STICK_UP_ABLE_SECONDS 0.3f
#define STICK_UP_FRAME_MAX 0.05f

#define STICK_GESTURE_NONE 0
#define STICK_GESTURE_FLICK 1
#define STICK_GESTURE_HOLD 2
/* the hold parameter: no hold (a flick at once), a hold or a flick (let go
early), a flick at once and a hold as well */
#define STICK_HOLD_NONE 0
#define STICK_HOLD_OR_FLICK 1
#define STICK_HOLD_AFTER_FLICK 2
/* two flicks quickly in a row: the seconds from the first's end */
#define STICK_DOUBLE_SECONDS 0.35f

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
	/* seconds of the push's last frames out of the cone (its grace), and
	whether the flick of a hold of 2 was given */
	float ended_gap;
	int flicked;
	/* what the push has come to (a flick, a hold) */
	int result;
	/* set by the caller: begins on more along than across (the third
	person's hold), not STICK_UP_BEGIN_RATIO times */
	int loose;
};

/* whether the stick is in the cone: along, the push the gesture's way (up:
the stick's y); across, its sideways push (its size) */
static inline int stick_gesture_inside(const struct stick_gesture *g, float along, float across)
{
	if (!g->held)
		return along >= STICK_UP_BEGIN &&
			(g->loose ? along > across : along >= across * STICK_UP_BEGIN_RATIO);
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
			if (g->unable < STICK_UP_ABLE_SECONDS && !g->taken && !g->flicked)
			{
				result = STICK_GESTURE_FLICK;
				g->result = result;
			}
			g->ended = 1;
			g->ended_seconds = g->seconds - g->gap;
			g->ended_result = g->result;
			g->ended_across = g->across;
			g->ended_along = g->along;
			g->ended_gap = g->gap + seconds;
		}
		g->held = 0;
		g->flicked = 0;
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
		g->flicked = 0;
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
	if (hold == STICK_HOLD_AFTER_FLICK)
	{
		/* (the flick as the push begins, the hold when it has lasted; a
		push of a flick that is let go is nothing more) */
		if (g->seconds >= STICK_UP_HOLD_SECONDS)
		{
			g->taken = 1;
			g->result = STICK_GESTURE_HOLD;
			return STICK_GESTURE_HOLD;
		}
		if (!g->flicked)
		{
			g->flicked = 1;
			g->result = STICK_GESTURE_FLICK;
			return STICK_GESTURE_FLICK;
		}
		return STICK_GESTURE_NONE;
	}
	if (hold == STICK_HOLD_NONE)
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

/* the pushes that did not begin because they were diagonal (along past
STICK_UP_BEGIN and more than across, but not STICK_UP_BEGIN_RATIO times it:
what the old cone took for a push), told for the log as the stick leaves
them: ended is set for that call, with the seconds and the extremes. A push
that goes on to begin (the thumb settles) is no diagonal. */
struct stick_diag
{
	int active;
	int ended;
	float seconds;
	float across;
	float along;
	float ended_seconds;
	float ended_across;
	float ended_along;
};

static inline void stick_diag_update(struct stick_diag *d, const struct stick_gesture *g, float along, float across,
	float seconds)
{
	int diagonal = !g->held && along >= STICK_UP_BEGIN && along > across && along < across * STICK_UP_BEGIN_RATIO;

	d->ended = 0;
	if (seconds > STICK_UP_FRAME_MAX)
		seconds = STICK_UP_FRAME_MAX;
	if (g->held)
	{
		d->active = 0;
		return;
	}
	if (diagonal)
	{
		if (!d->active)
		{
			d->active = 1;
			d->seconds = 0.0f;
			d->across = across;
			d->along = along;
		}
		else
		{
			d->seconds += seconds;
			if (across > d->across)
				d->across = across;
			if (along < d->along)
				d->along = along;
		}
	}
	else if (d->active && along < STICK_UP_KEEP)
	{
		d->active = 0;
		d->ended = 1;
		d->ended_seconds = d->seconds;
		d->ended_across = d->across;
		d->ended_along = d->along;
	}
}

/* one frame of the stick pushed down for bullet time (flick_down): along is
the push down (-y), third_taken what the caller sets when the third person's
hold took this push (cleared here when the push is over). STICK_GESTURE_FLICK
once, as a flick is let go; a hold (or a push the third person took) is
nothing. */
static inline int stick_down_update(struct stick_gesture *g, int *third_taken, int able, float along, float across,
	float seconds)
{
	int result = stick_gesture_update(g, able, along, across, STICK_HOLD_OR_FLICK, seconds);

	if (result == STICK_GESTURE_HOLD)
		result = STICK_GESTURE_NONE;
	if (result == STICK_GESTURE_FLICK && *third_taken)
		result = STICK_GESTURE_NONE;
	if (!g->held)
		*third_taken = 0;
	return result;
}

/* two flicks quickly in a row (the double flick): the first flick acts at
once (kind says how: STICK_KIND_*), and if the second begins within
STICK_DOUBLE_SECONDS of the first's end, the caller undoes the first and
does the double's. A hold (hold 2) that comes of the first's push undoes it
and ends the chance. */
#define STICK_KIND_NOTHING 0
#define STICK_KIND_TOGGLED 1
#define STICK_ACT_NONE 0
/* a flick on its own: the caller acts, then sets kind */
#define STICK_ACT_FLICK 1
/* the second flick: undo the first's toggle (kind TOGGLED), and go on to
the double's */
#define STICK_ACT_DOUBLE 2
/* the push of the first flick came to a hold: undo it by kind */
#define STICK_ACT_HOLD 3

struct stick_double
{
	/* a flick done and its push still held or its chance open; the push let
	go; seconds since it ended; what the flick did */
	int armed;
	int released;
	float since;
	int kind;
};

/* one frame: gesture, what stick_gesture_update returned; the gesture's
ended flag and seconds out of the cone at its end; seconds the frame's;
doubles, whether two flicks in a row mean something (else only the hold's
undo does) */
static inline int stick_double_update(struct stick_double *d, int gesture, int ended, float ended_gap, float seconds,
	int doubles)
{
	if (d->armed && !d->released && ended)
	{
		d->released = 1;
		d->since = ended_gap;
	}
	else if (d->armed && d->released)
	{
		d->since += seconds;
	}
	if (d->armed && d->released && d->since > STICK_DOUBLE_SECONDS)
		d->armed = 0;
	if (gesture == STICK_GESTURE_HOLD)
	{
		int act = d->armed && !d->released ? STICK_ACT_HOLD : STICK_ACT_NONE;

		if (act == STICK_ACT_HOLD)
			d->armed = 0;
		return act;
	}
	if (gesture == STICK_GESTURE_FLICK)
	{
		if (doubles && d->armed && d->released)
		{
			d->armed = 0;
			return STICK_ACT_DOUBLE;
		}
		d->armed = 1;
		d->released = 0;
		d->since = 0.0f;
		d->kind = STICK_KIND_NOTHING;
		return STICK_ACT_FLICK;
	}
	return STICK_ACT_NONE;
}

#endif
