#include "harness.h"

/* decal_blood_layers_full (effects/decals.c, game.blood_layers): a splat of
blood is left out where that many splats of blood, not yet faded, lie within
its radius in its layer of the cluster it lands in; anything else is laid */
typedef unsigned short word;
typedef unsigned long pixel32;
#include "config.inc"

struct collision_result { struct { short cluster_index; } location; real_point3d point; };
struct decal_definition { short layer; real radius_upper_bound; };

/* the map's tags: 0 and 1 blood splats, 2 a bullet hole */
enum { BLOOD = 0, OTHER_BLOOD = 1, BULLET_HOLE = 2, NUMBER_OF_DEFINITIONS };
static char const *const names[NUMBER_OF_DEFINITIONS] = {
	"effects\\decals\\blood splats\\blood splat human",
	"effects\\decals\\blood splats\\blood splat elite",
	"effects\\decals\\bullet holes\\metal",
};
static struct decal_definition definitions[NUMBER_OF_DEFINITIONS] = { { 1, 0.1f }, { 1, 0.1f }, { 1, 0.1f } };
static char const *tag_get_name(long index) { return names[index]; }
static struct decal_definition *decal_definition_get(long index) { return &definitions[index]; }

static long blood_layers = 6;
static unsigned long changes = 1;
static long config_integer(const char *name) { return !strcmp(name, "game.blood_layers") ? blood_layers : 0; }
static unsigned long config_changes(void) { return changes; }

static struct decal_globals globals;
static struct decal_globals *decal_globals = &globals;
static struct decal_datum decals[64];
static long decal_count;
#define DECAL_GET(index) (&decals[index])

#include "under_test.inc"

/* a decal laid as decal_insert puts it at the head of its list */
static void lay(long definition, short cluster, short layer, real x, unsigned char intensity, boolean permanent)
{
	long index = decal_count++;
	struct decal_datum *decal = &decals[index];

	memset(decal, 0, sizeof(*decal));
	decal->flags = permanent ? FLAG(_decal_permanent_bit) : 0;
	decal->cluster_index = cluster;
	decal->layer = layer;
	decal->position.x = x;
	decal->intensity = intensity;
	decal->definition_index = definition;
	decal->previous_decal_index = NONE;
	decal->next_decal_index = globals.first_decal_indices[layer][cluster];
	globals.first_decal_indices[layer][cluster] = index;
}

static boolean full(long definition, short cluster, real x)
{
	struct collision_result collision;

	memset(&collision, 0, sizeof(collision));
	collision.location.cluster_index = cluster;
	collision.point.x = x;
	return decal_blood_layers_full(definition, &collision, 1.0f);
}

int main(int argc, char **argv)
{
	const char *case_name = argv[1];
	long index;

	memset(globals.first_decal_indices, 0xFF, sizeof(globals.first_decal_indices));
	CASE("count")
	{
		/* five, then the sixth: full; another species' blood counts too */
		for (index = 0; index < 5; index++)
			lay(BLOOD, 3, 1, 0.01f * index, 255, FALSE);
		CHECK(!full(BLOOD, 3, 0.0f), "full at five splats of six");
		lay(OTHER_BLOOD, 3, 1, 0.0f, 255, FALSE);
		CHECK(full(BLOOD, 3, 0.0f), "not full at six splats");
		CHECK(!full(BULLET_HOLE, 3, 0.0f), "a bullet hole left out");
		CHECK(!full(BLOOD, 4, 0.0f), "another cluster's blood counted");
		CHECK(!full(BLOOD, 3, 0.5f), "blood out of reach counted");
		blood_layers = 0;
		changes++;
		CHECK(!full(BLOOD, 3, 0.0f), "game.blood_layers 0 left a splat out");
		blood_layers = 7;
		changes++;
		CHECK(!full(BLOOD, 3, 0.0f), "full at six splats of seven");
	}
	CASE("uncounted")
	{
		/* faded, the map's own, another layer, not blood: none counts */
		for (index = 0; index < 5; index++)
			lay(BLOOD, 3, 1, 0.0f, 255, FALSE);
		lay(BLOOD, 3, 1, 0.0f, 100, FALSE);
		lay(BLOOD, 3, 1, 0.0f, 255, TRUE);
		lay(BLOOD, 3, 2, 0.0f, 255, FALSE);
		lay(BULLET_HOLE, 3, 1, 0.0f, 255, FALSE);
		CHECK(!full(BLOOD, 3, 0.0f), "a faded, permanent, other layer's or other decal counted");
		lay(BLOOD, 3, 1, 0.0f, 128, FALSE);
		CHECK(full(BLOOD, 3, 0.0f), "a splat half faded not counted");
	}
	return 0;
}
