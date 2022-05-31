// gameticks.c

#include <SDL2/SDL.h>

#include "gameticks.h"

unsigned long baseTickCount, freezeTickCount;
int freezeLevel;

unsigned long MTickCount() {
	// the math here scales ticks to match a 60Hz refresh rate (SDL ticks are
	// milliseconds, so the return value increments approximately once every
	// 16.66 ms)
	return (unsigned long) ((float)SDL_GetTicks64() * 0.06f);
}

void InitGameTickCount( void )
{
	baseTickCount = freezeTickCount = 0;
	freezeLevel = 0;
}

void FreezeGameTickCount( void )
{
	if( freezeLevel	== 0 )
    {
         freezeTickCount = MTickCount( );
    }
	freezeLevel--;
}

void UnfreezeGameTickCount( void )
{
	freezeLevel++;
	if( freezeLevel >= 0 )
	{
		freezeLevel = 0;
		baseTickCount += MTickCount( ) - freezeTickCount;
	}
}

unsigned long GameTickCount( void )
{
	if( freezeLevel < 0 )
		return freezeTickCount - baseTickCount;

	return MTickCount( ) - baseTickCount;
}
