#ifndef GUARD_CONSTANTS_STORY_H
#define GUARD_CONSTANTS_STORY_H

// The hack's acts (see CLAUDE.md, "Hack design"), stored in VAR_STORY_ACT. A new game starts in Act 1.
#define STORY_ACT_CHILDHOOD     0 // Prologue / Act 1: no Poké Balls, befriending, one Pokémon follows
#define STORY_ACT_UNIVERSITY    1 // Act 2: Poké Balls, teams of up to 3, 3v3 battles
#define STORY_ACT_WORKFORCE     2 // Act 3: Silph Co.

// Act 1's story so far, in VAR_ACT1_PROGRESS (docs/hack/act1_story.md)
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
#define ACT1_BEFRIENDED_JANGMO_O    10

#endif // GUARD_CONSTANTS_STORY_H
