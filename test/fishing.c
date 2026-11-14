#include "global.h"
#include "fishing.h"
#include "item.h"
#include "test/test.h"
#include "constants/items.h"

// Pressing A facing water with a rod in the bag: SURF or FISH (data/scripts/fish_or_surf.inc), the best rod casts

TEST("Fishing: A on water casts the best rod in the bag")
{
    RemoveBagItem(ITEM_OLD_ROD, MAX_BAG_ITEM_CAPACITY);
    RemoveBagItem(ITEM_GOOD_ROD, MAX_BAG_ITEM_CAPACITY);
    RemoveBagItem(ITEM_SUPER_ROD, MAX_BAG_ITEM_CAPACITY);
    EXPECT_EQ(GetBestFishingRodItem(), ITEM_NONE); // No rod: A on water is the usual SURF prompt
    AddBagItem(ITEM_OLD_ROD, 1);
    EXPECT_EQ(GetBestFishingRodItem(), ITEM_OLD_ROD);
    AddBagItem(ITEM_SUPER_ROD, 1);
    EXPECT_EQ(GetBestFishingRodItem(), ITEM_SUPER_ROD);
    AddBagItem(ITEM_GOOD_ROD, 1);
    EXPECT_EQ(GetBestFishingRodItem(), ITEM_SUPER_ROD);
    RemoveBagItem(ITEM_SUPER_ROD, 1);
    EXPECT_EQ(GetBestFishingRodItem(), ITEM_GOOD_ROD);
    RemoveBagItem(ITEM_OLD_ROD, 1);
    RemoveBagItem(ITEM_GOOD_ROD, 1);
}
