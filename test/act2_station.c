#include "global.h"
#include "act2_station.h"
#include "event_data.h"
#include "string_util.h"
#include "test/test.h"
#include "constants/characters.h"
#include "constants/story.h"

// The ranger station's B1F (docs/hack/act2_finale.md): the notes round the clock room

TEST("The ranger station: the notes found are counted, the others not")
{
    u16 count;
    VarSet(VAR_ACT2_FINALE, ACT2_FINALE_CLOCK_SET | ACT2_FINALE_LAB_GUARD);
    EXPECT_EQ(Act2Station_CountNotes(), 0);
    VarSet(VAR_ACT2_FINALE, ACT2_FINALE_CLOCK_SET | ACT2_FINALE_NOTE_1 | ACT2_FINALE_NOTE_4 | ACT2_FINALE_NOTE_5);
    count = Act2Station_CountNotes();
    EXPECT_EQ(count, 3);
    EXPECT_EQ(gStringVar1[0], CHAR_3);
    VarSet(VAR_ACT2_FINALE, ACT2_FINALE_NOTES);
    EXPECT_EQ(Act2Station_CountNotes(), ACT2_STATION_NOTE_COUNT);
    VarSet(VAR_ACT2_FINALE, 0);
}
