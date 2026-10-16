#!/usr/bin/env python3
"""Ranks the species in src/data/pokemon/species_info for Act 1 befriending (docs/hack/befriending.md).

Writes one CSV with three separate rankings:
  three_stage_line  3-stage evolution lines whose base Pokémon fits the befriending lore (pick at least 3)
  boss              intimidating Pokémon that don't fit the lore, for the final one to befriend
  tutorial          the easiest Pokémon to befriend, for the tutorial encounter

Scores are heuristics from each species' data: Pokédex entry and category keywords, base friendship, catch rate,
size, base stats, types, egg groups and abilities. Legendaries, mythicals, Ultra Beasts, Paradox Pokémon and
battle/cosmetic forms are left out; regional forms are kept.

Usage: python3 tools/hack/rank_befriend_candidates.py [out.csv]
"""
import csv
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SPECIES_DIR = os.path.join(ROOT, "src/data/pokemon/species_info")
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "docs/hack/act1_befriend_rankings.csv")

# Lore keywords in the Pokédex entry or category. Gentle, people-friendly, wary-but-winnable Pokémon score up;
# dangerous or hostile ones score down (and up for the boss ranking).
POSITIVE = {
    "friendly": 3, "gentle": 3, "docile": 3, "affection": 3, "affectionate": 3, "loves": 2, "playful": 2,
    "people": 2, "humans": 2, "children": 2, "kind": 2, "calm": 2, "peaceful": 2, "cute": 2, "companion": 3,
    "pet": 3, "nuzzle": 2, "cuddl": 2, "follows": 2, "loyal": 3, "trust": 3, "curious": 1, "timid": 1, "shy": 1,
    "cheerful": 2, "happy": 1, "sleep": 1, "nap": 1, "garden": 1, "farm": 2, "town": 1, "fields": 1,
    "meadow": 1, "berries": 1, "berry": 1, "fruit": 1, "flower": 1, "sing": 1, "song": 1, "warm": 1, "soft": 1,
    "fluffy": 2, "fur": 1, "gift": 1, "help": 1, "heal": 1, "comfort": 2, "smile": 2, "lonely": 1, "mischiev": 1,
}
NEGATIVE = {
    "aggressive": 3, "vicious": 3, "ferocious": 3, "fierce": 2, "violent": 3, "savage": 3, "rage": 2, "angry": 2,
    "anger": 2, "dangerous": 3, "danger": 2, "attacks": 2, "attack": 1, "kills": 3, "kill": 3, "deadly": 3,
    "prey": 2, "devour": 3, "destroy": 2, "terrif": 2, "fear": 1, "feared": 2, "curse": 2, "haunt": 1,
    "toxic": 2, "venom": 2, "poison": 1, "brutal": 3, "ruthless": 3, "temper": 2, "territorial": 2, "bite": 1,
    "fangs": 1, "claws": 1, "rampage": 3, "crush": 1, "tear": 1, "shred": 2, "intimidat": 3, "menac": 2,
    "glare": 1, "hostile": 3, "wild": 1, "battle": 1, "fight": 1, "strong": 1, "power": 1,
}
FRIENDLY_TYPES = {"NORMAL": 4, "GRASS": 3, "FLYING": 2, "BUG": 2, "FAIRY": 4, "WATER": 2, "ELECTRIC": 1}
UNFRIENDLY_TYPES = {"GHOST": 4, "DARK": 5, "DRAGON": 5, "POISON": 3, "STEEL": 2, "FIGHTING": 3, "ROCK": 1}
FRIENDLY_EGG = {"FIELD": 3, "FLYING": 2, "FAIRY": 3, "GRASS": 2, "BUG": 1, "WATER_1": 1}
UNFRIENDLY_EGG = {"MONSTER": 3, "DRAGON": 4, "AMORPHOUS": 2, "MINERAL": 2}
FRIENDLY_ABILITIES = {
    "FRIEND_GUARD": 4, "CUTE_CHARM": 4, "PICKUP": 2, "RUN_AWAY": 2, "HONEY_GATHER": 3, "HEALER": 3,
    "SWEET_VEIL": 3, "FLUFFY": 2, "BALL_FETCH": 3, "HOSPITALITY": 4, "SERENE_GRACE": 2, "NATURAL_CURE": 2,
    "OWN_TEMPO": 1, "OBLIVIOUS": 1, "SYMBIOSIS": 3, "CHEEK_POUCH": 2, "PASTEL_VEIL": 2, "KLUTZ": 1,
}
INTIMIDATING_ABILITIES = {
    "INTIMIDATE": 6, "MOXIE": 4, "PRESSURE": 3, "STENCH": 2, "POISON_POINT": 2, "ROUGH_SKIN": 3, "IRON_BARBS": 2,
    "ANGER_POINT": 4, "GUTS": 2, "STRONG_JAW": 3, "TOUGH_CLAWS": 3, "SHEER_FORCE": 2, "UNNERVE": 4,
    "AFTERMATH": 2, "CURSED_BODY": 3, "BERSERK": 4, "DEFIANT": 2, "SCRAPPY": 1, "RECKLESS": 2, "HUSTLE": 1,
    "FRISK": 1, "MOLD_BREAKER": 2, "SHADOW_TAG": 3, "SUPERSWEET_SYRUP": -2,
}

