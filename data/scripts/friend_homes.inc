@ The friends' families (docs/hack/act2_story.md, "The friends' families"). Each friend grew up in the town of the
@ type they love, and their parents are always home, in every act:
@ - KAI (Fire), CELADON CITY: his two dads (CeladonCity_Condominiums_2F_Frlg; he fell for FIRE on a school trip)
@ - MINA (Water), VERMILION CITY, the Water course's harbor: her mom (VermilionCity_House3_Frlg)
@ - ROWAN (Grass), CELADON CITY: their mom and dad (CeladonCity_House1_Frlg)
@ What the parents say follows the story (FriendHomes_EventScript_GetStage).

.set FRIEND_HOMES_CHILDHOOD, 0 @ Act 1: the friends are kids at home
.set FRIEND_HOMES_COURSES,   1 @ Act 2, until the Trainer Card: at CELADON UNIVERSITY, the three courses
.set FRIEND_HOMES_PROJECT,   2 @ Act 2, from the Trainer Card: the final project (the friends' 120-power TMs)
.set FRIEND_HOMES_LATER,     3 @ Act 3, several years later

@ Sets VAR_RESULT to the FRIEND_HOMES_* stage
FriendHomes_EventScript_GetStage::
	setvar VAR_RESULT, FRIEND_HOMES_CHILDHOOD
	goto_if_eq VAR_STORY_ACT, STORY_ACT_CHILDHOOD, FriendHomes_EventScript_Return
	setvar VAR_RESULT, FRIEND_HOMES_LATER
	goto_if_ge VAR_STORY_ACT, STORY_ACT_WORKFORCE, FriendHomes_EventScript_Return
	setvar VAR_RESULT, FRIEND_HOMES_COURSES
	goto_if_lt VAR_ACT2_PROGRESS, ACT2_GOT_TRAINER_CARD, FriendHomes_EventScript_Return
	setvar VAR_RESULT, FRIEND_HOMES_PROJECT
FriendHomes_EventScript_Return::
	return

@ ---------------------------------------------------------------------------------------------------------------
@ KAI's dads, CELADON CITY: a suite in the CONDOMINIUMS. Neighbors (across town) of ROWAN's parents.

FriendHomes_EventScript_KaiDad::
	lock
	faceplayer
	call FriendHomes_EventScript_GetStage
	goto_if_eq VAR_RESULT, FRIEND_HOMES_CHILDHOOD, FriendHomes_EventScript_KaiDadChildhood
	goto_if_eq VAR_RESULT, FRIEND_HOMES_PROJECT, FriendHomes_EventScript_KaiDadProject
	goto_if_eq VAR_RESULT, FRIEND_HOMES_LATER, FriendHomes_EventScript_KaiDadLater
	msgbox FriendHomes_Text_KaiDadCourses
	release
	end

FriendHomes_EventScript_KaiDadChildhood::
	msgbox FriendHomes_Text_KaiDadChildhood
	release
	end

FriendHomes_EventScript_KaiDadProject::
	call_if_unset FLAG_ACT2_KAI_DADS_SNACKS, FriendHomes_EventScript_KaiDadsSnacks
	msgbox FriendHomes_Text_KaiDadProject
	release
	end

FriendHomes_EventScript_KaiDadLater::
	msgbox FriendHomes_Text_KaiDadLater
	release
	end

FriendHomes_EventScript_KaiPapa::
	lock
	faceplayer
	call FriendHomes_EventScript_GetStage
	goto_if_eq VAR_RESULT, FRIEND_HOMES_CHILDHOOD, FriendHomes_EventScript_KaiPapaChildhood
	goto_if_eq VAR_RESULT, FRIEND_HOMES_PROJECT, FriendHomes_EventScript_KaiPapaProject
	goto_if_eq VAR_RESULT, FRIEND_HOMES_LATER, FriendHomes_EventScript_KaiPapaLater
	msgbox FriendHomes_Text_KaiPapaCourses
	release
	end

FriendHomes_EventScript_KaiPapaChildhood::
	msgbox FriendHomes_Text_KaiPapaChildhood
	release
	end

FriendHomes_EventScript_KaiPapaProject::
	call_if_unset FLAG_ACT2_KAI_DADS_SNACKS, FriendHomes_EventScript_KaiDadsSnacks
	msgbox FriendHomes_Text_KaiPapaProject
	release
	end

FriendHomes_EventScript_KaiPapaLater::
	msgbox FriendHomes_Text_KaiPapaLater
	release
	end

@ The final project: KAI told the player to stop by for snacks. Once (FLAG_ACT2_KAI_DADS_SNACKS), whichever dad
@ the player talks to first: the loud one gives MOOMOO MILK, the quiet one POKé WRAPS (ITEM_POKE_WRAP).
FriendHomes_EventScript_KaiDadsSnacks:
	msgbox FriendHomes_Text_KaiDadSnacks
	giveitem ITEM_MOOMOO_MILK, 5
	msgbox FriendHomes_Text_KaiPapaSnacks
	giveitem ITEM_POKE_WRAP, 5
	setflag FLAG_ACT2_KAI_DADS_SNACKS
	return

FriendHomes_EventScript_KaiPhoto::
	msgbox FriendHomes_Text_KaiPhoto, MSGBOX_SIGN
	end

FriendHomes_EventScript_KaiNameplate::
	msgbox FriendHomes_Text_KaiNameplate, MSGBOX_SIGN
	end

@ ---------------------------------------------------------------------------------------------------------------
@ MINA's mom, VERMILION CITY. On the Water trip she follows the docks' fire (Act2_GetDockFireState).

FriendHomes_EventScript_MinaMom::
	lock
	faceplayer
	call FriendHomes_EventScript_GetStage
	goto_if_eq VAR_RESULT, FRIEND_HOMES_CHILDHOOD, FriendHomes_EventScript_MinaMomChildhood
	goto_if_eq VAR_RESULT, FRIEND_HOMES_PROJECT, FriendHomes_EventScript_MinaMomProject
	goto_if_eq VAR_RESULT, FRIEND_HOMES_LATER, FriendHomes_EventScript_MinaMomLater
	goto_if_eq VAR_ACT2_PROGRESS, ACT2_WATER_TRIP, FriendHomes_EventScript_MinaMomTrip
	msgbox FriendHomes_Text_MinaMomCourses
	release
	end

FriendHomes_EventScript_MinaMomTrip::
	specialvar VAR_RESULT, Act2_GetDockFireState
	goto_if_eq VAR_RESULT, DOCK_FIRE_BURNING, FriendHomes_EventScript_MinaMomTripFire
	goto_if_eq VAR_RESULT, DOCK_FIRE_OUT, FriendHomes_EventScript_MinaMomTripSaved
	msgbox FriendHomes_Text_MinaMomTrip
	release
	end

FriendHomes_EventScript_MinaMomTripFire::
	msgbox FriendHomes_Text_MinaMomTripFire
	release
	end

FriendHomes_EventScript_MinaMomTripSaved::
	msgbox FriendHomes_Text_MinaMomTripSaved
	release
	end

FriendHomes_EventScript_MinaMomChildhood::
	msgbox FriendHomes_Text_MinaMomChildhood
	release
	end

FriendHomes_EventScript_MinaMomProject::
	msgbox FriendHomes_Text_MinaMomProject
	release
	end

FriendHomes_EventScript_MinaMomLater::
	msgbox FriendHomes_Text_MinaMomLater
	release
	end

FriendHomes_EventScript_MinaPhoto::
	msgbox FriendHomes_Text_MinaPhoto, MSGBOX_SIGN
	end

@ ---------------------------------------------------------------------------------------------------------------
@ ROWAN's mom and dad, CELADON CITY (the university is across town; ROWAN lives in the dorms anyway)

FriendHomes_EventScript_RowanMom::
	lock
	faceplayer
	call FriendHomes_EventScript_GetStage
	goto_if_eq VAR_RESULT, FRIEND_HOMES_CHILDHOOD, FriendHomes_EventScript_RowanMomChildhood
	goto_if_eq VAR_RESULT, FRIEND_HOMES_PROJECT, FriendHomes_EventScript_RowanMomProject
	goto_if_eq VAR_RESULT, FRIEND_HOMES_LATER, FriendHomes_EventScript_RowanMomLater
	msgbox FriendHomes_Text_RowanMomCourses
	release
	end

FriendHomes_EventScript_RowanMomChildhood::
	msgbox FriendHomes_Text_RowanMomChildhood
	release
	end

FriendHomes_EventScript_RowanMomProject::
	msgbox FriendHomes_Text_RowanMomProject
	release
	end

FriendHomes_EventScript_RowanMomLater::
	msgbox FriendHomes_Text_RowanMomLater
	release
	end

FriendHomes_EventScript_RowanDad::
	lock
	faceplayer
	call FriendHomes_EventScript_GetStage
	goto_if_eq VAR_RESULT, FRIEND_HOMES_CHILDHOOD, FriendHomes_EventScript_RowanDadChildhood
	goto_if_eq VAR_RESULT, FRIEND_HOMES_PROJECT, FriendHomes_EventScript_RowanDadProject
	goto_if_eq VAR_RESULT, FRIEND_HOMES_LATER, FriendHomes_EventScript_RowanDadLater
	msgbox FriendHomes_Text_RowanDadCourses
	release
	end

FriendHomes_EventScript_RowanDadChildhood::
	msgbox FriendHomes_Text_RowanDadChildhood
	release
	end

FriendHomes_EventScript_RowanDadProject::
	msgbox FriendHomes_Text_RowanDadProject
	release
	end

FriendHomes_EventScript_RowanDadLater::
	msgbox FriendHomes_Text_RowanDadLater
	release
	end

FriendHomes_EventScript_RowanPhoto::
	msgbox FriendHomes_Text_RowanPhoto, MSGBOX_SIGN
	end

FriendHomes_EventScript_RowanPlant::
	msgbox FriendHomes_Text_RowanPlant, MSGBOX_SIGN
	end

@ ---------------------------------------------------------------------------------------------------------------
@ KAI's dad: big and loud. KAI gets it from him.

FriendHomes_Text_KaiDadChildhood:
	.string "KAI? Up on the roof, trying to\n"
	.string "light the barbecue again.\p"
	.string "He's been crazy about FIRE\n"
	.string "POKéMON since his school trip to\l"
	.string "CINNABAR's volcano. Ha!$"

FriendHomes_Text_KaiDadCourses:
	.string "Our boy KAI's at CELADON\n"
	.string "UNIVERSITY. A real student!\p"
	.string "It's right across town, and he\n"
	.string "still moved into the dorms. Ha!\p"
	.string "You must be {PLAYER}! He talks\n"
	.string "about you every time he's home.$"

FriendHomes_Text_KaiDadProject:
	.string "KAI called! He's making a move\n"
	.string "called FLARE BLITZ.\p"
	.string "All power, all in. Where do you\n"
	.string "think he gets that from? Ha!\p"
	.string "…Between you and me, I hope he\n"
	.string "remembers to duck.$"

FriendHomes_Text_KaiDadSnacks:
	.string "KAI's DAD: {PLAYER}! KAI said\n"
	.string "you'd stop by. Ha!\p"
	.string "You can't cross KANTO on an empty\n"
	.string "stomach. Here, MOOMOO MILK for\l"
	.string "the road!$"

FriendHomes_Text_KaiPapaSnacks:
	.string "KAI's PAPA: And these POKé WRAPS.\n"
	.string "Homemade. KAI's favorite.\p"
	.string "Don't laugh. They heal more than\n"
	.string "you'd think.$"

FriendHomes_Text_KaiDadLater:
	.string "{PLAYER}! Look at you, all\n"
	.string "grown up. SILPH CO., I hear!\p"
	.string "KAI's still out training FIRE\n"
	.string "POKéMON. Says he'll take on the\l"
	.string "GYM on CINNABAR one day.\p"
	.string "I've stopped telling him he's too\n"
	.string "reckless. Mostly.$"

@ KAI's papa: quiet, dry, the worrier

FriendHomes_Text_KaiPapaChildhood:
	.string "I'm KAI's papa. If you're looking\n"
	.string "for him, follow the smoke.$"

FriendHomes_Text_KaiPapaCourses:
	.string "KAI's studying at CELADON\n"
	.string "UNIVERSITY. FIRE, WATER, GRASS…\p"
	.string "He comes home for dinner on\n"
	.string "Sundays. Mostly for the dinner.\p"
	.string "ROWAN's parents live across town.\n"
	.string "Calm people. We could use some\l"
	.string "of that around here.$"

FriendHomes_Text_KaiPapaProject:
	.string "A TM with 120 power, all on his\n"
	.string "own. FLARE BLITZ hurts the user\l"
	.string "too, you know.\p"
	.string "I've read up on it. Twice.\p"
	.string "Look out for him, {PLAYER}.\n"
	.string "He listens to you. Sometimes.$"

FriendHomes_Text_KaiPapaLater:
	.string "KAI writes longer letters now.\n"
	.string "Three lines, sometimes four!\p"
	.string "He trains FIRE POKéMON full time.\n"
	.string "He's even learning patience.\p"
	.string "Thank you for being his friend,\n"
	.string "{PLAYER}. It shows.$"

FriendHomes_Text_KaiPhoto:
	.string "A photo of a small KAI between\n"
	.string "his two dads, beaming, a VULPIX\l"
	.string "plush held up high.$"

FriendHomes_Text_KaiNameplate:
	.string "KAI's family$"

@ MINA's mom: warm and practical, like MINA. It's been the two of them.

FriendHomes_Text_MinaMomChildhood:
	.string "MINA's down on the docks again,\n"
	.string "feeding the TENTACOOL. Again!\p"
	.string "The sailors all know her by name.\n"
	.string "She'd live on a boat if she could.$"

FriendHomes_Text_MinaMomCourses:
	.string "MINA's at CELADON UNIVERSITY,\n"
	.string "and she loves it.\p"
	.string "It's been just the two of us since\n"
	.string "she was little. The house is so\l"
	.string "quiet now. No more sea spray on\l"
	.string "the floor.\p"
	.string "She calls every Sunday. Never\n"
	.string "misses one. That's my MINA.$"

FriendHomes_Text_MinaMomTrip:
	.string "MINA's class is here, at the\n"
	.string "harbor? She didn't tell me!\p"
	.string "Oh! You must be {PLAYER}!\n"
	.string "MINA's told me all about you.\p"
	.string "She knows every fishing spot\n"
	.string "in VERMILION. Ask her!$"

FriendHomes_Text_MinaMomTripFire:
	.string "The docks are on fire! I can see\n"
	.string "the smoke from the window!\p"
	.string "MINA ran straight down there,\n"
	.string "didn't she? Of course she did.\p"
	.string "Please, {PLAYER}. Help them!$"

FriendHomes_Text_MinaMomTripSaved:
	.string "Word travels fast on the docks.\p"
	.string "Your class put out the fire, and\n"
	.string "saved a young LAPRAS, I hear!\p"
	.string "My MINA's class… Oh, I'm so proud\n"
	.string "I could burst!$"

FriendHomes_Text_MinaMomProject:
	.string "{PLAYER}! Come in, come in!\n"
	.string "MINA called about you.\p"
	.string "She's making WAVE CRASH for her\n"
	.string "final project. A whole wave!\p"
	.string "I worry, of course. But she's\n"
	.string "tougher than I ever was.\p"
	.string "Did you eat? Take a snack for\n"
	.string "the road, at least.$"

FriendHomes_Text_MinaMomLater:
	.string "MINA works at a marine research\n"
	.string "station now, way out at sea.\p"
	.string "She sends postcards with a STARYU\n"
	.string "drawn in the corner. They're all\l"
	.string "on the fridge.\p"
	.string "{PLAYER}, if you're ever out\n"
	.string "that way, visit her for me?$"

FriendHomes_Text_MinaPhoto:
	.string "A photo of a little girl on the\n"
	.string "pier, a STARYU in her arms.\p"
	.string "It's MINA!$"

@ ROWAN's mom: a gardener, cheerful

FriendHomes_Text_RowanMomChildhood:
	.string "ROWAN's out in the garden, reading\n"
	.string "to the ODDISH again.\p"
	.string "I've stopped asking why.$"

FriendHomes_Text_RowanMomCourses:
	.string "ROWAN's at CELADON UNIVERSITY,\n"
	.string "right across town.\p"
	.string "And still lives in the dorms!\n"
	.string "ROWAN says it's about being\l"
	.string "independent.\p"
	.string "The laundry ROWAN brings home\n"
	.string "says otherwise.$"

FriendHomes_Text_RowanMomProject:
	.string "You're {PLAYER}, aren't you?\n"
	.string "ROWAN says you notice things\l"
	.string "other people miss. High praise!\p"
	.string "SOLAR BEAM for the final project!\n"
	.string "ROWAN came by for my sunlamp.$"

FriendHomes_Text_RowanMomLater:
	.string "ROWAN works at a botanical lab\n"
	.string "now. All glass and sunlight.\p"
	.string "There are plants there nobody's\n"
	.string "named yet. ROWAN's naming them.\p"
	.string "Slowly. Very carefully.$"

@ ROWAN's dad: bookish and quiet; ROWAN gets it from him

FriendHomes_Text_RowanDadChildhood:
	.string "ROWAN asked me why leaves are\n"
	.string "green. I said I'd look it up.\p"
	.string "That was a week ago. ROWAN has\n"
	.string "read three books on it since.$"

FriendHomes_Text_RowanDadCourses:
	.string "ROWAN takes the FIRE, WATER and\n"
	.string "GRASS courses this year.\p"
	.string "GRASS is the favorite, of course.\n"
	.string "It always has been.\p"
	.string "KAI's dads live across town, in\n"
	.string "the CONDOMINIUMS. One of them is\l"
	.string "very… loud.$"

FriendHomes_Text_RowanDadProject:
	.string "SOLAR BEAM takes a whole turn to\n"
	.string "charge. ROWAN told me.\p"
	.string "In detail. Over dinner.\p"
	.string "…I'm proud. I just hope ROWAN\n"
	.string "remembers to sleep, too.$"

FriendHomes_Text_RowanDadLater:
	.string "{PLAYER}, isn't it? ROWAN's\n"
	.string "friend from university!\p"
	.string "ROWAN runs a greenhouse lab now.\n"
	.string "We get a new plant every visit.\p"
	.string "The house is turning into a\n"
	.string "jungle. I've never been happier.$"

FriendHomes_Text_RowanPhoto:
	.string "A photo of a little ROWAN, asleep\n"
	.string "under a tree, a book still open.$"

FriendHomes_Text_RowanPlant:
	.string "A potted plant. A label in neat\n"
	.string "handwriting: “Water on Sundays.\l"
	.string "Not before. -ROWAN”$"
