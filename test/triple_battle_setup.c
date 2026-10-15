#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "party_menu.h"
#include "triple_battle.h"
#include "constants/story.h"
#include "test/overworld_script.h"
#include "test/test.h"

static void GivePlayerMons(u32 count)
{
    ZeroPlayerPartyMons();
    for (u32 i = 0; i < count; i++)
        RUN_OVERWORLD_SCRIPT( givemon SPECIES_PIKACHU, 20; );
}

TEST("Triple battles: doubles and triples trainers fight a double battle in Act 1")
{
    GivePlayerMons(3);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    EXPECT_EQ(GetTripleBattleTypeFlagsForFoes(4), BATTLE_TYPE_DOUBLE);
}

TEST("Triple battles: from Act 2 the field size is the smaller of the two sides, up to 3")
{
    u32 act, playerMons, foes, flags;
    PARAMETRIZE { act = STORY_ACT_UNIVERSITY; playerMons = 3; foes = 4; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE; }
    PARAMETRIZE { act = STORY_ACT_WORKFORCE;  playerMons = 6; foes = 4; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE; }
    PARAMETRIZE { act = STORY_ACT_UNIVERSITY; playerMons = 3; foes = 2; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE | BATTLE_TYPE_TRIPLE_2V2; }
    PARAMETRIZE { act = STORY_ACT_UNIVERSITY; playerMons = 2; foes = 4; flags = BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE | BATTLE_TYPE_TRIPLE_2V2; }
    PARAMETRIZE { act = STORY_ACT_UNIVERSITY; playerMons = 1; foes = 4; flags = 0; }
    PARAMETRIZE { act = STORY_ACT_UNIVERSITY; playerMons = 3; foes = 1; flags = 0; }
    GivePlayerMons(playerMons);
    VarSet(VAR_STORY_ACT, act);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    EXPECT_EQ(GetTripleBattleTypeFlagsForFoes(foes), flags);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Triple battles: fainted Pokémon don't count toward the field size")
{
    u32 hp = 0;
    GivePlayerMons(3);
    SetMonData(&gParties[B_TRAINER_PLAYER][1], MON_DATA_HP, &hp);
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    EXPECT_EQ(GetTripleBattleTypeFlagsForFoes(4), BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TRIPLE | BATTLE_TYPE_TRIPLE_2V2);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
}

TEST("Triple battles: battles against two trainers or with a partner stay double battles")
{
    GivePlayerMons(3);
    VarSet(VAR_STORY_ACT, STORY_ACT_UNIVERSITY);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS;
    EXPECT_EQ(GetTripleBattleTypeFlagsForFoes(4), BATTLE_TYPE_DOUBLE);
    VarSet(VAR_STORY_ACT, STORY_ACT_CHILDHOOD);
    gBattleTypeFlags = 0;
}
