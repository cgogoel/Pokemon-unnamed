#!/usr/bin/env python3
"""Generate src/data/tm_research_species.h: every Pokémon of each type that can be found in this game.

The TM Database (docs/hack/tm_database.md) unlocks every TM of a type once the player has caught (Pokédex caught
flag) every one of these species of that type. "Can be found in this game" is:

- Kanto's wild encounter tables in FireRed (src/data/wild_encounters.json, labels with FireRed, not the Sevii
  Islands' maps), the hack's Act 2 tables (sAct2_*_FireRed) included.
- The Pokémon from other regions that Act 2 mixes in: the courses' (sOtherRegionFire/Water/Grass in
  src/act2_story.c) and the road home's (src/data/act2_other_region_pools.h, by gen_other_region_pools.py).
- Gifts and static encounters: givemon/giveegg/setwildbattle/befriendbattle in Kanto maps' scripts (the maps with
  "region": "REGION_KANTO", not the Sevii Islands), the hack's own shared scripts (data/scripts/act*.inc), and the
  hack's gifts made in C (the glade's MORELULL, the ranger station's dead-end Pokémon).

Left out: legendaries and mythicals (one-off encounters; set INCLUDE_LEGENDARIES to count them), and species with
no National Dex number. A species counts for both of its types. Forms that share a National Dex number count once
(the caught flag is per number).

Usage: python3 tools/hack/gen_tm_research_species.py
"""
import glob
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_other_region_pools as pools  # noqa: E402

ROOT = pools.ROOT
OUT = os.path.join(ROOT, 'src/data/tm_research_species.h')
INCLUDE_LEGENDARIES = False
SEVII = re.compile(r'^MAP_(ONE|TWO|THREE|FOUR|FIVE|SIX|SEVEN)_ISLAND|^MAP_(NAVEL_ROCK|BIRTH_ISLAND)')
SEVII_DIR = re.compile(r'^(One|Two|Three|Four|Five|Six|Seven)Island|^(NavelRock|BirthIsland)')
LEGENDARY_FLAGS = ['isRestrictedLegendary', 'isSubLegendary', 'isMythical']
C_GIFTS = [('src/act2_night_bloom.c', r'CreateRandomMon\(&mon, (SPECIES_\w+)'),
           ('src/act2_station.c', r'sDeadEndSpecies\[\] = \{([^}]*)\}')]


def species_info():
    dex = pools.national_dex_numbers()
    raw, defines = pools.parse_species()
    info = {}
    for name, body in raw.items():
        types = re.findall(r'TYPE_(\w+)', pools.resolve(pools.field(body, 'types') or '', defines))
        if not types:
            continue
        nat = (pools.field(body, 'natDexNum') or '').strip()
        info[name] = {
            'types': {types[0], types[1] if len(types) > 1 else types[0]},
            'dex': dex.get(nat, 0),
            'legendary': any(re.search(r'\.' + f + r'\s*=\s*TRUE', body) for f in LEGENDARY_FLAGS),
        }
    return info


def wild_species():
    found = {}
    data = json.load(open(os.path.join(ROOT, 'src/data/wild_encounters.json'), encoding='utf-8'))
    for group in data['wild_encounter_groups']:
        if group.get('label') != 'gWildMonHeaders':
            continue
        for enc in group['encounters']:
            if 'FireRed' not in enc['base_label'] or SEVII.match(enc.get('map', '')):
                continue
            for key, value in enc.items():
                if isinstance(value, dict) and 'mons' in value:
                    for mon in value['mons']:
                        found.setdefault(mon['species'], 'wild')
    return found


def pool_species():
    found = {}
    text = pools.read(os.path.join(ROOT, 'src/data/act2_other_region_pools.h'))
    for body in re.findall(r'static const u16 sAct2RoadPool_\w+\[\] =\s*\{(.*?)\};', text, re.S):
        for s in re.findall(r'SPECIES_\w+', body):
            found.setdefault(s, 'road home pool')
    text = pools.read(os.path.join(ROOT, 'src/act2_story.c'))
    for body in re.findall(r'static const u16 sOtherRegion\w+\[\] =\s*\{(.*?)\};', text, re.S):
        for s in re.findall(r'SPECIES_\w+', body):
            found.setdefault(s, 'course pool')
    return found


def gift_species():
    found = {}
    pattern = re.compile(r'\b(?:givemon|giveegg|setwildbattle|befriendbattle)\s+(SPECIES_\w+)')
    files = []
    for path in glob.glob(os.path.join(ROOT, 'data/maps/*/map.json')):
        folder = os.path.dirname(path)
        if SEVII_DIR.match(os.path.basename(folder)):
            continue
        if '"region": "REGION_KANTO"' in pools.read(path):
            files.append(os.path.join(folder, 'scripts.inc'))
    files += glob.glob(os.path.join(ROOT, 'data/scripts/act*.inc'))
    for path in files:
        if os.path.exists(path):
            for s in pattern.findall(pools.read(path)):
                found.setdefault(s, 'gift/static')
    for path, regex in C_GIFTS:
        for m in re.findall(regex, pools.read(os.path.join(ROOT, path))):
            for s in re.findall(r'SPECIES_\w+', m):
                found.setdefault(s, 'gift/static')
    return found


def main():
    info = species_info()
    sources = {}
    for found in (wild_species(), pool_species(), gift_species()):
        for s, src in found.items():
            sources.setdefault(s, src)

    by_type = {t: {} for t in pools.TYPES}
    for s in sources:
        data = info.get(s)
        if data is None or data['dex'] == 0 or (data['legendary'] and not INCLUDE_LEGENDARIES):
            continue
        for t in data['types']:
            if t in by_type:
                by_type[t].setdefault(data['dex'], s)  # One per National Dex number

    out = ['// Generated by tools/hack/gen_tm_research_species.py: don\'t edit.',
           '// Every Pokémon of each type that can be found in this game (the TM Database\'s "catch them all" rule,',
           '// docs/hack/tm_database.md), one per National Dex number, in National Dex order.', '']
    for t in pools.TYPES:
        members = [s for _, s in sorted(by_type[t].items())]
        if not members:
            continue
        out.append('static const u16 sTmResearchSpecies_%s[] =' % t.capitalize())
        out.append('{')
        for i in range(0, len(members), 4):
            out.append('    ' + ' '.join(m + ',' for m in members[i:i + 4]))
        out.append('};')
        out.append('')
    out.append('static const struct TmResearchSpeciesList sTmResearchSpecies[NUMBER_OF_MON_TYPES] =')
    out.append('{')
    for t in pools.TYPES:
        if by_type[t]:
            arr = 'sTmResearchSpecies_%s' % t.capitalize()
            out.append('    [TYPE_%s] = { %s, ARRAY_COUNT(%s) },' % (t, arr, arr))
    out.append('};')
    with open(OUT, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out) + '\n')

    for t in pools.TYPES:
        print('%-9s %4d' % (t.capitalize(), len(by_type[t])))
    print('->', os.path.relpath(OUT, ROOT))


if __name__ == '__main__':
    main()
