"""Blood layers (game.blood_layers): a splat of blood is left out where that many unfaded splats of blood lie within its
radius, in its layer of its cluster; nothing else is."""
import sys
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from harness import CHECK_FAILED, build, constant, enum_with, function, inline, mutated, read, run, structure

CASES = ['count', 'uncounted']
CONTROLS = {
    # faded splats counted
    'faded': (('decal->intensity>=128 &&', ''), 'uncounted'),
    # the map's own decals counted
    'permanent': (('!TEST_FLAG(decal->flags, _decal_permanent_bit) &&', ''), 'uncounted'),
    # any decal counted, blood or not
    'any-decal': (('(decal->definition_index==decal_definition_index ||\n'
                   '\t\t\t\tdecal_definition_is_blood(decal->definition_index)) &&', ''), 'uncounted'),
    # every splat counted, however far
    'any-distance': (('distance_squared3d(&decal->position, &collision->point)<radius*radius &&', ''), 'count'),
    # the setting read once only
    'read-once': (('if (read_at!=config_changes())', 'if (read_at==(unsigned long)-1)'), 'count'),
}


def generated(control=None):
    decals = read('source/effects/decals.c')
    math = read('source/math/real_math.h')
    clusters = constant(read('source/structures/structures.h'), 'MAXIMUM_CLUSTERS_PER_STRUCTURE')
    config = '\n'.join([
        f'enum {{ MAXIMUM_CLUSTERS_PER_STRUCTURE = {clusters} }};',
        enum_with(decals, 'NUMBER_OF_DECAL_LAYERS'),
        enum_with(decals, '_decal_permanent_bit'),
        structure(decals, 'decal_datum'),
        structure(decals, 'decal_globals'),
    ]) + '\n'
    code = '\n'.join([inline(math, name) for name in ['vector_from_points3d', 'magnitude_squared3d',
                                                       'distance_squared3d']] +
                     [function(decals, name) for name in ['decal_get_first_decal_index', 'decal_definition_is_blood',
                                                          'decal_blood_layers_full']])
    if control:
        code = mutated(code, *CONTROLS[control][0])
    return (('config.inc', config), ('under_test.inc', code))


@pytest.mark.parametrize('case', CASES)
def test_case(case):
    status, output = run(build('decal_blood_layers', generated()), case)
    assert status == 0, output


@pytest.mark.parametrize('control', CONTROLS)
def test_negative_control(control):
    status, output = run(build('decal_blood_layers', generated(control)), CONTROLS[control][1])
    assert status == CHECK_FAILED and 'check failed:' in output, f'{control} escaped its CHECK: {status}: {output}'
