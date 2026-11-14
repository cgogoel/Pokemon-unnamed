# Celadon University remake: exterior depth (design phase)

Feedback: the campus buildings look flat next to FireRed's own; keep the unique assets but give them FRLG's depth and
shading. This compares our buildings with FRLG's at the pixel level, says what makes ours flat, and offers three
options, each mocked up from a real (in-memory) build of the whole campus tileset so the budget numbers are honest.
Nothing in `data/` is changed yet.

**Applied: option B** (with the clock and greenhouse fixes below). `draw_university_tiles.py` now draws the B kit;
the draft that compared the options (`tools/hack/draft_exterior_depth.py`) is in this branch's history (commit
"University exterior depth: FRLG comparison, three shading options"); the option mockups below are from it.

## Applied (option B)

| | |
|---|---|
| Campus tileset | **380/384 tiles, 346/384 metatiles, 4/6 palettes** (was 365 / 371 / 4) |
| Recoveries | Science roof as a capsule, stadium track corners radius 32, pool ropes dashed every 4 px |
| Clock tower | The tower's cornice was drawn over the top of the clock's rim; the clock now sits below it (center 16 px lower, radius 13): the whole gold rim, the face, the hour marks and the hands at noon (`after_b_tower.png`) |
| Greenhouse | The mid-height band of leaf texture is gone: clear glass, the vault's lower panes bright with the light inside, a low planted bed with pink flowers behind the front glass. Trees inside the glass were tried and look livelier, but every tree cell is unique: 553/384 tiles, 169 over (`greenhouse_options.png`) |
| Rooms | Unchanged: they keep the first windows as `room_window()` / `room_sqwindow()` and the glass palette's steels |
| Maps | Collision, elevation and behaviors identical in all 4864 campus cells, no event moved; six stadium corner cells are now plain grass (the tighter track) |
| Images | `after_b_section.png`, `after_b_tower.png`, `after_b_greenhouse.png`, `after_b_stadium.png`, `after_b_science_roof.png` (before / after), `campus_option_b.png` (the whole campus) |

Differences from the B mockup: the plaques' shadow is under them (not offset), and the greenhouse changed (above).

