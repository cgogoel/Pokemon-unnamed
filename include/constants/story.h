#ifndef GUARD_CONSTANTS_STORY_H
#define GUARD_CONSTANTS_STORY_H

// The hack's acts (see CLAUDE.md, "Hack design"), stored in VAR_STORY_ACT. A new game starts in Act 1.
#define STORY_ACT_CHILDHOOD     0 // Prologue / Act 1: no Poké Balls, befriending, one Pokémon follows
#define STORY_ACT_UNIVERSITY    1 // Act 2: Poké Balls, teams of up to 3, 3v3 battles
#define STORY_ACT_WORKFORCE     2 // Act 3: Silph Co.

// Act 1's story so far, in VAR_ACT1_PROGRESS (docs/hack/act1_story.md). Map coord events use the numbers (11, 12 in
// the Farmyard's map.json): map data doesn't see these names.
#define ACT1_START                  0  // Oak didn't choose the player; the Pokédex waits at his lab
#define ACT1_GOT_POKEDEX            1
#define ACT1_MOM_SENT_TO_DAD        2  // Mom: go talk to Dad in the barn
#define ACT1_DAD_SENT_TO_WOODS      3  // Dad: be kind and patient; an Oran Berry for Blipbug. The Woods open.
#define ACT1_BEFRIENDED_BLIPBUG     4  // Mom: Dad wants to talk again
#define ACT1_DAD_WARNED_JANGMO_O    5  // Dad: there are others in the Woods, but avoid Jangmo-o
#define ACT1_BEFRIENDED_THREE       6  // Tynamo, Hatenna and Teddiursa befriended
#define ACT1_DAD_SAYS_TOO_DANGEROUS 7  // Dad: Jangmo-o is still too dangerous; go see Mom
#define ACT1_MOM_SAYS_FOLLOW_HEART  8  // Mom: follow your heart. Dad gives the Hard Stone next.
#define ACT1_GOT_HARD_STONE         9  // Talking to Jangmo-o starts its encounter
#define ACT1_BEFRIENDED_JANGMO_O    10 // Bringing it home: Mom and Dad can't believe it, and send the player to rest
#define ACT1_RESTED                 11 // Mom and Dad: show the Pokédex to Oak. Oak arrives as the player leaves.
#define ACT1_CHOOSING_FRIENDS       12 // Oak: Celadon University; choose two friends to bring. Then the prologue ends.

// Act 2's story so far, in VAR_ACT2_PROGRESS (docs/hack/act2_story.md). Map coord events use the numbers: map data
// doesn't see these names. Until ACT2_GOT_TRAINER_CARD, trainers won't battle the player (Act2_TrainersLocked).
#define ACT2_START                  0  // Woke up in the dorm room; the friends wait in the dorm lobby
#define ACT2_MET_FRIENDS            1  // Fire course: its professor in Lecture Hall A
#define ACT2_FIRE_TRIP              2  // Field trip to Cinnabar Island: catch a Fire type, pass the exam
#define ACT2_FIRE_SEE_KAI           3  // Passed; back at the dorms at night: KAI asked the player to come by his room
#define ACT2_FIRE_INVITED           4  // KAI: the club meets at the Sports Complex tonight
#define ACT2_CLUB_1_DONE            5  // Water course: its professor in the audiovisual classroom
#define ACT2_WATER_TRIP             6  // Field trip to Vermilion City (the harbor; Route 6)
#define ACT2_WATER_SEE_MINA         7  // Back at night: see MINA in her room
#define ACT2_WATER_INVITED          8
#define ACT2_CLUB_2_DONE            9  // Grass course: its professor in Home Economics
#define ACT2_GRASS_TRIP             10 // Field trip to Viridian City at night (Viridian Forest's night bloom)
#define ACT2_GRASS_SEE_ROWAN        11 // Back at night: see ROWAN in their room
#define ACT2_GRASS_INVITED          12 // One more night: battle for a real Trainer Card
#define ACT2_GOT_TRAINER_CARD       13 // Trainers battle again. The director's final project brief waits at the institute.
#define ACT2_FINAL_PROJECT          14 // Briefed: free to explore; levels scaled (docs/hack/act2_finale.md)
#define ACT2_PARENTS_TALKED         15 // Mom and Dad: meditate in the Woods. The ranger station opens.
#define ACT2_LEGENDS_CALMED         16 // Dad's Mew stopped the legendary trio; the friends visit the player's room
#define ACT2_DONE                   17 // Tri-Blast made: Act 2 is over

