#include "global.h"
#include "act1_story.h"
#include "event_data.h"
#include "item.h"
#include "test/test.h"
#include "constants/items.h"

// Act 1's story, see docs/hack/act1_story.md

TEST("Act 1: an offered loved item comes back where it was found while its Pokémon isn't befriended")
{
    FlagClear(FLAG_BEFRIENDED_TYNAMO);
    FlagSet(FLAG_ACT1_ITEM_CHERI_BERRY); // Picked up...
    RemoveBagItem(ITEM_CHERI_BERRY, MAX_BAG_ITEM_CAPACITY); // ...and offered
    Act1_RestockLovedItems();
    EXPECT(!FlagGet(FLAG_ACT1_ITEM_CHERI_BERRY));
}

TEST("Act 1: a loved item still in the bag doesn't come back")
{
    FlagClear(FLAG_BEFRIENDED_HATENNA);
    FlagSet(FLAG_ACT1_ITEM_CHESTO_BERRY);
    AddBagItem(ITEM_CHESTO_BERRY, 1);
    Act1_RestockLovedItems();
    EXPECT(FlagGet(FLAG_ACT1_ITEM_CHESTO_BERRY));
    RemoveBagItem(ITEM_CHESTO_BERRY, 1);
}

TEST("Act 1: once its Pokémon is befriended, a loved item doesn't come back")
{
    FlagSet(FLAG_BEFRIENDED_TEDDIURSA);
    FlagSet(FLAG_ACT1_ITEM_HONEY);
    RemoveBagItem(ITEM_HONEY, MAX_BAG_ITEM_CAPACITY);
    Act1_RestockLovedItems();
    EXPECT(FlagGet(FLAG_ACT1_ITEM_HONEY));
    FlagClear(FLAG_BEFRIENDED_TEDDIURSA);
}
