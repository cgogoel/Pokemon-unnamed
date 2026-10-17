# Celadon University

Act 2's setting. The campus is its own map north of Celadon City, through a gate.

| Map | Layout | Notes |
|---|---|---|
| `MAP_CELADON_CITY` | `LAYOUT_CELADON_CITY` | Route 6's gate front stamped on the north edge, doors at (38,5)-(39,5) (warps 13, 14) |
| `MAP_CELADON_CITY_UNIVERSITY_GATE` | `LAYOUT_SAFFRON_CITY_NORTH_SOUTH_ENTRANCE` (shared with Saffron's gates) | A friendly guard; no thirsty-guard triggers |
| `MAP_CELADON_UNIVERSITY` | `LAYOUT_CELADON_UNIVERSITY` (64x56, Celadon's tilesets) | Gate at the bottom, warps at (31,49)-(32,49) |

## The base layout

`tools/hack/compose_university.py` builds the campus from pieces of FireRed's maps (with their collision), as a
starting point to refine in Porymap. Re-running it overwrites the campus layout, so after editing it in Porymap,
don't. `--celadon` also stamps the gate into Celadon City (already done; only needed again on a fresh Celadon).

| Spot | Building (placeholder look) | Door |
|---|---|---|
| Middle | Central building (Department Store), clock-tower fountain plaza in front | (29,23), (33,23) |
| Top left | Sports field: fenced, with a track | |
| Top middle | Literature (two Celadon houses) | |
| Top right | Science Research Institute (Condominiums) | |
| Left | University hospital (Pokémon Center), pool (Celadon's pond) | |
| Bottom left | Research ranch: fenced | |
| Bottom right | Home Economics (Gym) | |
| By the gate | Dorms (two row houses) | |

None of the buildings have interiors or warps yet. `python3 tools/hack/render_layout.py LAYOUT_CELADON_UNIVERSITY
out.png --map CeladonUniversity` previews it.

Act 2 still starts in the placeholder dorm room in Celadon's Condominiums (`ACT2_START_MAP`).