// The current course's research, in VAR_ACT2_RESEARCH (reset when a course starts)
#define ACT2_RESEARCH_USED_MOVE     (1 << 0) // The player's Pokémon used a move of the course's type
#define ACT2_RESEARCH_PUZZLE_1      (1 << 1) // The site's puzzle, its three parts
#define ACT2_RESEARCH_PUZZLE_2      (1 << 2)
#define ACT2_RESEARCH_PUZZLE_3      (1 << 3)
#define ACT2_RESEARCH_PUZZLE_ALL    (ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2 | ACT2_RESEARCH_PUZZLE_3)
#define ACT2_RESEARCH_PUZZLE_DONE   (1 << 4) // Its end: the last task
#define ACT2_RESEARCH_GOT_TM        (1 << 5)
#define ACT2_RESEARCH_BRIEFED       (1 << 7) // The professor explained the assignment at the site
// Act2_GetDockFireState
#define DOCK_FIRE_NONE    0
#define DOCK_FIRE_BURNING 1
#define DOCK_FIRE_OUT     2

#define ACT2_RESEARCH_DOCK_FIRE          (1 << 10) // Water: a WATER Pokémon was caught on the trip, and the docks caught fire
#define ACT2_RESEARCH_DOCK_FIRE_SEEN     (1 << 11) // Water: a friend took the player to see the fire
#define ACT2_RESEARCH_DOCK_FIRE_ARRIVING (1 << 12) // Water: on the way there (the dock scene plays on arrival)
#define ACT2_RESEARCH_CLUB_ARRIVED  (1 << 9) // A club night: the friends explained tonight's battle (cleared by the next course)
#define ACT2_RESEARCH_MOVES_DONE    (1 << 8) // A third different move of the type was seen (after VAR_ACT2_MOVE_SEEN_1/2)

// The Grass course's night bloom (docs/hack/act2_story.md), bits of VAR_ACT2_NIGHT_BLOOM (reset when the course
// starts). Puzzle part 1 is all three glowcaps, part 2 all three ODDISH, part 3 all three waves.
#define NIGHT_BLOOM_GLOWCAP_1       (1 << 0)  // Viridian Forest: a glowcap woken (each widens the light)
#define NIGHT_BLOOM_GLOWCAP_2       (1 << 1)
#define NIGHT_BLOOM_GLOWCAP_3       (1 << 2)
#define NIGHT_BLOOM_ODDISH_1        (1 << 3)  // An ODDISH held still: it showed the next leg of the way
#define NIGHT_BLOOM_ODDISH_2        (1 << 4)
#define NIGHT_BLOOM_ODDISH_3        (1 << 5)  // The last one leads to the glade's opening
#define NIGHT_BLOOM_WAVE_1          (1 << 6)  // The glade: a wave of BUG POKéMON beaten or kept out
#define NIGHT_BLOOM_WAVE_2          (1 << 7)
#define NIGHT_BLOOM_WAVE_3          (1 << 8)
#define NIGHT_BLOOM_HEDGE_1         (1 << 9)  // A hedge planted in that wave's gap
#define NIGHT_BLOOM_HEDGE_2         (1 << 10)
#define NIGHT_BLOOM_HEDGE_3         (1 << 11)
#define NIGHT_BLOOM_ARRIVED         (1 << 12) // The glade: the bud and the MORELULL were found
#define NIGHT_BLOOM_DAWN            (1 << 13) // The night is over: day for the rest of the trip
#define NIGHT_BLOOM_FERN            (1 << 14) // PROF. FERN came to the glade (the TM and the exam are there)
#define NIGHT_BLOOM_GLOWCAPS        (NIGHT_BLOOM_GLOWCAP_1 | NIGHT_BLOOM_GLOWCAP_2 | NIGHT_BLOOM_GLOWCAP_3)
#define NIGHT_BLOOM_ODDISH          (NIGHT_BLOOM_ODDISH_1 | NIGHT_BLOOM_ODDISH_2 | NIGHT_BLOOM_ODDISH_3)
#define NIGHT_BLOOM_WAVES           (NIGHT_BLOOM_WAVE_1 | NIGHT_BLOOM_WAVE_2 | NIGHT_BLOOM_WAVE_3)

