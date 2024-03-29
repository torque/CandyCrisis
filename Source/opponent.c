// opponent.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "opponent.h"

#include "blitter.h"
#include "control.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "level.h"
#include "main.h"
#include "players.h"
#include "random.h"

SDL_Surface* opponentSurface;
SDL_Surface* opponentMaskSurface;
SDL_Surface* opponentDrawSurface;

SDL_Rect opponentWindowZRect, opponentWindowRect;
int opponentMood, opponentFrame;
int opponentTime, glowTime[kGlows], glowFrame[kGlows], panicTime, panicFrame;
int heavyGlowArray[kGlowArraySize], glowArray[kGlowArraySize], lightGlowArray[kGlowArraySize];

void InitOpponent( void )
{
	SDL_Rect littleRect = { .y = 0, .x = 0, .h = 64, .w = 64};
	SDL_Rect bigRect    = { .y = 0, .x = 0, .h = 64, .w = 64*(kOppFrames*3) };
	double   index, value;

	opponentDrawSurface = SDLU_InitSurface( &littleRect, 16 );
	opponentSurface     = SDLU_InitSurface( &bigRect, 16 );

	bigRect.h *= kGlows + 1;
	opponentMaskSurface = SDLU_InitSurface( &bigRect, 1 );

	opponentWindowZRect.y = opponentWindowZRect.x = 0;
	opponentWindowZRect.h = opponentWindowZRect.w = 64;
	opponentWindowRect = opponentWindowZRect;
	CenterRectOnScreen( &opponentWindowRect, 0.5, 0.5 );

	opponentMood = 0;

	for( index=0; index<kGlowArraySize; index++ )
	{
		value = sin( index*pi/kGlowArraySize );
		value *= value;

		heavyGlowArray[(int)index] = (int)(value * 24.0);
		glowArray     [(int)index] = (int)(value * 16.0);
		lightGlowArray[(int)index] = (int)(value * 12.0);
	}
}

void BeginOpponent( int which )
{
	int count;

	DrawPICTInSurface( opponentSurface,     5000 + which );
	DrawPICTInSurface( opponentMaskSurface, 5100 + which );

	opponentTime = panicTime = GameTickCount( );
	for( count=0; count<kGlows; count++ )
	{
		glowTime[count] = panicTime;
		glowFrame[count] = 0;
	}

	opponentMood = 0;
	emotions[0] = emotions[1] = kEmotionNeutral;
}

void DrawFrozenOpponent( void )
{
	SDL_Rect   myRect = { .y = 0, .x = 0, .h = 64, .w = 64 };

	SDLU_OffsetRect( &myRect, opponentFrame * 64, 0 );

	SDLU_BlitFrontSurface( opponentSurface, &myRect, &opponentWindowRect );
}

void OpponentPissed( void )
{
	opponentMood = 7;
	opponentTime = GameTickCount();
}

void OpponentChatter( bool on )
{
	opponentMood = on? 5: 0;
	opponentTime = GameTickCount();
}

void UpdateOpponent( void )
{
	SDL_Rect myRect  = { .x = 0, .y = 0, .w = 64, .h = 64};
	SDL_Rect dstRect = { .x = 0, .y = 0, .w = 64, .h = 64};
	SDL_Rect maskRect;
	int      emotiMap[] = {0, 1, 2, 1}, count;
	bool     draw = false;

	if( GameTickCount( ) > opponentTime )
	{
		switch( opponentMood )
		{
			case 0: 				// Idle
				opponentTime += 60 + RandomBefore(180);
				opponentMood = RandomBefore(2) + 1;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames);
				break;

			case 1:					// Shifty Eyes
				opponentTime += 40 + RandomBefore(60);
				opponentMood = 0;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + RandomBefore(2) + 1;
				break;

			case 2: // Blinks
				opponentTime += 3;
				opponentMood = 3;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 3;
				break;

			case 3: // Blinks (more)
				opponentTime += 3;
				opponentMood = 4;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 4;
				break;

			case 4: // Blinks (more)
				opponentTime += 3;
				opponentMood = 0;
				opponentFrame = (emotiMap[emotions[1]] * kOppFrames) + 3;
				break;

			case 5: // Chatter (only good for tutorial)
				opponentTime += 8;
				opponentMood = 6;
				opponentFrame = 5;
				break;

			case 6: // Chatter 2 (only good for tutorial)
				opponentTime += 8;
				opponentMood = 5;
				opponentFrame = 6;
				break;

			case 7: // Pissed (when hit with punishments)
				opponentTime += 60;
				opponentFrame = 7;
				opponentMood = 0;
				break;
		}

		draw = true;
	}

	if( GameTickCount( ) > panicTime )
	{
		panicTime += 2;

		if( emotions[1] == kEmotionPanic )
		{
			if( ++panicFrame >= kGlowArraySize ) panicFrame = 0;
			draw = true;
		}
		else
		{
			panicFrame = 0;
		}
	}

	for( count=0; count<kGlows; count++ )
	{
		if( GameTickCount( ) > glowTime[count] )
		{
			glowTime[count] += character[1].glow[count].time;

			if( character[1].glow[count].color )
			{
				if( ++glowFrame[count] >= kGlowArraySize ) glowFrame[count] = 0;
				draw = true;
			}
			else
			{
				glowFrame[count] = 0;
			}
		}
	}

	if( draw )
	{
		SDLU_OffsetRect( &myRect, 64*opponentFrame, 0 );

		// INVESTIGATE: SDLU_GetCurrentSurface() doesn't appear to be used
		// at all before this is released. Is it even used?
		// SDLU_AcquireSurface( opponentDrawSurface );

		SDLU_BlitSurface( opponentSurface,     &myRect,
		                  opponentDrawSurface, &dstRect );

		maskRect = myRect;
		for( count=0; count<kGlows; count++ )
		{
			// maskRect is for blitting the glowing parts.
			SDLU_OffsetRect( &maskRect, 0, 64 );

			if( glowFrame[count] )
			{
				if( character[1].glow[count].color & 0x8000 )
				{
					SurfaceBlitColor(  opponentMaskSurface,  opponentDrawSurface,
					                  &maskRect,            &dstRect,
					                  (character[1].glow[count].color & 0x7C00) >> 10,
					                  (character[1].glow[count].color & 0x03E0) >> 5,
					                  (character[1].glow[count].color & 0x001F),
					                   heavyGlowArray[glowFrame[count]] );
				}
				else
				{
					SurfaceBlitColor(  opponentMaskSurface,  opponentDrawSurface,
					                  &maskRect,            &dstRect,
					                  (character[1].glow[count].color & 0x7C00) >> 10,
					                  (character[1].glow[count].color & 0x03E0) >> 5,
					                  (character[1].glow[count].color & 0x001F),
					                   lightGlowArray[glowFrame[count]] );
				}
			}
		}

		if( panicFrame )
		{
			SurfaceBlitColor(  opponentMaskSurface,  opponentDrawSurface,
			                  &myRect,              &dstRect,
			                   31, 31, 22, glowArray[panicFrame] );
		}

		// SDLU_ReleaseSurface( opponentDrawSurface );

		SDLU_BlitFrontSurface( opponentDrawSurface, &opponentWindowZRect, &opponentWindowRect );
	}
}