EXCLUDE_FLAGS = ["isRestrictedLegendary", "isSubLegendary", "isMythical", "isUltraBeast", "isParadox", "isTotem",
                 "isMegaEvolution", "isPrimalReversion", "isUltraBurst", "isGigantamax", "isTeraForm"]
REGIONAL_FLAGS = ["isAlolanForm", "isGalarianForm", "isHisuianForm", "isPaldeanForm"]


def read_sources():
    defines, dex_texts, text = {}, {}, ""
    macros = {}
    for path in sorted(glob.glob(os.path.join(SPECIES_DIR, "*.h"))):
        src = open(path, encoding="utf-8").read()
        text += src + "\n"
        for name, value in re.findall(r"^#define\s+(\w+)\s+(.+?)\s*$", src, re.M):
            defines[name] = value
        # Multi-line function-like macros (species shared between forms, e.g. FLABEBE_MISC_INFO)
        for name, body in re.findall(r"^#define\s+(\w+)\([^)]*\)\s*\\\n((?:.*\\\n)*.*)$", src, re.M):
            macros[name] = body.replace("\\\n", "\n")
        for name, body in re.findall(r"const u8 (g\w+PokedexText)\[\] = _\((.*?)\);", src, re.S):
            dex_texts[name] = " ".join(re.findall(r'"((?:[^"\\]|\\.)*)"', body))
    defines["__macros__"] = macros
    return text, defines, dex_texts


def resolve(value, defines, depth=0):
    value = value.strip().rstrip(",")
    if depth < 5 and value in defines:
        return resolve(defines[value], defines, depth + 1)
    m = re.search(r"\?\s*([^:]+?)\s*:", value)  # (P_UPDATED_X >= GEN_n) ? new : old -> new
    if m:
        return resolve(m.group(1), defines, depth + 1)
    return value


def to_int(value, defines, default=0):
    value = resolve(value, defines)
    if value == "STANDARD_FRIENDSHIP":
        return 50
    m = re.match(r"-?\d+", value)
    return int(m.group()) if m else default


def field(block, name):
    m = re.search(r"\." + name + r"\s*=\s*(.+?),\s*\n", block)
    return m.group(1) if m else None


def parse_species(text, defines, dex_texts):
    species = {}
    macros = defines.get("__macros__", {})
    starts = [(m.start(), m.group(1)) for m in re.finditer(r"^\s*\[(SPECIES_\w+)\]\s*=", text, re.M)]
    for i, (start, const) in enumerate(starts):
        block = text[start:starts[i + 1][0] if i + 1 < len(starts) else len(text)]
        # Fields that come from a shared macro go after the species' own, so its own win
        expanded, pending = set(), re.findall(r"\b([A-Z0-9_]+)\(", block)
        while pending:  # Macros can use other macros (FLOETTE_NORMAL_INFO -> FLOETTE_MISC_INFO)
            name = pending.pop()
            if name in macros and name not in expanded:
                expanded.add(name)
                block += "\n" + macros[name] + ",\n"
                pending += re.findall(r"\b([A-Z0-9_]+)\(", macros[name])
        name = re.search(r'\.speciesName\s*=\s*_\("([^"]*)"\)', block)
        if not name:
            continue
        flags = {f: bool(re.search(r"\." + f + r"\s*=\s*TRUE", block)) for f in EXCLUDE_FLAGS + REGIONAL_FLAGS}
        types = re.search(r"\.types\s*=\s*MON_TYPES\(([^)]*)\)", block)
        types = [t.strip().replace("TYPE_", "") for t in types.group(1).split(",")] if types else []
        types = [resolve(t, defines).replace("TYPE_", "") for t in types]
        eggs = re.search(r"\.eggGroups\s*=\s*MON_EGG_GROUPS\(([^)]*)\)", block)
        eggs = [e.strip().replace("EGG_GROUP_", "") for e in eggs.group(1).split(",")] if eggs else []
        abilities = field(block, "abilities") or ""
        abilities = resolve(abilities, defines)
        abilities = [a.replace("ABILITY_", "") for a in re.findall(r"ABILITY_\w+", abilities) if a != "ABILITY_NONE"]
        desc = re.search(r"\.description\s*=\s*COMPOUND_STRING\((.*?)\)\s*,", block, re.S)
        if desc:
            dex = " ".join(re.findall(r'"((?:[^"\\]|\\.)*)"', desc.group(1)))
        else:
            sym = field(block, "description")
            dex = dex_texts.get(sym.strip(), "") if sym else ""
        dex = dex.replace("\\n", " ").replace("  ", " ").strip()
        category = re.search(r'\.categoryName\s*=\s*_\("([^"]*)"\)', block)
        stats = [to_int(field(block, s) or "0", defines, 50)
                 for s in ("baseHP", "baseAttack", "baseDefense", "baseSpeed", "baseSpAttack", "baseSpDefense")]
        evo = re.search(r"\.evolutions\s*=\s*EVOLUTION\((.*?)\)\s*,?\s*(?:\n\s*(?:\.|#|\}|//)|$)", block, re.S)
        evolutions = sorted(set(re.findall(r"SPECIES_\w+", evo.group(1)))) if evo else []
        species[const] = {
            "const": const, "name": name.group(1), "types": types, "eggs": eggs, "abilities": abilities,
            "dex": dex, "category": category.group(1) if category else "",
            "catchRate": to_int(field(block, "catchRate") or "0", defines),
            "friendship": to_int(field(block, "friendship") or "50", defines, 50),
            "height": to_int(field(block, "height") or "0", defines), "weight": to_int(field(block, "weight") or "0", defines),
            "stats": stats, "bst": sum(stats), "evolutions": evolutions, "flags": flags,
            "natDex": resolve(field(block, "natDexNum") or "", defines),
        }
    return species