// specialvar VAR_RESULT, Act2_GetGladeScene: what's due in the glade
#define GLADE_SCENE_NONE            0
#define GLADE_SCENE_ARRIVAL         1 // First time in: the bud and the MORELULL, then the vigil
#define GLADE_SCENE_VIGIL           2 // The waves of BUG POKéMON (one left off: they're still circling)
#define GLADE_SCENE_DAWN            3 // The waves are over: dawn, the bloom, the dew
#define GLADE_SCENE_MORNING         4 // The MORELULL comes along, PROF. FERN arrives

// The MORELULL that lit the glade all night, which asks to come along (FLAG_ACT2_GOT_MORELULL)
#define ACT2_MORELULL_LEVEL         18

// Day and night are the game's (VAR_GAME_TIME, src/game_time.c): the clock doesn't decide them
#define GAME_TIME_DAY               0
#define GAME_TIME_NIGHT             1

// specialvar VAR_RESULT, GameTime_GetDoomScrollState
#define DOOM_SCROLL_USABLE          0
#define DOOM_SCROLL_TOO_EARLY       1 // Before the final project (Act 2) or in Act 1
#define DOOM_SCROLL_STORY_TIME      2 // The story holds the time (a club night, the Grass trip's night)

// The young Lapras saved on Vermilion's docks (the Water course), which asks to come along. In Act 3 it's the
// Lapras in Silph Co.'s lab that masters the ultimate move and goes to Red (FLAG_ACT2_GOT_LAPRAS).
#define ACT2_LAPRAS_LEVEL           15

// The time skip: the Pokémon the player brings to university are at least this level
#define ACT2_START_LEVEL            10

// In Act 2, wild Pokémon are the party's average level, give or take this many (Act2_GetWildLevel)
#define ACT2_WILD_LEVEL_SPREAD      3

// Where Act 2 starts: the player's room in the Celadon University dorms
#define ACT2_START_MAP              MAP_CELADON_UNIVERSITY_DORM_ROOM
#define ACT2_START_X                2
#define ACT2_START_Y                3

// Where Act 3 starts: the player's lab at Silph Co., the room of research terminals on 7F (the floor where the
// Lapras saved in Act 2 is kept). The player stands at the terminals.
#define ACT3_START_MAP              MAP_SILPH_CO_7F
#define ACT3_START_X                3
#define ACT3_START_Y                12

// The final project's ranger station (docs/hack/act2_finale.md), bits of VAR_ACT2_FINALE
#define ACT2_FINALE_BASEMENT_OPEN   (1 << 0) // 1F: the wall switch opened the stair behind the counter
#define ACT2_FINALE_CLOCK_SOLVED    (1 << 1) // B1F: 4:15 a.m. set; the gate around the stair is open
#define ACT2_FINALE_MEDKIT_USED     (1 << 2) // B2F: the medkit was opened (it can be used again)
#define ACT2_FINALE_GRASS_CROSSED   (1 << 3) // B3F: walked the hidden path once
#define ACT2_FINALE_CLOCK_SET       (1 << 4) // B1F: VAR_ACT2_CLOCK set to ACT2_CLOCK_START (on the first visit)

// VAR_ACT2_CLOCK: bit (n - 1) for the torch at n o'clock, then the moon and sun furnaces
#define ACT2_CLOCK_MOON             12
#define ACT2_CLOCK_SUN              13
#define ACT2_CLOCK_START            ((1 << (10 - 1)) | (1 << (2 - 1))) // Lit at the start: 10 and 2
#define ACT2_CLOCK_SOLUTION         ((1 << (4 - 1)) | (1 << (3 - 1)) | (1 << ACT2_CLOCK_MOON)) // 4:15 a.m.

// The final project's levels, against the party's average (Act2_GetWildLevel, Act2_ScaleTrainerParty)
#define ACT2_ROUTE_WILD_OFFSET      (-3) // On the direct route home: wild Pokémon, from this up to the average
#define ACT2_ROUTE_TRAINER_OFFSET   0    // On the direct route home: trainers' Pokémon
#define ACT2_OFF_ROUTE_OFFSET       15   // Anywhere else: both

// The legendary trio in the ranger station's chamber
#define ACT2_LEGENDS_LEVEL          50

#endif // GUARD_CONSTANTS_STORY_H
