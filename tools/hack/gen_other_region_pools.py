#!/usr/bin/env python3
"""Generate src/data/act2_other_region_pools.h: the final project's Pokémon from beyond Kanto.

Act 2's final project (docs/hack/act2_finale.md, "The road home") sometimes swaps a wild or trainer's Pokémon for
one from another region with the same primary type and evolution stage. This script builds those pools from
src/data/pokemon/species_info/*.h:

- National Dex number above 151 (not Kanto's).
- Not legendary, mythical, an Ultra Beast or a paradox Pokémon; not a Mega, Primal, Gigantamax, Ultra Burst,
  Tera or totem form; not a regional form; only a species' base form (the first in its form table), so nothing
  that needs a form change to exist.
- Not a baby (an undiscovered-egg-group Pokémon that starts its line and evolves into one that can breed).
- Stage: 0 basic, 1 stage 1, 2 stage 2, counting pre-evolutions but not babies (so Pikachu is basic).

It also writes every species' stage (sAct2RoadEvoStage), which the game uses for the Pokémon being replaced.
Species defined through a macro (Unown, Vivillon's line, Furfrou, Minior, Alcremie...) are skipped.

Usage: python3 tools/hack/gen_other_region_pools.py
"""
import glob
import os
import re

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
SPECIES_DIR = os.path.join(ROOT, 'src/data/pokemon/species_info')
FORM_TABLES = os.path.join(ROOT, 'src/data/pokemon/form_species_tables.h')
POKEDEX_H = os.path.join(ROOT, 'include/constants/pokedex.h')
OUT = os.path.join(ROOT, 'src/data/act2_other_region_pools.h')

KANTO_DEX_COUNT = 151
TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'FIRE', 'WATER',
         'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']
EXCLUDE_FLAGS = ['isRestrictedLegendary', 'isSubLegendary', 'isMythical', 'isUltraBeast', 'isParadox', 'isTotem',
                 'isMegaEvolution', 'isPrimalReversion', 'isUltraBurst', 'isGigantamax', 'isTeraForm',
                 'isAlolanForm', 'isGalarianForm', 'isHisuianForm', 'isPaldeanForm']


def read(path):
    with open(path, encoding='utf-8') as f:
        return f.read()


def match_close(text, start, open_ch, close_ch):
    """Index just past the bracket matching text[start] (an opening bracket)."""
    depth = 0
    for i in range(start, len(text)):
        if text[i] == open_ch:
            depth += 1
        elif text[i] == close_ch:
            depth -= 1
            if depth == 0:
                return i + 1
    raise ValueError('unbalanced')


def national_dex_numbers():
    text = read(POKEDEX_H)
    body = text[text.index('enum NationalDexOrder'):]
    body = body[body.index('{') + 1:body.index('}')]
    names = re.findall(r'^\s*(NATIONAL_DEX_\w+)', body, re.M)
    return {name: i for i, name in enumerate(names)}


def form_tables():
    tables = {}
    for name, body in re.findall(r'static const u16 (\w+)\[\] = \{(.*?)\};', read(FORM_TABLES), re.S):
        tables[name] = re.findall(r'SPECIES_\w+', body)
    return tables


def parse_species():
    defines = {}
    species = {}
    for path in sorted(glob.glob(os.path.join(SPECIES_DIR, 'gen_*_families.h'))):
        text = read(path)
        # Type macros (CLEFAIRY_FAMILY_TYPES...): the first definition is the up-to-date one (P_UPDATED_TYPES)
        for name, value in re.findall(r'#\s*define (\w+) (.+)', text):
            defines.setdefault(name, value.strip())
        for m in re.finditer(r'\[(SPECIES_\w+)\]\s*=\s*\{', text):
            end = match_close(text, m.end() - 1, '{', '}')
            species[m.group(1)] = text[m.end():end - 1]
    return species, defines


def resolve(value, defines):
    for _ in range(4):
        value = re.sub(r'\b[A-Z][A-Z0-9_]*_TYPE[S12]?\b', lambda m: defines.get(m.group(0), m.group(0)), value)
    # (P_UPDATED_TYPES >= GEN_X ? NEW : OLD): the up-to-date one
    return re.sub(r'\(\s*P_UPDATED_TYPES\s*>=\s*GEN_\d+\s*\?\s*(TYPE_\w+)\s*:\s*TYPE_\w+\s*\)', r'\1', value)


