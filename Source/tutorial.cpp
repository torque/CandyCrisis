// tutorial.c

#include <string.h>

#include <SDL/SDL.h>
#include "SDLU.h"

#include "tutorial.h"

#include "blitter.h"
#include "control.h"
#include "font.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "opponent.h"
#include "pause.h"
#include "soundfx.h"

AutoPattern tutorialPattern[] =
{
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Welcome to the\nCandy Crisis\ntutorial!" },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "I'll be your guide\nthroughout the\ntutorial. Let's\nget started!" },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 240, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "When you start the\ngame, you'll find a\npair of candies\nfalling from the sky." },
	{ .command = kIdleTicks,      .d1 = 240, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Your goal is to\nkeep these candies\nfrom overflowing the\nboard!" },
	{ .command = kBlockUntilLand, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 2,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You control the\ncandy by moving\nthem left and right." },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Press '~~'\nto make the pieces\nmove left." },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 90,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Use '||' to\nmake the pieces\nmove right." },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You can also\nmake the pieces\nrotate around each\nother." },
	{ .command = kRetrieve,       .d1 = 4,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Press '{{'\nto make the pieces\nrotate." },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Also, '``'\ncauses the candy\nto drop faster." },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "To pause the game\nor adjust settings,\npress 'esc.'" },
	{ .command = kIdleTicks,      .d1 = 280, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "The candy in\nthis game is made\nfrom a highly\nunstable substance!" },
	{ .command = kRetrieve,       .d1 = 2,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 200, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "So when four\npieces of the same\ncolor come into\ncontact..." },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "... they vaporize!" },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Let's see that\nonce again." },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Pop!" },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You can even get\nfive or more\npieces to pop\nall at the same\ntime!" },
	{ .command = kRetrieve,       .d1 = 4,   .d2 = 4,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 4,   .d2 = 4,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 3,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 4,   .d2 = 4,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Pop!\n\nTechniques like this\ncan earn lots of\nbonus points." },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You can also pop\nmore than one color\nat once." },
	{ .command = kRetrieve,       .d1 = 5,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 6,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 3,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "All right!" },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 0,   .d2 = 6,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You can even set\nup devastating\nchain reactions..." },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 0,   .d2 = 6,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 90,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 6,   .d2 = 6,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "... like this!" },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Tricky, isn't it?" },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 3,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Let's see one more\nexample of a chain\nreaction." },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 5,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 5,   .d2 = 3,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 90,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "There's one more\nthing you need to\nknow about..." },
	{ .command = kIdleTicks,      .d1 = 180, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Watch out for the\nsee-through candy!" },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kPunish,         .d1 = 18,  .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "When your opponent\nvaporizes a group of\ncandies, they also\nsend some transparent\npieces to you!" },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 90,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 30,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 90,  .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "You can get rid of\nthese pieces by\nvaporizing something\nnext to them." },
	{ .command = kIdleTicks,      .d1 = 150, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 2,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "There are also\nsome bonus items\nwhich can come\nin handy." },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 1,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 20,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 15,  .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "One of these bonus\nitems is the Crazy\nCandy!" },
	{ .command = kRetrieve,       .d1 = -1,  .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 15,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 10,  .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 100, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "The Crazy Candy\ntakes on the\ncolor of one of\nits neighbors." },
	{ .command = kIdleTicks,      .d1 = 60,  .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 4,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 1,   .d2 = 5,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 1,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Another useful bonus\nitem is the bomb!" },
	{ .command = kRetrieve,       .d1 = 2,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 0,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 3,   .d2 = 2,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 4,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 3,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve,       .d1 = 2,   .d2 = 3,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kSpin,           .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 5,   .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kRetrieve, .d1 = kBombBottom, .d2 = kBombTop, .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "When the bomb lands\non one color\nof candy, all\npieces of that\ncolor will\nvaporize!" },
	{ .command = kIdleTicks,      .d1 = 100, .d2 = 0,   .message = NULL },
	{ .command = kPosition,       .d1 = 4,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 120, .d2 = 0,   .message = NULL },
	{ .command = kBlockUntilDrop, .d1 = 0,   .d2 = 0,   .message = NULL },
	{ .command = kIdleTicks,      .d1 = 200, .d2 = 0,   .message = NULL },
	{ .command = kMessage,        .d1 = 0,   .d2 = 0,   .message = "Now you're ready\nto play Candy Crisis.\nGood luck!" },
	{ .command = kIdleTicks,      .d1 = 270, .d2 = 0,   .message = NULL },

	{ .command = kComplete,       .d1 = 0,   .d2 = 0,   .message = NULL }
};

SDL_Rect        balloonRect = { .y = 0, .x = 0, .h = 190, .w = 210 };
SkittlesFontPtr balloonFont;
SDLU_Point      balloonPt;
char*           balloonChar;
char            balloonMsg[256];
int             balloonTime, tutorialTime;
SDL_Surface*    balloonSurface = NULL;

