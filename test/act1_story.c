#include "global.h"
#include "act1_story.h"
#include "event_data.h"
#include "item.h"
#include "test/overworld_script.h"
#include "test/test.h"
#include "constants/story.h"
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

TEST("Act 1: caught by Jangmo-o, a following Blipbug runs back to the Woods and the story steps back")
{
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_BLIPBUG, 3; );
    FlagSet(FLAG_BEFRIENDED_BLIPBUG);
    VarSet(VAR_ACT1_PROGRESS, ACT1_BEFRIENDED_BLIPBUG);

    Act1_ReleaseFollowerToWoods();
    EXPECT_EQ(gSpecialVar_Result, TRUE);
    EXPECT_EQ(CalculatePlayerPartyCount(), 0);
    EXPECT(!FlagGet(FLAG_BEFRIENDED_BLIPBUG));
    EXPECT_EQ(VarGet(VAR_ACT1_PROGRESS), ACT1_DAD_SENT_TO_WOODS);
    VarSet(VAR_ACT1_PROGRESS, ACT1_START);
}

TEST("Act 1: a following Tynamo runs back, but the story only steps back if it was the last of the three")
{
    ZeroPlayerPartyMons();
    RUN_OVERWORLD_SCRIPT( givemon SPECIES_TYNAMO, 5; );
    FlagSet(FLAG_BEFRIENDED_TYNAMO);
    VarSet(VAR_ACT1_PROGRESS, ACT1_DAD_WARNED_JANGMO_O);

    Act1_ReleaseFollowerToWoods();
    EXPECT(!FlagGet(FLAG_BEFRIENDED_TYNAMO));
    EXPECT_EQ(VarGet(VAR_ACT1_PROGRESS), ACT1_DAD_WARNED_JANGMO_O);
    VarSet(VAR_ACT1_PROGRESS, ACT1_START);
}

TEST("Act 1: nothing runs back when nobody is following")
{
    ZeroPlayerPartyMons();
    Act1_ReleaseFollowerToWoods();
    EXPECT_EQ(gSpecialVar_Result, FALSE);
}
