#!/usr/bin/env python3
"""Places the night bloom's spore trails in Viridian Forest (Act 2's Grass course, docs/hack/act2_story.md).

Each trail leads from the last glowcap (or the forest's entrance) to the next one along the walkable way, a spore
every SPORE_SPACING squares, and always one right beside the glowcap it leads to. The spores are light sprites
(OBJ_EVENT_GFX_LIGHT_SPRITE, LIGHT_TYPE_GLOWCAP_SPORE) shown while their trail's flag is clear (SetGlowcapTrail in
src/act2_night_bloom.c). They blink in a row toward the glowcap: each spore's place in its trail goes in its movement
range X (0 first), and the timing is src/event_object_movement.c's GLOWCAP_SPORE_BLINK_*.
Re-running rewrites the forest's spores (they go at the end of its object events).

    python3 tools/hack/place_glowcap_spores.py
"""
import json
import os

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
MAP = os.path.join(ROOT, 'data/maps/ViridianForest_Frlg/map.json')

SPORE_SPACING = 6  # Squares between spores along a trail

# Each trail: its flag, the way (corners, walkable squares) and the square beside the glowcap it leads to
TRAILS = [
    ('FLAG_TEMP_11', [(29, 61), (29, 59)], (31, 59)),                       # Entrance -> glowcap 1 (31,58)
    ('FLAG_TEMP_12', [(31, 57), (41, 57), (41, 46)], (40, 46)),             # Glowcap 1 -> glowcap 2 (39,45)
    ('FLAG_TEMP_13', [(41, 45), (41, 30)], (40, 30)),                       # Glowcap 2 -> glowcap 3 (39,29)
]


def walk(corners):
    """The squares along the way, corner to corner (straight lines), the first excluded"""
    squares = []
    for (x0, y0), (x1, y1) in zip(corners, corners[1:]):
        dx, dy = (x1 > x0) - (x1 < x0), (y1 > y0) - (y1 < y0)
        x, y = x0, y0
        while (x, y) != (x1, y1):
            x, y = x + dx, y + dy
            squares.append((x, y))
    return squares


def spores(corners, last):
    way = walk(corners)
    placed = [way[i] for i in range(SPORE_SPACING - 1, len(way), SPORE_SPACING)]
    # Not crowding the last one, beside the glowcap
    placed = [s for s in placed if abs(s[0] - last[0]) + abs(s[1] - last[1]) > SPORE_SPACING // 2]
    return placed + [last]


def light(x, y, flag, order):
    return {'type': 'object', 'graphics_id': 'OBJ_EVENT_GFX_LIGHT_SPRITE', 'x': x, 'y': y, 'elevation': 3,
            'movement_type': 'MOVEMENT_TYPE_NONE', 'movement_range_x': order, 'movement_range_y': 1,
            'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': 'LIGHT_TYPE_GLOWCAP_SPORE',
            'script': '0', 'flag': flag}


def main():
    d = json.load(open(MAP))
    d['object_events'] = [o for o in d['object_events'] if o['graphics_id'] != 'OBJ_EVENT_GFX_LIGHT_SPRITE']
    for flag, corners, last in TRAILS:
        for order, (x, y) in enumerate(spores(corners, last)):
            assert order < 16  # movement range X is 4 bits
            d['object_events'].append(light(x, y, flag, order))
        print(flag, spores(corners, last))
    open(MAP, 'w').write(json.dumps(d, indent=2, ensure_ascii=False) + '\n')


if __name__ == '__main__':
    main()