def field(body, name):
    m = re.search(r'\.' + name + r'\s*=\s*', body)
    if not m:
        return None
    i = m.end()
    if body[i:].startswith('EVOLUTION(') or body[i:].startswith('MON_TYPES(') or body[i:].startswith('MON_EGG_GROUPS('):
        j = body.index('(', i)
        return body[i:match_close(body, j, '(', ')')]
    return body[i:body.index(',\n', i)] if ',\n' in body[i:] else body[i:]


def main():
    dex = national_dex_numbers()
    tables = form_tables()
    raw, defines = parse_species()

    info = {}
    for name, body in raw.items():
        types_text = resolve(field(body, 'types') or '', defines)
        types = re.findall(r'TYPE_(\w+)', types_text)
        if not types:
            continue
        evo_text = field(body, 'evolutions') or ''
        targets = []
        # {method, param, target, ...}
        for method, target in re.findall(r'\{\s*(EVO_\w+)\s*,[^,{}]*,\s*(SPECIES_\w+)', evo_text):
            if method != 'EVO_NONE':
                targets.append(target)
        nat = (field(body, 'natDexNum') or '').strip()
        table = (field(body, 'formSpeciesIdTable') or '').strip()
        info[name] = {
            'types': [types[0], types[1] if len(types) > 1 else types[0]],
            'dex': dex.get(nat, 0),
            'flags': {f for f in EXCLUDE_FLAGS if re.search(r'\.' + f + r'\s*=\s*TRUE', body)},
            'undiscovered': 'EGG_GROUP_NO_EGGS_DISCOVERED' in (field(body, 'eggGroups') or ''),
            'evolutions': targets,
            'base_form': not table or tables.get(table, [name])[0] == name,
        }

    pre = {}
    for name, data in info.items():
        for target in data['evolutions']:
            pre.setdefault(target, name)

    def is_baby(name):
        data = info[name]
        if not data['undiscovered'] or not data['evolutions'] or name in pre:
            return False
        return any(t in info and not info[t]['undiscovered'] for t in data['evolutions'])

    def stage(name):
        count, seen = 0, set()
        while name in pre and name not in seen:
            seen.add(name)
            name = pre[name]
            if name in info and not is_baby(name):
                count += 1
        return min(count, 2)

    pools = {(t, s): [] for t in TYPES for s in range(3)}
    for name, data in info.items():
        if data['dex'] <= KANTO_DEX_COUNT or data['flags'] or not data['base_form'] or is_baby(name):
            continue
        if data['types'][0] not in TYPES:
            continue
        pools[(data['types'][0], stage(name))].append((data['dex'], name))

    out = ['// Generated by tools/hack/gen_other_region_pools.py from src/data/pokemon/species_info/*.h: don\'t edit.',
           '// The final project\'s Pokémon from beyond Kanto, by primary type and evolution stage (src/act2_road.c).', '']
    for (t, s), members in pools.items():
        if members:
            out.append('static const u16 sAct2RoadPool_%s_%d[] =' % (t.capitalize(), s))
            out.append('{')
            names = [n for _, n in sorted(members)]
            for i in range(0, len(names), 4):
                out.append('    ' + ' '.join(n + ',' for n in names[i:i + 4]))
            out.append('};')
            out.append('')
    out.append('static const struct Act2RoadPool sAct2RoadPools[NUMBER_OF_MON_TYPES][ACT2_ROAD_STAGES] =')
    out.append('{')
    for t in TYPES:
        entries = []
        for s in range(3):
            if pools[(t, s)]:
                arr = 'sAct2RoadPool_%s_%d' % (t.capitalize(), s)
                entries.append('{ %s, ARRAY_COUNT(%s) }' % (arr, arr))
            else:
                entries.append('{ NULL, 0 }')
        out.append('    [TYPE_%s] = { %s },' % (t, ', '.join(entries)))
    out.append('};')
    out.append('')
    out.append('// Every species past the basic stage (babies don\'t count): 1 for stage 1, 2 for stage 2')
    out.append('static const u8 sAct2RoadEvoStage[NUM_SPECIES] =')
    out.append('{')
    for name in sorted(info):
        s = stage(name)
        if s:
            out.append('    [%s] = %d,' % (name, s))
    out.append('};')
    with open(OUT, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out) + '\n')

    print('%-9s %5s %5s %5s' % ('', 'basic', 'st.1', 'st.2'))
    for t in TYPES:
        print('%-9s %5d %5d %5d' % (t.capitalize(), *(len(pools[(t, s)]) for s in range(3))))
    print('total', sum(len(v) for v in pools.values()), '->', os.path.relpath(OUT, ROOT))


if __name__ == '__main__':
    main()
