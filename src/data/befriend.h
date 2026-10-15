// What each Act 1 Pokémon likes. See docs/hack/befriending.md
// Placeholders until the Act 1 Pokémon are decided; unlisted species use sBefriendDefault.

static const u8 sText_BefriendHint_Eevee[] = _("It keeps sniffing at your bag,\nand its ears perk up at any sound.");
static const u8 sText_BefriendHint_Pidgey[] = _("It's watching the ground nearby\nfor something to peck at.");
static const u8 sText_BefriendHint_Default[] = _("It's eyeing you closely.");

static const struct BefriendInfo sBefriendInfo[] =
{
    {
        .species = SPECIES_EEVEE,
        .lovedItem = ITEM_ORAN_BERRY,
        .dislikedItems = { ITEM_PECHA_BERRY },
        .lovedAction = BEFRIEND_ACTION_HUM,
        .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
        .patience = 5,
        .watchHint = sText_BefriendHint_Eevee,
    },
    {
        .species = SPECIES_PIDGEY,
        .lovedItem = ITEM_LEPPA_BERRY,
        .dislikedItems = { ITEM_ORAN_BERRY },
        .lovedAction = BEFRIEND_ACTION_CROUCH,
        .dislikedActions = (1 << BEFRIEND_ACTION_HUM) | (1 << BEFRIEND_ACTION_HOLD_OUT_HAND),
        .patience = 4,
        .watchHint = sText_BefriendHint_Pidgey,
    },
};

static const struct BefriendInfo sBefriendDefault =
{
    .species = SPECIES_NONE,
    .lovedItem = ITEM_NONE,
    .lovedAction = BEFRIEND_ACTION_COUNT, // Nothing in particular
    .dislikedActions = 1 << BEFRIEND_ACTION_APPROACH,
    .patience = BEFRIEND_DEFAULT_PATIENCE,
    .watchHint = sText_BefriendHint_Default,
};
