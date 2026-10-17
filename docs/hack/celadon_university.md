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

It follows the lore art (the Tamamushi University diagram): a wooded campus with the main hall across the middle.

| Spot | Building (placeholder look) | Doors |
|---|---|---|
| Middle | Main hall: the Department Store as the clock tower, Game Corner wings, the Restaurant's glass roof as the greenhouse behind | (29,27), (33,27); wings (18,26), (23,27), (39,27), (44,26) |
| In front of it | Gyarados fountain square, then the avenue to the gate, with a long reflecting pool and a row of trees | |
| Top left | Stadium: fenced, with a track | |
| Left | Research ranch: fenced | |
| Behind the east wing | Literature (two houses) | (38,18), (42,18) |
| Top right | Science Research Institute (Condominiums) | (53,15) |
| Bottom left | University hospital (Pokémon Center) and the pool, with a stone deck around it | (20,42) |
| By the gate | Dorms: two residence towers (Condominiums) | (39,44) lobby, (46,44) the player's room |
| Right | Home Economics (Gym) with a garden and pond | (53,38) |

The rest is filled with groves of trees that keep a one-tile walkway along every path and building
(`fill_with_trees`). Every door is reachable from the gate.

None of the buildings have interiors or warps yet. `python3 tools/hack/render_layout.py LAYOUT_CELADON_UNIVERSITY
out.png --map CeladonUniversity` previews it.

## Interiors

Every door has a room, each a copy of a FireRed interior layout (its own `LAYOUT_CELADON_UNIVERSITY_*`, so editing
it in Porymap leaves the original alone). The Main Hall's two doors lead to the same lobby.

| Map (`MAP_CELADON_UNIVERSITY_*`) | Door on campus | Copied from |
|---|---|---|
| `MAIN_HALL`, upstairs `MAIN_HALL_2F` (clock tower gallery; the elevator goes there too) | (29,27), (33,27) | Trainer Tower lobby; Pewter Museum 2F |
| `LECTURE_HALL_A` | (23,27) west wing | Viridian City school |
| `LECTURE_HALL_B` (audiovisual classroom) | (39,27) east wing | Fuchsia City warden's house |
| `CAFETERIA` | (18,26) | Celadon City restaurant |
| `FACULTY_OFFICE` | (44,26) | A FireRed house |
| `LITERATURE` | (38,18) | Saffron City Pokémon Trainer Fan Club |
| `LIBRARY` | (42,18) | Five Island Resort Gorgeous house |
| `SCIENCE_INSTITUTE`, with `SCIENCE_LOUNGE`, `SCIENCE_RESEARCH_ROOM`, `SCIENCE_EXPERIMENT_ROOM` | (53,15) | Cinnabar Island Pokémon Lab and its three rooms |
| `HOSPITAL` (the nurse heals), upstairs `HOSPITAL_2F` (the ward) | (20,42) | Pokémon Center 1F and 2F |
| `DORM_LOBBY`, upstairs `DORM_2F`, `DORM_3F` (the roof is locked) | (39,44) | Celadon Condominiums 1F-3F |
| `DORM_ROOM`, the player's | (46,44) | Lorelei's house. **Act 2 starts here**, at (2,3) beside the bed (`ACT2_START_MAP`) |
| `HOME_ECONOMICS` | (53,38) | Celadon City Gym (the garden) |

## Decoration

- **Outside** (`compose_university.py`): signposts at every building, lamp posts along the avenue and at the
  fountain, floodlights at the stadium, flower beds along the street. The signs' texts are bg events in the
  campus's `map.json`.
- **People and Pokémon**: students and staff everywhere, with flavor lines from the lore (the Gyarados fountain
  marks noon; the strongest Magikarp came from the Science Research Institute; the emblem's Magikarp and Gyarados
  mean anyone can grow with effort; Erika guest-lectures Home Economics). Ponyta on the ranch, Psyduck at the pool,
  a swimmer in it, Chansey at the hospital, Meowth and Clefairy in the dorm lobby, the professor's Slowpoke.
- **Furniture** has things to read: blackboards, bookshelves, computers, exhibits, notice boards. The dorm room has
  the player's things: a photo of the farm, the course schedule, and a Lapras doll from Mom.

Nothing here is story yet: no flags, no scenes, the same lines in every act.