def is_candidate(s, species):
    if any(s["flags"][f] for f in EXCLUDE_FLAGS):
        return False
    regional = any(s["flags"][f] for f in REGIONAL_FLAGS)
    # A species whose constant extends another species' constant is a form of it (keep regional ones)
    parts = s["const"].split("_")
    for n in range(len(parts) - 1, 1, -1):
        if "_".join(parts[:n]) in species:
            return regional
    return True


STEMS = {"affection", "cuddl", "mischiev", "terrif", "intimidat", "menac", "fluff", "venom", "toxic", "aggress"}


def keyword_hits(text, table):
    # Whole words ("pet" isn't "petals", "fur" isn't "furious"), except stems that take several endings
    text = text.lower()
    hits = {k: w for k, w in table.items()
            if re.search(r"\b" + re.escape(k) + ("" if k in STEMS else r"\b"), text)}
    return sum(hits.values()), sorted(hits)


def befriend_score(s):
    lore = s["dex"] + " " + s["category"]
    pos, pos_hits = keyword_hits(lore, POSITIVE)
    neg, neg_hits = keyword_hits(lore, NEGATIVE)
    score = 40 + 4 * pos - 4 * neg
    score += (s["friendship"] - 50) / 10                 # 0 / 35 / 50 / 70 / 100 / 140
    score += s["catchRate"] / 255 * 12                    # common and easy to approach
    score += max(0, 10 - min(s["height"], 20) / 2)        # small (height in decimeters)
    score += max(-8, min(10, (500 - s["bst"]) / 30))      # not too strong
    score += sum(FRIENDLY_TYPES.get(t, 0) - UNFRIENDLY_TYPES.get(t, 0) for t in set(s["types"]))
    score += sum(FRIENDLY_EGG.get(e, 0) - UNFRIENDLY_EGG.get(e, 0) for e in set(s["eggs"]))
    score += sum(FRIENDLY_ABILITIES.get(a, 0) - INTIMIDATING_ABILITIES.get(a, 0) / 2 for a in set(s["abilities"]))
    return score, pos_hits, neg_hits


def intimidation_score(s):
    lore = s["dex"] + " " + s["category"]
    pos, pos_hits = keyword_hits(lore, POSITIVE)
    neg, neg_hits = keyword_hits(lore, NEGATIVE)
    score = 30 + 5 * neg - 3 * pos
    score += (50 - s["friendship"]) / 8
    score += (255 - s["catchRate"]) / 255 * 8             # rare and hard to approach
    score += min(s["height"], 30) / 3                     # big
    score += min(12, max(0, (s["stats"][1] - 60) / 8))    # hits hard
    score += min(8, max(0, (s["bst"] - 350) / 30))
    score += sum(UNFRIENDLY_TYPES.get(t, 0) - FRIENDLY_TYPES.get(t, 0) / 2 for t in set(s["types"]))
    score += sum(UNFRIENDLY_EGG.get(e, 0) for e in set(s["eggs"]))
    score += sum(INTIMIDATING_ABILITIES.get(a, 0) for a in set(s["abilities"]))
    return score, neg_hits, pos_hits


