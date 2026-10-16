#!/usr/bin/env python3
"""Deletes script code that nothing reaches any more from a map's scripts.inc.

Used when stripping the base game's scenes: detach a scene first (its map.json event, map_script entry or goto), then
run this on the map to drop the code and texts that only the scene used. See docs/hack/act1_story.md.

    python3 tools/hack/prune_dead_scripts.py data/maps/<Map>/scripts.inc [--dry-run]

A label is kept if anything outside the file mentions it (map.json events, other scripts, C code) or if kept code
mentions it or falls through into it. Generated files (events.inc, build/) don't count.
"""
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
SEARCH_DIRS = ['data', 'src', 'include', 'test']
SKIP_FILES = {'events.inc', 'header.inc', 'connections.inc'}
LABEL = re.compile(r'^(\w+):{1,2}')
# A block ending with one of these doesn't run on into the next label
TERMINAL = re.compile(r'^\s*(?:(?:end|return|step_end)\b|goto\s|\.2byte\s+0\b|\.byte\s+0\b|\.string\s.*\$")')


def read_blocks(path):
    """The file split at labels: (label or None, lines)."""
    blocks = [(None, [])]
    for line in open(path).read().split('\n'):
        match = LABEL.match(line)
        if match:
            blocks.append((match.group(1), [line]))
        else:
            blocks[-1][1].append(line)
    return blocks


def outside_mentions(labels, path):
    found = set()
    pattern = re.compile(r'\b(' + '|'.join(map(re.escape, labels)) + r')\b')
    for top in SEARCH_DIRS:
        for folder, _, files in os.walk(os.path.join(ROOT, top)):
            for name in files:
                full = os.path.join(folder, name)
                if name in SKIP_FILES or os.path.samefile(full, path):
                    continue
                if not name.endswith(('.inc', '.s', '.c', '.h', '.json')):
                    continue
                try:
                    found.update(pattern.findall(open(full, errors='ignore').read()))
                except OSError:
                    pass
    return found


def code_lines(lines):
    return [l for l in lines if l.strip() and not l.strip().startswith('@')]


def main():
    path = sys.argv[1]
    blocks = read_blocks(path)
    labels = [label for label, _ in blocks if label]
    keep = outside_mentions(labels, path)
    keep.update(label for label, lines in blocks if label and label.endswith('_MapScripts'))
    mention = re.compile(r'\b(' + '|'.join(map(re.escape, labels)) + r')\b')
    changed = True
    while changed:
        changed = False
        for i, (label, lines) in enumerate(blocks):
            if label is not None and label not in keep:
                continue
            body = lines[1:] if label else lines
            for name in mention.findall('\n'.join(l.split('@')[0] for l in body)):
                if name not in keep:
                    keep.add(name)
                    changed = True
            code = code_lines(body)
            if label and i + 1 < len(blocks) and (not code or not TERMINAL.match(code[-1])):
                nxt = blocks[i + 1][0]
                if nxt and nxt not in keep:
                    keep.add(nxt)
                    changed = True
    removed = [label for label in labels if label not in keep]
    for label in removed:
        print('remove', label)
    if '--dry-run' in sys.argv or not removed:
        return
    out = []
    for label, lines in blocks:
        if label is None or label in keep:
            out.extend(lines)
    text = re.sub(r'\n{3,}', '\n\n', '\n'.join(out))
    open(path, 'w').write(text)


if __name__ == '__main__':
    main()
