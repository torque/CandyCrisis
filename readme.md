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
release. FMOD 3 doesn't appear to exist anywhere.

The code compiles (with some warnings that aren't currently worth
fixing) against `SDL 1.2.15` and `SDL_Image 1.2.12`.

However, during gameplay, and in the menu, there are severe performance
issues. Some inspection with Instruments indicates that most of the time
is being spent in `CGColorTransformConvertData`, which leads me to
believe the problem exists somewhere between SDL and Quartz. It seems
likely that the conversion routine from the 16-bit surfaces provided by
SDL to the (presumably) 32-bit Quartz surfaces causes the game speed to
become memory-limited. This may in fact be a bug in SDL 1.2.15, but
older versions require obnoxious amounts of patching to build on 10.9,
so I don't care to pursue this theory any further.

Unfortunately, after some tinkering, it seems that there is not an
obvious, simple way to prevent these conversion routines from lagging
the gameplay. The way the graphics are stored currently is not
particularly conducive to handing the blitting over to SDL and keeping
everything in a 32-bit pipeline.

It seems that if major effort is required to fix the slowdown problems,
the game may as well also be ported to SDL2.

While currently playable, it is a long way from being optimal.

[puyo]: http://en.wikipedia.org/wiki/Puyo_Puyo_(series)
[CCX]: https://github.com/philstopford/CCX
[MGSkittles]: http://macintoshgarden.org/games/skittles
[MGCandy Crisis]: http://macintoshgarden.org/games/candy-crisis
