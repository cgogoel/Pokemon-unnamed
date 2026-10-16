// What each Act 1 Pokémon likes. See docs/hack/befriending.md (lineup chosen from docs/hack/act1_befriend_rankings.csv).
// Unlisted species use sBefriendDefault.

static const u8 sText_BefriendHint_Blipbug[] = _("It keeps peering up at you, as if\nit wants to study you at its level.");
static const u8 sText_BefriendHint_Tynamo[] = _("It flickers weakly, glancing around\nas if looking for others of its kind.");
static const u8 sText_BefriendHint_Hatenna[] = _("It flinches whenever you move.\nIt seems to want you to keep still.");
static const u8 sText_BefriendHint_Teddiursa[] = _("It keeps licking its paws and sniffing\nthe air for something sweet.");
static const u8 sText_BefriendHint_JangmoO[] = _("It clangs its scales at you.\nIt's daring you to stand your ground.");
static const u8 sText_BefriendHint_Default[] = _("It's eyeing you closely.");

static const struct BefriendInfo sBefriendInfo[] =
{
    // Tutorial: curious and harmless, it wants to look you over
    {
        .species = SPECIES_BLIPBUG,
        .lovedItem = ITEM_ORAN_BERRY,
        .lovedAction = BEFRIEND_ACTION_CROUCH,
        .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
        .patience = 6,
        .calmRate = 150,
        .watchHint = sText_BefriendHint_Blipbug,
    },
    // Weak on its own and lives in schools: it wants company
    {
        .species = SPECIES_TYNAMO,
        .lovedItem = ITEM_CHERI_BERRY,
        .dislikedItems = { ITEM_PECHA_BERRY },
        .lovedAction = BEFRIEND_ACTION_HOLD_OUT_HAND,
        .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
        .patience = 5,
        .calmRate = 100,
        .watchHint = sText_BefriendHint_Tynamo,
    },
    // Senses emotions and only warms to calm people: stay still and quiet
    {
        .species = SPECIES_HATENNA,
        .lovedItem = ITEM_CHESTO_BERRY,
        .dislikedItems = { ITEM_CHERI_BERRY },
        .lovedAction = BEFRIEND_ACTION_CROUCH,
        .dislikedActions = (1 << BEFRIEND_ACTION_APPROACH) | (1 << BEFRIEND_ACTION_HOLD_OUT_HAND),
        .patience = 4,
        .calmRate = 100,
        .watchHint = sText_BefriendHint_Hatenna,
    },
    // Loves honey and licks its sweet palms: hold out a hand
    {
        .species = SPECIES_TEDDIURSA,
        .lovedItem = ITEM_HONEY,
        .dislikedItems = { ITEM_RAWST_BERRY },
        .lovedAction = BEFRIEND_ACTION_HOLD_OUT_HAND,
        .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
        .patience = 5,
        .calmRate = 100,
        .watchHint = sText_BefriendHint_Teddiursa,
    },
    // Boss: grows by battling and respects boldness; gentleness reads as weakness. Calms down slowly.
    {
        .species = SPECIES_JANGMO_O,
        .lovedItem = ITEM_HARD_STONE,
        .dislikedItems = { ITEM_ORAN_BERRY, ITEM_PECHA_BERRY },
        .lovedAction = BEFRIEND_ACTION_APPROACH,
        .dislikedActions = (1 << BEFRIEND_ACTION_CROUCH) | (1 << BEFRIEND_ACTION_HUM),
        .patience = 6,
        .calmRate = 50,
        .watchHint = sText_BefriendHint_JangmoO,
    },
};

static const struct BefriendInfo sBefriendDefault =
{
    .species = SPECIES_NONE,
    .lovedItem = ITEM_NONE,
    .lovedAction = BEFRIEND_ACTION_COUNT, // Nothing in particular
    .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
    .patience = BEFRIEND_DEFAULT_PATIENCE,
    .calmRate = 100,
    .watchHint = sText_BefriendHint_Default,
};
