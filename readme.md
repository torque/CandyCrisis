## Candy Crisis

Candy Crisis is a [Puyo Puyo][puyo] clone, written by John Stiles quite
a while ago (information is a bit sparse, but it predates OS X). It was
the successor, or perhaps evolution, of his earlier Puyo Puyo clone,
called Skittles. Based on some Internet sluthery, it seems that Candy
Crisis was originally called Skittles 2, but the name was changed for what
I imagine should be fairly obvious copyright reasons.

The website Macintosh Garden has a tiny amount of information about both
[Skittles][MGSkittles] and [Candy Crisis][MGCandy Crisis], as well as a
few screenshots of them.

### Downloads

If you're on OS X 10.9, grab a [release][releases]. The latest one is
always recommended.

### So what is this?

The last official build of Candy Crisis was made for PPC macs, and it
ran fine until Apple got rid of Rosetta in 10.7, which made it
impossible for modern OS X to run old PPC executables.

Fortunately, Stiles released the source code for Candy Crisis on
SourceForge. The goal of this repository is to clean up the codebase a
little, and get it running on modern operating systems.

Yes, I am aware of [CCX][CCX]. However, I am mostly doing this as an
exercise for myself, to work on my C(++) fluency. There are also some
decisions CCX made that I disagree with.

### Status (On OS X)

The audio backend was rewritten to use FMODex, based against the 4.44.41
release.

The code now uses designated initializers, a C99 feature that clang
(fortunately) allows in C++, even though it is not a C++ feature.

All of the legacy mac M types (`MBoolean`, `MPoint` and `MRect`) have
been stripped out of the code. They have been replaced with language
builtins (`bool`) or library structs (`SDL_Rect` and `SDL_Point`).

Depraved speedhacks have been implemented that make the game run less
smoothly, but at an actually playable rate. This sacrifice is an
acceptable holdover for the major changes that will (hopefully) provide
the real fixes.

The roadmap for now is: port to SDL2, improve blitting/modify resources,
and then start adding features/rewriting in real C++.

[puyo]: http://en.wikipedia.org/wiki/Puyo_Puyo_(series)
[MGSkittles]: http://macintoshgarden.org/games/skittles
[MGCandy Crisis]: http://macintoshgarden.org/games/candy-crisis
[releases]: https://github.com/torque/CandyCrisis/releases
[CCX]: https://github.com/philstopford/CCX
