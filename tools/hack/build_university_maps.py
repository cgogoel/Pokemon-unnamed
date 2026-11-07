#!/usr/bin/env python3
"""Lays out Celadon University with its own tilesets (docs/hack/university_remake/plan.md): the 76x64 campus and every
room at the room size rule, from tools/hack/university_stamps.json (written by draw_university_tiles.py, run that
first), and moves every warp, person, sign and trigger to its new place.

    python3 tools/hack/build_university_maps.py

Writes the LAYOUT_CELADON_UNIVERSITY* map.bin/border.bin files and their entries in layouts.json (sizes, tilesets),
and the events in data/maps/CeladonUniversity*/map.json. Re-running overwrites the layouts (Porymap edits included).
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import draw_university_tiles as D  # noqa: E402

ROOT = D.ROOT
SOLID, PASSABLE = 1 << 10, 3 << 12
CAMPUS_MAP = 'MAP_CELADON_UNIVERSITY'
U = 'MAP_CELADON_UNIVERSITY_'

# --- Warps -------------------------------------------------------------------------------------------------------------
# Campus doors: room -> campus warp id. 0-1 are the gate's passage.
CAMPUS_DOORS = ['MAIN_HALL', 'LECTURE_HALL_A', 'LECTURE_HALL_B', 'FACULTY_OFFICE', 'CAFETERIA', 'LITERATURE',
                'LIBRARY', 'SCIENCE_INSTITUTE', 'HOSPITAL', 'DORM_LOBBY', 'DORM_ROOM', 'HOME_ECONOMICS']
# Which stamp door leads where: (stamp index in D.CAMPUS, door index in the stamp)
DOOR_OF = {'MAIN_HALL': (7, 0), 'LECTURE_HALL_A': (6, 0), 'LECTURE_HALL_B': (8, 0), 'FACULTY_OFFICE': (5, 0),
           'CAFETERIA': (9, 0), 'LITERATURE': (1, 0), 'LIBRARY': (1, 1), 'SCIENCE_INSTITUTE': (4, 0),
           'HOSPITAL': (12, 0), 'DORM_LOBBY': (15, 0), 'DORM_ROOM': (16, 0), 'HOME_ECONOMICS': (2, 0)}
# Stairs between floors: (room, its warp cells) <-> (room, its warp cells), in order
STAIRS = [
    ('MAIN_HALL', [(1, 3), (2, 3), (16, 3), (17, 3)], 'MAIN_HALL_2F', [(1, 3), (2, 3), (16, 3), (17, 3)]),
    ('SCIENCE_INSTITUTE', [(14, 2), (15, 2)], 'SCIENCE_LOUNGE', [(14, 2), (15, 2)]),
    ('SCIENCE_LOUNGE', [(1, 2), (2, 2)], 'SCIENCE_RESEARCH_ROOM', [(1, 2), (2, 2)]),
    ('SCIENCE_RESEARCH_ROOM', [(14, 2), (15, 2)], 'SCIENCE_EXPERIMENT_ROOM', [(14, 2), (15, 2)]),
    ('HOSPITAL', [(14, 2), (15, 2)], 'HOSPITAL_2F', [(14, 2), (15, 2)]),
    ('DORM_LOBBY', [(1, 2), (2, 2)], 'DORM_2F', [(1, 2), (2, 2)]),
    ('DORM_2F', [(18, 2), (19, 2)], 'DORM_3F', [(18, 2), (19, 2)]),
]

# --- People, signs, triggers: new positions ------------------------------------------------------------------------
# Objects by local_id or script (several objects sharing a script: a list, in map order)
OBJECTS = {
    'CeladonUniversity': {
        'CeladonUniversity_EventScript_Lass0': (34, 40), 'CeladonUniversity_EventScript_Youngster1': (37, 52),
        'LOCALID_CAMPUS_RUNNER': (12, 8), 'LOCALID_CAMPUS_STADIUM_STUDENT': (10, 12),
        'CeladonUniversity_EventScript_Scientist4': (60, 22), 'CeladonUniversity_EventScript_Man5': (5, 30),
        'CeladonUniversity_EventScript_Ponyta6': (51, 12), 'CeladonUniversity_EventScript_Ponyta7': (55, 14),
        'CeladonUniversity_EventScript_SwimmerFWater8': (11, 46), 'CeladonUniversity_EventScript_Psyduck9': (12, 49),
        'CeladonUniversity_EventScript_Beauty10': (44, 18), 'CeladonUniversity_EventScript_Gentleman11': (30, 18),
        'CeladonUniversity_EventScript_Boy12': (44, 46), 'CeladonUniversity_EventScript_Woman213': (23, 51),
        'CeladonUniversity_EventScript_BaldingMan14': (39, 44),
        # Waiting outside the player's room (the annex door is (56,57); the player steps out to (56,58))
        'LOCALID_CAMPUS_KAI': (53, 58), 'LOCALID_CAMPUS_MINA': (54, 58), 'LOCALID_CAMPUS_ROWAN': (59, 58),
        # The club, on the stadium's infield
        'LOCALID_CAMPUS_NIGHT_KAI': (15, 10), 'LOCALID_CAMPUS_NIGHT_MINA': (13, 11),
        'LOCALID_CAMPUS_NIGHT_ROWAN': (17, 11), 'LOCALID_CAMPUS_DALE': (15, 9),
    },
    'CeladonUniversity_MainHall': {
        'CeladonUniversity_MainHall_EventScript_Woman10': (3, 10),
        'CeladonUniversity_MainHall_EventScript_UnionRoomReceptionist1': (9, 4),
        'CeladonUniversity_MainHall_EventScript_CooltrainerF2': (12, 10),
        'CeladonUniversity_MainHall_EventScript_BaldingMan3': (15, 9),
    },
    'CeladonUniversity_MainHall2F': {
        'CeladonUniversity_MainHall2F_EventScript_Scientist0': (9, 4),
        'CeladonUniversity_MainHall2F_EventScript_OldMan21': (5, 8),
        'CeladonUniversity_MainHall2F_EventScript_Lass2': ((14, 9), 'MOVEMENT_TYPE_FACE_UP'),
        'CeladonUniversity_MainHall2F_EventScript_Man3': (10, 11),
    },
    'CeladonUniversity_LectureHallA': {
        'Act2_EventScript_ProfAshford': (8, 2), 'CeladonUniversity_LectureHallA_EventScript_Lass1': (5, 4),
        'LOCALID_LECTURE_A_KAI': (2, 6), 'LOCALID_LECTURE_A_MINA': (3, 6), 'LOCALID_LECTURE_A_ROWAN': (11, 6),
    },
    'CeladonUniversity_LectureHallB': {
        'Act2_EventScript_ProfMarlowe': (8, 2),
        'LOCALID_LECTURE_B_KAI': (10, 6), 'LOCALID_LECTURE_B_MINA': (11, 6), 'LOCALID_LECTURE_B_ROWAN': (12, 6),
    },
    'CeladonUniversity_FacultyOffice': {
        'CeladonUniversity_FacultyOffice_EventScript_BaldingMan0': ((6, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
        'CeladonUniversity_FacultyOffice_EventScript_Slowpoke1': (10, 5),
    },
    'CeladonUniversity_Cafeteria': {
        'CeladonUniversity_Cafeteria_EventScript_Chef0': (3, 2),
        'CeladonUniversity_Cafeteria_EventScript_Woman21': (9, 5),
        'CeladonUniversity_Cafeteria_EventScript_FatMan2': (6, 4),
        'CeladonUniversity_Cafeteria_EventScript_BaldingMan3': (1, 4),
        'CeladonUniversity_Cafeteria_EventScript_Youngster4': (5, 6),
    },
    'CeladonUniversity_Literature': {
        'CeladonUniversity_Literature_EventScript_Gentleman0': (5, 4),
        'CeladonUniversity_Literature_EventScript_Youngster1': (5, 5),
        'CeladonUniversity_Literature_EventScript_Woman32': ((9, 4), 'MOVEMENT_TYPE_FACE_LEFT'),
        'CeladonUniversity_Literature_EventScript_Beauty3': ((9, 5), 'MOVEMENT_TYPE_FACE_LEFT'),
    },
    'CeladonUniversity_Library': {
        'CeladonUniversity_Library_EventScript_Woman20': ((7, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
        'CeladonUniversity_Library_EventScript_Gentleman1': ((5, 7), 'MOVEMENT_TYPE_FACE_RIGHT'),
    },
    'CeladonUniversity_DormLobby': {
        'CeladonUniversity_DormLobby_EventScript_OldWoman0': (5, 4),
        'CeladonUniversity_DormLobby_EventScript_Meowth1': ((16, 9), 'MOVEMENT_TYPE_FACE_LEFT'),
        'CeladonUniversity_DormLobby_EventScript_Clefairy2': (8, 7),
    },
    'CeladonUniversity_Dorm2F': {
        'CeladonUniversity_Dorm2F_EventScript_CooltrainerM0': (16, 7),
        'CeladonUniversity_Dorm2F_EventScript_Lass1': (10, 3),
        'LOCALID_DORM_KAI': (3, 7), 'LOCALID_DORM_MINA': (10, 7),
    },
    'CeladonUniversity_Dorm3F': {
        'CeladonUniversity_Dorm3F_EventScript_Rocker0': (3, 7),
        'CeladonUniversity_Dorm3F_EventScript_PokeManiac1': (10, 7),
        'CeladonUniversity_Dorm3F_EventScript_FatMan2': (10, 3),
        'LOCALID_DORM_ROWAN': (16, 7),
    },
    'CeladonUniversity_DormRoom': {
        'CeladonUniversity_DormRoom_EventScript_LaprasDoll0': (9, 4),
    },
    'CeladonUniversity_HomeEconomics': {
        'Act2_EventScript_ProfFern': (10, 2),
        'CeladonUniversity_HomeEconomics_EventScript_Lass1': (3, 8),
        'CeladonUniversity_HomeEconomics_EventScript_Beauty2': (17, 8),
        'CeladonUniversity_HomeEconomics_EventScript_Picnicker3': (12, 8),
        'CeladonUniversity_HomeEconomics_EventScript_Lass4': (11, 2),
        'LOCALID_HOME_EC_KAI': (8, 6), 'LOCALID_HOME_EC_MINA': (12, 6), 'LOCALID_HOME_EC_ROWAN': (13, 6),
    },
    'CeladonUniversity_ScienceInstitute': {
        'CeladonUniversity_ScienceInstitute_EventScript_Scientist0': (3, 7),
        # The brief: the player walks up to (8,10), between KAI and MINA, facing the director
        'LOCALID_INSTITUTE_DIRECTOR': (8, 9),
        'LOCALID_INSTITUTE_KAI': (7, 10), 'LOCALID_INSTITUTE_MINA': (9, 10), 'LOCALID_INSTITUTE_ROWAN': (10, 10),
    },
    'CeladonUniversity_ScienceLounge': {
        'CeladonUniversity_ScienceLounge_EventScript_Scientist0': (8, 10),
        'CeladonUniversity_ScienceLounge_EventScript_OldMan21': (4, 10),
        'CeladonUniversity_ScienceLounge_EventScript_Woman22': (12, 9),
    },
    'CeladonUniversity_ScienceResearchRoom': {
        'CeladonUniversity_ScienceResearchRoom_EventScript_PokeManiac0': (6, 11),
        'CeladonUniversity_ScienceResearchRoom_EventScript_Scientist1': (13, 4),
        # The lab scene: the player at the center terminal (8,2), the friends behind, the professors come up the
        # stairs (2,2)
        'LOCALID_LAB_KAI': (7, 3), 'LOCALID_LAB_MINA': (8, 3), 'LOCALID_LAB_ROWAN': (9, 3),
        'LOCALID_LAB_ASHFORD': ((2, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
        'LOCALID_LAB_MARLOWE': ((2, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
        'LOCALID_LAB_FERN': ((2, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
        # The TM Database (docs/hack/tm_database.md): the researcher beside the TM MACHINE (11,1)
        'LOCALID_LAB_TM_RESEARCHER': ((12, 2), 'MOVEMENT_TYPE_FACE_DOWN'),
    },
    'CeladonUniversity_ScienceExperimentRoom': {
        'CeladonUniversity_ScienceExperimentRoom_EventScript_Boy0': (8, 8),
        'CeladonUniversity_ScienceExperimentRoom_EventScript_Scientist1': (5, 7),
    },
    'CeladonUniversity_Hospital': {
        'CeladonUniversity_Hospital_EventScript_Nurse0': (8, 3),
        'CeladonUniversity_Hospital_EventScript_Chansey1': (9, 3),
        'CeladonUniversity_Hospital_EventScript_Gentleman2': (4, 6),
        'CeladonUniversity_Hospital_EventScript_CooltrainerF3': (11, 7),
        'CeladonUniversity_Hospital_EventScript_Youngster4': (12, 6),
    },
    'CeladonUniversity_Hospital2F': {
        'CeladonUniversity_Hospital2F_EventScript_Nurse0': (8, 6),
        'CeladonUniversity_Hospital2F_EventScript_OldWoman1': ((2, 4), 'MOVEMENT_TYPE_FACE_UP'),
        'CeladonUniversity_Hospital2F_EventScript_LittleGirl2': (4, 9),
    },
}

# Signs: script -> cells (every bg event of that script is replaced)
SIGNS = {
    'CeladonUniversity': {'CeladonUniversity_EventScript_Sign%d' % i: [xy] for i, xy in enumerate(D.CAMPUS_SIGNS)},
    'CeladonUniversity_MainHall': {'CeladonUniversity_MainHall_EventScript_Sign0': [(3, 2), (15, 2)]},
    'CeladonUniversity_MainHall2F': {
        'CeladonUniversity_MainHall2F_EventScript_Sign0': [(3, 6), (4, 6)],
        'CeladonUniversity_MainHall2F_EventScript_Sign1': [(13, 6), (14, 6), (15, 6), (13, 7), (14, 7), (15, 7)]},
    'CeladonUniversity_LectureHallA': {
        'CeladonUniversity_LectureHallA_EventScript_Sign0': [(5, 1), (6, 1), (8, 1), (9, 1)],
        'CeladonUniversity_LectureHallA_EventScript_Sign1': [(4, 3)],
        'CeladonUniversity_LectureHallA_EventScript_Sign2': [(1, 1), (13, 1)]},
    'CeladonUniversity_LectureHallB': {
        'CeladonUniversity_LectureHallB_EventScript_Sign0': [(1, 1), (2, 1), (12, 1), (13, 1)]},
    'CeladonUniversity_Library': {'CeladonUniversity_Library_EventScript_Sign0': [(2, 1), (3, 1), (11, 1), (12, 1)]},
    'CeladonUniversity_DormLobby': {'CeladonUniversity_DormLobby_EventScript_Sign0': [(11, 1), (12, 1)]},
    'CeladonUniversity_Dorm2F': {
        'CeladonUniversity_Dorm2F_EventScript_Bed': [(1, 6), (8, 6), (14, 6)],
        'CeladonUniversity_Dorm2F_EventScript_Sign0': [(9, 1), (10, 1)],
        'Act2_EventScript_RoomPlateKai': [(4, 4)], 'Act2_EventScript_RoomPlateMina': [(11, 4)]},
    'CeladonUniversity_Dorm3F': {
        'CeladonUniversity_Dorm3F_EventScript_Bed': [(1, 6), (8, 6), (14, 6)],
        'Act2_EventScript_RoomPlateRowan': [(17, 4)]},
    'CeladonUniversity_DormRoom': {
        'CeladonUniversity_DormRoom_EventScript_Sign0': [(1, 2), (1, 3)],
        'CeladonUniversity_DormRoom_EventScript_Sign1': [(3, 1)],
        'CeladonUniversity_DormRoom_EventScript_Sign2': [(8, 1), (9, 1)],
        'CeladonUniversity_DormRoom_EventScript_Sign3': [(6, 2)],
        'CeladonUniversity_DormRoom_EventScript_Sign4': [(5, 1)]},
    'CeladonUniversity_HomeEconomics': {'CeladonUniversity_HomeEconomics_EventScript_Sign0': [(4, 1), (16, 1)]},
    'CeladonUniversity_ScienceInstitute': {
        'CeladonUniversity_ScienceInstitute_EventScript_Sign0': [(4, 1)],
        'CeladonUniversity_ScienceInstitute_EventScript_Sign1': [(12, 1)],
        # The rooms' door plaques: the rooms are floors now (Sign1 is the floor directory)
        'CeladonUniversity_ScienceInstitute_EventScript_Sign2': [],
        'CeladonUniversity_ScienceInstitute_EventScript_Sign3': []},
    'CeladonUniversity_ScienceResearchRoom': {
        'CeladonUniversity_ScienceResearchRoom_EventScript_Sign0': [(4, 1), (8, 1)],
        'CeladonUniversity_ScienceResearchRoom_EventScript_TmMachine': [(11, 1)]},
}

# Triggers: map -> list of (cells, var, value, script)
STADIUM_GATE = [(x, 15) for x in range(5, 25)]
TRIGGERS = {
    # Walking into the stadium on a club night (Act2_EventScript_ClubGate picks the night)
    'CeladonUniversity': [(STADIUM_GATE, 'VAR_TEMP_1', '0', 'Act2_EventScript_ClubGate')],
    # Just inside the institute's door: the director's brief
    'CeladonUniversity_ScienceInstitute': [([(x, 12) for x in range(1, 16)], 'VAR_ACT2_PROGRESS', '13',
                                            'Act2_EventScript_FinalBrief')],
}
# Campus cells forced solid: the stadium's sides (the way in is from the path below, over the triggers)
CAMPUS_SOLID = [(x, y) for x in (4, 25) for y in range(7, 15)]


def map_folder(room):
    return 'CeladonUniversity_' + ''.join(p.capitalize() for p in room.split('_')).replace('2f', '2F').replace(
        '3f', '3F')


def write_bin(path, values):
    with open(path, 'wb') as f:
        f.write(struct.pack('<%dH' % len(values), *values))


def cell(mid, solid):
    return mid | (SOLID if solid else PASSABLE)


def main():
    st = json.load(open(D.STAMPS))
    ext, rooms = st['exterior'], st['interior']
    layouts_path = os.path.join(ROOT, 'data/layouts/layouts.json')
    layouts_doc = json.load(open(layouts_path))
    layouts = {l['id']: l for l in layouts_doc['layouts'] if 'id' in l}

    # The campus
    reserve = [xy for xy in OBJECTS['CeladonUniversity'].values() if isinstance(xy[0], int)]
    reserve += [(x, y) for x in range(52, 61) for y in range(58, 60)]
    g, solid = D.campus_ground(ext, reserve)
    for xy in CAMPUS_SOLID:
        solid[xy] = 1
    blocks = [cell(g.g[y][x], solid[(x, y)]) for y in range(D.CAMPUS_H) for x in range(D.CAMPUS_W)]
    lay = layouts['LAYOUT_CELADON_UNIVERSITY']
    write_bin(os.path.join(ROOT, lay['blockdata_filepath']), blocks)
    write_bin(os.path.join(ROOT, lay['border_filepath']), [0x01C | SOLID, 0x01D | SOLID, 0x014 | SOLID, 0x015 | SOLID])
    lay.update(width=D.CAMPUS_W, height=D.CAMPUS_H, secondary_tileset='gTileset_CeladonUniversity')

    # The campus doors
    doors = {}
    for room, (i, k) in DOOR_OF.items():
        name, x, y = D.CAMPUS[i]
        dx, dy = ext[name]['doors'][k]
        doors[room] = (x + dx, y + dy)
    gate_name, gx, gy = next(c for c in D.CAMPUS if c[0] == 'GATE')
    campus_warps = [warp(gx + c, gy + r, 'MAP_CELADON_CITY_UNIVERSITY_GATE', 0) for (c, r) in ext['GATE']['warps']]
    for room in CAMPUS_DOORS:
        campus_warps.append(warp(*doors[room], U + room, 0))

    # The rooms
    room_warps = {room: [] for room in rooms}
    for room in rooms:
        mat = [tuple(map(int, k.split(','))) for k, v in rooms[room]['marks'].items() if v == 'mat']
        if mat:
            room_warps[room].append(warp(*mat[0], CAMPUS_MAP, 2 + CAMPUS_DOORS.index(room)))
    for (a, a_cells, b, b_cells) in STAIRS:
        base_a, base_b = len(room_warps[a]), len(room_warps[b])
        for k, (ca, cb) in enumerate(zip(a_cells, b_cells)):
            assert rooms[a]['marks']['%d,%d' % ca] == 'up' and rooms[b]['marks']['%d,%d' % cb] == 'down', (a, b)
            room_warps[a].append(warp(*ca, U + b, base_b + k))
            room_warps[b].append(warp(*cb, U + a, base_a + k))
    for room, r in rooms.items():
        lay = layouts['LAYOUT_CELADON_UNIVERSITY_' + room]
        blocks = [cell(r['rows'][y][x], r['solid'][y][x]) for y in range(r['h']) for x in range(r['w'])]
        write_bin(os.path.join(ROOT, lay['blockdata_filepath']), blocks)
        lay.update(width=r['w'], height=r['h'], primary_tileset='gTileset_BuildingFrlg', secondary_tileset=r['tileset'])
    with open(layouts_path, 'w') as f:
        json.dump(layouts_doc, f, indent=2, ensure_ascii=False)
        f.write('\n')

    # Events
    update_events('CeladonUniversity', campus_warps, CAMPUS_SOLID)
    for room in rooms:
        update_events(map_folder(room), room_warps[room])


def warp(x, y, dest, dest_id):
    return {'x': x, 'y': y, 'elevation': 0, 'dest_map': dest, 'dest_warp_id': str(dest_id)}


def update_events(folder, warps, _=None):
    path = os.path.join(ROOT, 'data/maps', folder, 'map.json')
    m = json.load(open(path))
    m['warp_events'] = warps
    table = OBJECTS.get(folder, {})
    for o in m.get('object_events') or []:
        key = o.get('local_id') if o.get('local_id') in table else o.get('script')
        assert key in table, '%s: no new place for %s' % (folder, key)
        where = table[key]
        if isinstance(where[0], tuple):
            where, o['movement_type'] = where
        o['x'], o['y'] = where
    signs = SIGNS.get(folder, {})
    bg = []
    for script, cells in signs.items():
        if not cells:
            continue
        old = next((b for b in m.get('bg_events') or [] if b.get('script') == script), None)
        assert old is not None, '%s: no sign %s' % (folder, script)
        for (x, y) in cells:
            b = dict(old)
            b.update(x=x, y=y)
            bg.append(b)
    for b in m.get('bg_events') or []:
        assert b.get('script') in signs, '%s: sign %s not placed' % (folder, b.get('script'))
    m['bg_events'] = bg
    coords = []
    for (cells, var, value, script) in TRIGGERS.get(folder, []):
        for (x, y) in cells:
            coords.append({'type': 'trigger', 'x': x, 'y': y, 'elevation': 3, 'var': var, 'var_value': value,
                           'script': script})
    m['coord_events'] = coords
    with open(path, 'w') as f:
        json.dump(m, f, indent=2, ensure_ascii=False)
        f.write('\n')


if __name__ == '__main__':
    main()
