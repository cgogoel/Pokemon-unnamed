#!/usr/bin/env python3
"""Converts src/data/triple_battle_moves.csv into src/data/triple_battle_moves.h.

The CSV is hand-edited; see docs/hack/triple_battles.md for what each column means.
Only the move, aoe, hits_allies and reach columns are used; the rest are notes for the editor.
"""

import csv
import sys

CSV_PATH = "src/data/triple_battle_moves.csv"
HEADER_PATH = "src/data/triple_battle_moves.h"

AOE_VALUES = {"1", "2", "3"}
YES_NO = {"Y": "TRUE", "N": "FALSE"}
REACH_VALUES = {"FRONT": "TRIPLE_REACH_FRONT", "ANY": "TRIPLE_REACH_ANY"}


def main():
    errors = []
    entries = []
    seen = set()

    with open(CSV_PATH, newline="", encoding="utf-8") as f:
        for line, row in enumerate(csv.DictReader(f), start=2):
            move = (row.get("move") or "").strip()
            aoe = (row.get("aoe") or "").strip()
            allies = (row.get("hits_allies") or "").strip().upper()
            reach = (row.get("reach") or "").strip().upper()

            if not move:
                continue
            if not move.startswith("MOVE_"):
                errors.append(f"line {line}: move '{move}' should be a MOVE_ constant")
            if move in seen:
                errors.append(f"line {line}: {move} is listed twice")
            if aoe not in AOE_VALUES:
                errors.append(f"line {line}: {move} aoe must be 1, 2 or 3, not '{aoe}'")
            if allies not in YES_NO:
                errors.append(f"line {line}: {move} hits_allies must be Y or N, not '{allies}'")
            if reach not in REACH_VALUES:
                errors.append(f"line {line}: {move} reach must be FRONT or ANY, not '{reach}'")
            seen.add(move)
            entries.append((move, aoe, YES_NO.get(allies), REACH_VALUES.get(reach)))

    if errors:
        for error in errors:
            print(f"{CSV_PATH}: {error}", file=sys.stderr)
        sys.exit(1)

    with open(HEADER_PATH, "w", encoding="utf-8") as f:
        f.write("// DO NOT MODIFY THIS FILE! It is auto-generated from src/data/triple_battle_moves.csv\n")
        f.write("// by tools/triple_battle_moves/triple_moves_to_header.py. Edit the CSV instead.\n\n")
        f.write("const struct TripleMoveInfo gTripleMoveInfo[MOVES_COUNT] =\n{\n")
        for move, aoe, allies, reach in entries:
            f.write(f"    [{move}] = {{ .aoe = {aoe}, .hitsAllies = {allies}, .reach = {reach} }},\n")
        f.write("};\n")


if __name__ == "__main__":
    main()
