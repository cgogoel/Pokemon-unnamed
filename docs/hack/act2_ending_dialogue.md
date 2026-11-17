# The end of Act 2: dialogue for editing

Everything after the chamber, in the order it plays. The parents' speech in the chamber is in
[parents_ending_dialogue.md](parents_ending_dialogue.md). Edit the lines freely; I'll put them back into the script.
Each `---` is a new text box (a button press). A box shows two lines; a third or fourth line scrolls up in the same
box. A line fits about 208 pixels, roughly 32-38 characters (I'll measure and rewrap if needed). `{PLAYER}` is the
player's name. Italics are stage directions, not text.

Source: `data/scripts/act2_finale.inc` (labels in brackets), and `src/act2_end.c` for the end card.

## Home, right after the chamber

The player wakes up in their room (2F). Mom calls up the stairs, then KAI, MINA and ROWAN come up one by one and
line up beside the player.

`Act2_Text_MomFriendsHere`

MOM: {PLAYER}! Your friends are
here! ...
You all can go on up.

`Act2_Text_FriendsVisit`

KAI: {PLAYER}! We came all
the way out here to find you.

---

MINA: Did you finish your exam?
My WAVE CRASH is done!

---

ROWAN: You look like you saw
a ghost. What happened?

---

{PLAYER}: …

ROWAN: So you saw three legendary pokemon??

MINA: And they attacked you?!

KAI: AWESOME!

ROWAN: KAI...
So a FIRE, WATER, and GRASS attack
all at the same time huh... wow

---

*(The player gets an idea: "!" over their head.)*

`Act2_Text_Epiphany`

{PLAYER}: …!

---

KAI: You want us to go all the
way back to the lab? Now??

---

ROWAN: Huh? Wait up!

*(Fade to black; warp to the lab, the institute's Research Room.)*

## If you talk to a friend or professor

A fallback for talking to the friends (in your room or the lab) or the professors. Both scenes hold the player
still until they end, so in practice it hardly ever shows.

`Act2_Text_HomeFriend`

Whoa, {PLAYER}!

## The lab: Tri-Blast

The player is at the center terminal with the friends behind them.

`Act2_Text_LabWorking`

{PLAYER} worked through the night…

*(PC sounds.)*

`Act2_Text_LabDone`

{PLAYER}: Done!

*(Fanfare.)*

`Act2_Text_MadeTriBlast`

{PLAYER} made TM TRI-BLAST!

`Act2_Text_FriendsReactTriBlast`

KAI: TRI-BLAST?

---

ROWAN: FIRE, WATER and GRASS
in one attack. 

---

MINA: All three at once… woven
together...

---

ROWAN: …the target takes super-effective
damage without normal type resistance

---

KAI: No way. That's amazing!

*(PROF. ASHFORD, PROF. MARLOWE and PROF. FERN come up the stairs and stand behind the friends.)*

`Act2_Text_ProfessorsMarvel`

PROF. ASHFORD: What's all the
noise in here? Is that…

---

PROF. MARLOWE: A new TM???
FIRE, WATER and
GRASS in a single move.
In all my years…

---

PROF. FERN: All three courses in
one TM. You really listened,
and then some.

---

PROF. ASHFORD: {PLAYER}, this
isn't just a passing grade.
It's a discovery.

---

Congratulations, graduate! Go celebrate
with your friends. Then cancel your plans 
for the next week. You've got
some writing to do, we're getting this
published!

*(Fade to black.)*

## The end card

On black, then Act 3 starts at Silph Co.:

Several years later

## Leftovers

What Mom and Dad say if the player talks to them at home between the chamber and the end of Act 2. In normal play
the ending runs straight from the chamber through the lab to Act 3, so this never shows. Cut it or keep it as you
like.

`Act2_Text_ParentsAfterChamber`

MOM: No more sneaking into old
ranger stations, OK?

---

DAD: Heh. It runs in the family.

The ranger station's door from then on (it also shows in Act 3):

`Act2_Text_StationShut`

The old ranger station.
The door won't budge.