def main():
    text, defines, dex_texts = read_sources()
    species = parse_species(text, defines, dex_texts)
    # Evolution targets built by token pasting (SPECIES_FLOETTE_ ##FORM) point at the first matching form
    for s in species.values():
        s["evolutions"] = sorted({e if e in species else next((c for c in species if c.startswith(e)), e)
                                  for e in s["evolutions"]})
    candidates = {c: s for c, s in species.items() if is_candidate(s, species)}
    # Cosmetic variants without a base constant (Flabébé colors, Vivillon patterns): keep one per name
    seen_names = set()
    for c in list(candidates):
        s = candidates[c]
        key = c if any(s["flags"][f] for f in REGIONAL_FLAGS) else s["name"]
        if key in seen_names:
            del candidates[c]
        seen_names.add(key)
    # Evolutions into a dropped variant go to the kept one with the same name
    kept_by_name = {s["name"]: c for c, s in candidates.items() if not any(s["flags"][f] for f in REGIONAL_FLAGS)}
    for s in candidates.values():
        s["evolutions"] = sorted({e if e in candidates else kept_by_name.get(species[e]["name"], e) if e in species else e
                                  for e in s["evolutions"]})

    # Evolution families among the candidates
    pre = {}
    for c, s in candidates.items():
        for e in s["evolutions"]:
            if e in candidates:
                pre.setdefault(e, c)


    def line(c):
        out, layer, seen = [candidates[c]["name"]], [c], {c}
        while True:  # Some forms evolve back and forth, so stop at species already listed
            layer = sorted({e for x in layer for e in candidates[x]["evolutions"] if e in candidates and e not in seen})
            if not layer:
                return out, layer
            seen.update(layer)
            out.append("/".join(candidates[x]["name"] for x in layer))

    def stages(c):
        return len(line(c)[0])

    def mids_finals(c):
        mids = [e for e in candidates[c]["evolutions"] if e in candidates]
        finals = sorted({f for m in mids for f in candidates[m]["evolutions"] if f in candidates})
        return mids, finals

    rows = []

    def row(ranking, c, score, extra_line="", pos=(), neg=()):
        s = candidates[c]
        return {
            "ranking": ranking, "species": c, "name": s["name"], "line": extra_line, "score": round(score, 1),
            "types": "/".join(s["types"]), "category": s["category"], "catch_rate": s["catchRate"],
            "friendship": s["friendship"], "height_m": s["height"] / 10, "weight_kg": s["weight"] / 10,
            "bst": s["bst"], "lore_for": " ".join(pos), "lore_against": " ".join(neg),
            "abilities": " ".join(sorted(set(s["abilities"]))), "pokedex": s["dex"],
        }

    # 3-stage lines: scored mostly on the base Pokémon (the one befriended), a little on what it grows into
    lines = []
    for c in candidates:
        if c in pre or stages(c) != 3:
            continue
        mids, finals = mids_finals(c)
        base, pos, neg = befriend_score(candidates[c])
        mid = max(befriend_score(candidates[m])[0] for m in mids)
        fin = max(befriend_score(candidates[f])[0] for f in finals) if finals else mid
        names, _ = line(c)
        lines.append((0.7 * base + 0.2 * mid + 0.1 * fin, c, " > ".join(names), pos, neg))
    for rank, (score, c, names, pos, neg) in enumerate(sorted(lines, key=lambda x: -x[0]), 1):
        rows.append({"rank": rank, **row("three_stage_line", c, score, names, pos, neg)})

    # Boss: the most intimidating Pokémon (any stage)
    boss = sorted(((intimidation_score(s), c) for c, s in candidates.items()), key=lambda x: -x[0][0])
    for rank, ((score, neg, pos), c) in enumerate(boss, 1):
        names, _ = line(c) if c not in pre else ([candidates[c]["name"]], [])
        rows.append({"rank": rank, **row("boss", c, score, " > ".join(names), pos, neg)})

    # Tutorial: the easiest to befriend; extra weight on being very common, friendly and harmless
    tutorial = []
    for c, s in candidates.items():
        score, pos, neg = befriend_score(s)
        score += s["catchRate"] / 255 * 10 + (s["friendship"] - 50) / 10 - 3 * len(neg)
        tutorial.append((score, c, pos, neg))
    for rank, (score, c, pos, neg) in enumerate(sorted(tutorial, key=lambda x: -x[0]), 1):
        names, _ = line(c) if c not in pre else ([candidates[c]["name"]], [])
        rows.append({"rank": rank, **row("tutorial", c, score, " > ".join(names), pos, neg)})

    fields = ["ranking", "rank", "species", "name", "line", "score", "types", "category", "catch_rate", "friendship",
              "height_m", "weight_kg", "bst", "lore_for", "lore_against", "abilities", "pokedex"]
    with open(OUT, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
    print(f"{len(species)} species, {len(candidates)} candidates, {len(lines)} three-stage lines -> {OUT}")


if __name__ == "__main__":
    main()
