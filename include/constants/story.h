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
#define ACT2_FIRE_PASSED            3  // Back at the dorms with the friends
#define ACT2_FIRE_INVITED           4  // The club meets at the stadium tonight
#define ACT2_CLUB_1_DONE            5  // Water course: its professor in the audiovisual classroom
#define ACT2_WATER_TRIP             6  // Field trip to Vermilion City (the harbor; Route 6)
#define ACT2_WATER_PASSED           7
#define ACT2_WATER_INVITED          8
#define ACT2_CLUB_2_DONE            9  // Grass course: its professor in Home Economics
#define ACT2_GRASS_TRIP             10 // Field trip to Viridian City (Route 22's grass)
#define ACT2_GRASS_PASSED           11
#define ACT2_GRASS_INVITED          12 // One more night: battle for a real Trainer Card
#define ACT2_GOT_TRAINER_CARD       13 // Trainers battle again. The director at the Science Research Institute waits.
#define ACT2_FINAL_PROJECT          14 // Catch a Pokémon that's two of Fire, Water and Grass
#define ACT2_PROJECT_DONE           15 // Act 2 is over

// The current course's research, in VAR_ACT2_RESEARCH (reset when a course starts)
#define ACT2_RESEARCH_USED_MOVE     (1 << 0) // The player's Pokémon used a move of the course's type
#define ACT2_RESEARCH_PUZZLE_1      (1 << 1) // The site's puzzle, its three parts
#define ACT2_RESEARCH_PUZZLE_2      (1 << 2)
#define ACT2_RESEARCH_PUZZLE_3      (1 << 3)
#define ACT2_RESEARCH_PUZZLE_ALL    (ACT2_RESEARCH_PUZZLE_1 | ACT2_RESEARCH_PUZZLE_2 | ACT2_RESEARCH_PUZZLE_3)
#define ACT2_RESEARCH_PUZZLE_DONE   (1 << 4) // Its end: the last task
#define ACT2_RESEARCH_GOT_TM        (1 << 5)
#define ACT2_RESEARCH_USED_TM_MOVE  (1 << 6) // In the exam, the player's Pokémon used the TM's move
#define ACT2_RESEARCH_BRIEFED       (1 << 7) // The professor explained the assignment at the site

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

#endif // GUARD_CONSTANTS_STORY_H
