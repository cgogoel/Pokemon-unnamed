#include "global.h"
#include "test/battle.h"

// The hack: KAI's dads' homemade POKé WRAP (data/scripts/friend_homes.inc)

SINGLE_BATTLE_TEST("Poké Wrap restores 75 HP")
{
    GIVEN {
        ASSUME(gItemsInfo[ITEM_POKE_WRAP].battleUsage == EFFECT_ITEM_RESTORE_HP);
        ASSUME(GetItemPocket(ITEM_POKE_WRAP) == POCKET_ITEMS);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); MaxHP(400); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_WRAP, partyIndex: 0); }
    } SCENE {
        HP_BAR(player, damage: -75);
    }
}