void InitTutorial( void )
{
	// Balloon font
	balloonFont = GetFont( picBalloonFont );

	// Balloon backbuffer
	if( balloonSurface == NULL )
	{
		SDL_Rect surfaceRect = { 0, 0, backdropSurface->w, backdropSurface->h };
		balloonSurface = SDLU_InitSurface( &surfaceRect, 16 );
	}

	// Set up auto pattern
	autoPattern = tutorialPattern;
	tutorialTime = 0;
}

void EndTutorial( void )
{
	QuickFadeOut();

	showStartMenu = true;
}

static int CalculateBalloonWidth( char *message )
{
	int maxWidth = 40;
	int currentWidth = 0;

	for( ;; )
	{
		char in = *message++;

		switch(in)
		{
			case 0:
				return (currentWidth > maxWidth)? currentWidth: maxWidth;

			case '\n':
				maxWidth = (currentWidth > maxWidth)? currentWidth: maxWidth;
				currentWidth = 0;
				break;

			default:
				currentWidth += balloonFont->width[(int)in];
				break;
		}
	}
}

static int CalculateBalloonHeight( char *message )
{
	int lines = 2;
	char *scan = message;

	while( *scan ) lines += (*scan++ == '\n');

	return lines * 20;
}

void StopBalloon( void )
{
	balloonTime = 0x7FFFFFFF;
}

void StartBalloon( const char *message )
{
	SDLU_Point  balloonTip, balloonFill;
	int         replace;
	const char *match[] = { "~~", "||", "``", "{{" };
	char       *search;
	SDL_Rect    balloonSDLRect, balloonContentsSDLRect, balloonContentsRect;

	strcpy( balloonMsg, message );
	for( replace=0; replace<4; replace++ )
	{
		search = strstr( balloonMsg, match[replace] );
		if( search )
		{
			char temp[256];

			search[0] = '%';
			search[1] = 's';
			sprintf( temp, balloonMsg, SDL_GetKeyName( playerKeys[1][replace] ) );
			strcpy( balloonMsg, temp );
		}
	}

	// Erase previous balloons
	SDLU_BlitFrontSurface( backdropSurface, &balloonRect, &balloonRect );

	// Draw empty balloon outline
	SDLU_AcquireSurface( balloonSurface );

	balloonRect.w = CalculateBalloonWidth ( balloonMsg );
	balloonRect.h = CalculateBalloonHeight( balloonMsg );
	balloonRect.x += balloonRect.w - 25;
	balloonRect.y += balloonRect.h - 25;

	SDLU_BlitSurface( backdropSurface, &balloonRect,
	                  balloonSurface,  &balloonRect  );

	balloonContentsRect = balloonRect;
	balloonContentsRect.h -= 25;

	SurfaceGetEdges( balloonSurface, &balloonContentsRect );
	SDL_FillRect( balloonSurface, &balloonContentsRect,
	              SDL_MapRGB( balloonSurface->format, 0xFF, 0xFF, 0xFF ) );
	SurfaceCurveEdges( balloonSurface, &balloonContentsRect );

	balloonTip.y = balloonContentsRect.y + balloonContentsRect.h - 2;
	balloonTip.x = balloonContentsRect.x + balloonContentsRect.w - 40;
	balloonFill = balloonTip;

	SurfaceBlitCharacter( balloonFont, '[', &balloonFill,  0,  0,  0,  0 );
	SurfaceBlitCharacter( balloonFont, ']', &balloonTip,  31, 31, 31,  0 );

	SDLU_ReleaseSurface( balloonSurface );

	// Blit empty balloon to screen
	SDLU_BlitFrontSurface( balloonSurface, &balloonRect, &balloonRect );

	balloonPt.x = balloonRect.x + 10;
	balloonPt.y = balloonRect.y + 10;
	balloonChar = balloonMsg;
	balloonTime = GameTickCount( );

	OpponentChatter( true );
}

void UpdateBalloon( void )
{
	SDL_Rect balloonSDLRect;

	if( control[0] != kAutoControl ) return;
	if( GameTickCount() < balloonTime ) return;

	if( balloonChar )
	{
		char in = *balloonChar++;

		switch( in )
		{
			case 0:
				OpponentChatter( false );
				balloonChar = NULL;
				balloonTime += 120;
				break;

			case '\n':
				balloonPt.x = balloonRect.x + 10;
				balloonPt.y += 20;
				break;

			default:
				if( balloonFont->width[(int)in] > 0 )
				{
					SDLU_AcquireSurface( balloonSurface );
					SurfaceBlitCharacter( balloonFont, in, &balloonPt, 0, 0, 0, 0 );
					SDLU_ReleaseSurface( balloonSurface );

					SDLU_BlitFrontSurface( balloonSurface, &balloonRect, &balloonRect );

					balloonTime += 2;
				}
				break;
		}
	}
	else
	{
		SDLU_BlitFrontSurface( backdropSurface, &balloonRect, &balloonRect );

		StopBalloon();
	}
}