| Image | What |
|---|---|
| `depth_overview.png` | The same corner (Lecture Hall A, the Main Hall's portico and tower) now / A / B / C, scale 2 |
| `depth_current.png`, `depth_a.png`, `depth_b.png`, `depth_c.png` | The section (Faculty Office, Lecture Hall A, Main Hall + tower, fountain square, hospital), scale 3 |
| `depth_beside_frlg_<option>.png` | Lecture Hall, hospital, Faculty Office next to Celadon's Dept. Store + condominium, the Pewter Museum and the Pokémon Mansion, all scale 3 on their own grass |
| `depth_recovery.png` | The tile savings B and C need, outside the section (Science roof, stadium, pool), before/after |

## How FRLG gives depth (measured on Celadon, Saffron/Silph, Pewter, Cinnabar, Indigo)

Light is from the upper left, but the big buildings are mostly **lit from above and symmetric**; the left/right cues
are small and repeated (pilaster edges, glass streaks, a 1-2 px drop shadow on the right and below).

1. **Walls are mid-value; white is only a highlight.** The facade's dominant color is one or two steps below white:
   Pewter Museum `#D5C5BD`, condominium `#94CDB4`, Pewter house `#ACBDCD`. The lightest color covers 3-8% of a facade.
2. **Roofs are planes, not texture.** A pale lit plane at the top (museum `#F6D5F6` over `#C573B4`; condominium
   `#D5DEA4` over `#B4BD83`), a long quiet main slope (seams every 8 px, one shade apart), then the eave: outline,
   a lit fascia, a shaded lip. Roof edge contrast is low (10-13, see below).
3. **The eave casts a shadow on the wall**: under the roof's outline, 1-2 rows of `#7B7B83` before the wall color.
4. **Corners and piers have a lit and a shaded face**: museum pilasters `#FFF6EE` | `#D5C5BD` | `#AC9C94` | `#7B7B83`;
   Silph's and the Dept. Store's sides are white faces that read as the building's flanks (a 3/4 view); the Pewter
   house's corner posts `#F6F6FF` + `#CDDEEE`.
5. **Windows are recessed**: a dark lintel/arch head over the glass (museum: 5 rows of `#394162`), graded glass
   (`#CDDEEE` top, `#73BDF6`, `#9CD5FF`), one diagonal highlight streak, a lit sill (`#CDB46A`, `#EEEEF6`).
6. **Doors sit in a dark recess** under a canopy or arch; steps at the foot.
7. **Buildings sit on the ground**: under the bottom outline a grey `#7B7B83` line (1 px on grass for houses, up to 5 on
   Silph's paving), and 2 px to the right on the Pewter house and the Mansion. A light base course above the outline.
8. **Outline**: `#414A6A` / `#52526A` (grey-violet), the same family as the interiors (`interior_art.md`). Ours is
   navy `#283048`: darker and more saturated, it reads as a hard ink line.
9. **Palettes**: the neutral ramp `#F6F6FF #CDDEEE #ACBDCD #94A4B4 #7B7B83 #5A5A73 #414A6A` is in almost every exterior
   tileset (General 2/3/5, Saffron, Pewter, Indigo, Sevii). Cool, slightly blue whites; mid saturation (~0.2-0.3).
10. **Dithering** is rare on buildings (Celadon's condominium wall dots, a few soft shadows); shading is flat bands.

Measured on the renders (building pixels only; "edge" = mean luminance step between vertically adjacent pixels):

| | dominant color (share) | lightest color's share | mean luminance | edge |
|---|---|---|---|---|
| **Our Main Hall facade** | `#F8F8F0` (60%) | **60%** | **211** | 15 |
| **Our hospital facade** | `#F8F8F0` (76%) | **76%** | **229** | 15 |
| **Our roof** | `#7080A8` (38%) | 6% | 135 | **38** |
| Pewter Museum facade | `#D5C5BD` (23%) | 8% | 163 | 21 |
| Museum roof | `#C573B4` (38%) | 2% | 165 | 10 |
| Celadon condominium | `#94CDB4` (42%) | 3% | 168 | 19 |
| Condominium roof | `#B4BD83` (52%) | 17% | 168 | 13 |
| Cinnabar Mansion facade | `#7B7B8B` (18%) | 4% | 161 | 28 |
| Pewter house | `#5A5A73` (31%) | 3% | 138 | 20 |
| Silph Co. | `#7BCDD5` (22%) | 3% | 158 | 26 |

## What makes ours flat

1. **The wall body is the brightest color** (`#F8F8F0`, 60-76% of the facade): nothing can be lighter, so there are
   no highlights, and the wall reads as paper. The only texture is a pale course line every 8 px.
2. **No shadow anywhere on the facade**: the cornice doesn't shade the wall, windows have no lintel shadow, the sills
   no shadow, the door no recess, the plinth no dark foot.
3. **Roofs are busy and uniform**: 4-px shingle courses with joints every 8 px, the same from ridge to eave (edge
   contrast 38 vs FRLG's 10-13), no lit plane, no shaded eave. They read as a fabric pattern, not a slope.
4. **No sides**: buildings are a flat front with a 1-px outline; no lit/shaded corners, no visible flank.
5. **They float**: no ground shadow; the plinth's last row is the building's edge.
6. **Navy outline** `#283048` vs FRLG's grey-violet `#52526A`; with the cream walls it gives an ink-on-paper look.
7. **Warm cream stone** (`#F8F8F0`/`#E0E0D0`) next to FRLG's cool neutrals (`#F6F6FF`/`#DEDEE6`): slightly yellow.
8. Portico, clock tower spire, fountain: shaded symmetrically (mirror-drawn), so nothing says where the light is.

## The options

All three keep the buildings, layout, stamps and doors as they are (same footprints and door cells; plaques stay by
the doors). Each includes the one before.

### A: shading pass (same palettes)

The wall body one step down (`#E0E0D0`) so white is free for highlights; quoins at the corners; the cornice re-cut
(lit top, fascia, dentils, shadowed underside) with a 2-row cast shadow on the wall; roofs as planes (ridge highlight,
lit upper slope, a curb, 8-px courses half as busy, a shaded last course); dormers with a hipped cap; windows with
the arch's soffit shadow, graded glass, keystone, lit sill and its shadow; square windows with a lit lintel; doors
with a shadowed soffit and steps; columns with capitals and bases; the porch in shadow; a lit plinth with a dark
foot and a 1-px ground shadow; the hospital's parapet roof with its inner faces; the fountain with a shaded basin
and its shadow on the paving. Outline recolored to `#52526A` in all four palettes (only that color changes).

Everything stays mirror-symmetric, so the flip deduplication still works. **379/384 tiles, 368/384 metatiles, 4/6
palettes** (+14 tiles). Fits as is, 5 tiles left.

Limits: the cream body (`#E0E0D0`) reads a little khaki next to FRLG's cool whites; no left/right light.

### B: A + FRLG palettes + light from the upper left

The four palettes' shared stone colors and the stone palette re-tuned to FRLG's exterior ramps (still 15 colors per
palette, still 4 palettes): stone `#F6F6FF #DEDEE6 #ACBDCD #7B7B83` (Celadon/Pewter's neutrals), slate = the
interiors' school slate `#739CC5 #5A7BB4 #41629C` with a lit-plane `#9CBDE6`, glass `#9CD5FF #62A4DE` (+ the slate's
dark), gold `#F6E68B #D5AC4A` (Celadon), wood `#C58341 #8B5229` (the Lab's), outline `#52526A`; the glass/water/
accent palettes take the same outline/whites/greys and FRLG steels `#CDCDD5 #94A4B4 #5A5A73`.

Light from the upper left on the cues that repeat (cheap) plus the landmark: roof verges and wall corners lit on the
left, shaded on the right; **pilasters** between the bays of the arched facades (museum style: lit edge, body, shaded
edge); porch columns lit on the left; dormer caps with a shaded right slope; **the clock tower's spire with two
faces** (lit left, shaded right, a lit hip line). (The glass streak, +6, didn't fit: it's in C.)

**381/384 tiles, 346/384 metatiles, 4/6 palettes** with the recovery below (R1-R3). Without it: 412.

### C: B + structural depth

A diagonal highlight in each window pane; roofs get **end boards** (the museum's: lit on the left end, shaded on the right); the right corner becomes a 6-px
**shaded side wall** (on the tower too), a 3/4 hint like Silph's flanks; **window reveals** (the left reveal in
shadow, the right lit, an architrave that steps the window in); a **stepped portal** (shadowed left jamb, lit right
jamb, a second step); columns and dormers **cast shadows** (on the porch's back wall, on the roof); each building
**casts a 2-px shadow on the grass to its right** (the building is 2 px narrower than its stamp; the stamp and the
layout don't change).

**402/384 tiles: over by 18** even with every recovery below (R1-R4), 342/384 metatiles, 4/6 palettes. The mockup is
rendered past the 10-bit tile limit (draft-only renderer) so you can judge the look. To ship it, ~20 more tiles must
go: e.g. simplify the fountain (31 tiles) or the Science podium and door (13), or drop the spire's second face (-10)
and the reveals (-8).

## The budget

The campus tileset is at 365/384 tiles now. Symmetric shading is cheap (the flip deduplication halves every
building); **anything lit from one side costs the right half of whatever it's on**, because those tiles were flips of
the left half. Repeated elements pay once per kind; big unique ones (tower, pediment, fountain, portico, the
hospital's cross) pay in full. Measured costs on top of the symmetric base (tiles, each alone):

| Cue | Tiles | In | Cue | Tiles | In |
|---|---|---|---|---|---|
| edges (verges, corners) | 10 | B C | pediment rakes | 15 | (neither) |
| pilasters | 7 | B C | tower: whole tower lit | 19 | (neither) |
| glass streak | 6 | C | tower: spire faces only | 10 | B C |
| columns lit left | 4 | B C | hospital (parapet, cross) | 14 | (neither) |
| dormer caps | 2 | B C | fountain (offset shadow) | 12 | (neither) |
| canopy shadow | 0 | B C | door jambs | 10 | (neither) |
| sill/plaque offset shadows | 6 | (neither) | end boards | 5 | C |
| side wall | 7 | C | portal | 8 | C |
| reveals | 8 | C | column shadows | 3 | C |
| dormer shadows | 2 | C | cast shadow on the grass | 14 | C |

Combined, C's structural cues cost little on top of B (they ride on tiles B already made unique).

Recovery (outside the section; `depth_recovery.png`):

| | Change | Tiles |
|---|---|---|
| R1 | The Science Institute's roof: a capsule (straight middle, round ends) instead of a 144-px ellipse; same hub | -13 |
| R2 | The stadium track's corner radius 52 -> 32 (36 gives -11; under ~28 the 26-px track fills the infield) | -14 |
| R3 | The pool's lane ropes dashed every 4 px (6 px doesn't divide the 8-px tiles) | -4 |
| R4 | The greenhouse vault's ends radius 24 -> 12 | -1 |

Tried and dropped: putting the lighting in the metatile's second layer as an overlay over the mirrored art (costs
more: baking already reuses the left half's tiles for every translated window/pier); centering the stadium's aisles
in a cell (no tile saving). The 23 blank tiles at the end of `gTileset_General_Frlg` could in principle hold campus
tiles, but that touches the shared primary: a last resort, not proposed.

Palettes: all options keep 4 of 6 slots. Metatiles are never the limit (B and C drop to ~345 with R1-R2).

## Recommendation

**B.** It fixes all eight causes of flatness that matter at play scale (mid-value walls, roof planes, cornice and
window shadows, ground shadow, FRLG's outline and cool neutrals, pilasters, left light on corners/glass, the 3D
spire), it sits beside Celadon and Pewter without looking like another game, and it fits (381/384) for the price of
a capsule roof on the Science tower, slightly tighter stadium corners and evenly dashed pool ropes. A is the safe fallback if you want zero changes
outside the buildings in the section (it fits with no recovery, but stays a bit khaki and flat-lit). C's extra
structure (side walls, reveals, cast shadows, glass streaks) is visible mostly when zoomed in and is 18 tiles over; its best parts
can be added to B later one at a time if tiles are freed (the side wall at 7 and the end boards at 5 are the most
noticeable per tile).

Implementing the chosen option: move the kit changes from the draft into `draw_university_tiles.py` (palettes, the
symmetric patterns, the cue set), apply the recoveries to `science()`/`stadium()`/`pool()`, re-run it (it rewrites
the tileset, stamps and mockups), rebuild the campus with `build_university_maps.py` (no event moves: footprints and
doors are unchanged), build both ROMs, and check in mGBA. The interiors' windows reuse `window()`/`sqwindow()`
recolored, so check those rooms' mockups too.
