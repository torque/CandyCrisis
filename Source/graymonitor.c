// graymonitor.c

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "graymonitor.h"

#include "graphics.h"
#include "grays.h"
#include "gworld.h"
#include "level.h"
#include "main.h"
#include "score.h"

static SDL_Surface* smallGrayDrawSurface;

SDL_Rect grayMonitorZRect, grayMonitorRect[2];
bool grayMonitorVisible[2] = { true, true };

void InitGrayMonitors( void )
{
	const double windowLoc[ ] = { 0.16, 0.84 };

	grayMonitorZRect.y = grayMonitorZRect.x = 0;
	grayMonitorZRect.h = 32;
	grayMonitorZRect.w = 144;

	grayMonitorRect[0] = grayMonitorRect[1] = grayMonitorZRect;
	CenterRectOnScreen( &grayMonitorRect[0], windowLoc[0], 0.11 );
	CenterRectOnScreen( &grayMonitorRect[1], windowLoc[1], 0.11 );

	smallGrayDrawSurface = SDLU_InitSurface( &grayMonitorZRect, 16 );
	DrawPICTInSurface( smallGrayDrawSurface, picBoard );
}

void ShowGrayMonitor( short player )
{
	short      monitor;
	SDL_Rect   myRect = { .x = 4, .y = 4, .h = kBlobVertSize, .w = kBlobHorizSize };
	SDL_Rect   srcRect;
	const int  smallGrayList[] = { 0, kSmallGray1, kSmallGray2, kSmallGray3, kSmallGray4, kSmallGray5 };

	if( !grayMonitorVisible[player] ) return;

	if( control[player] != kNobodyControl )
	{
		SDLU_AcquireSurface( smallGrayDrawSurface );

		SDLU_BlitSurface( boardSurface[player], &smallGrayDrawSurface->clip_rect,
		                  smallGrayDrawSurface, &smallGrayDrawSurface->clip_rect  );

		monitor = unallocatedGrays[player];

		CalcBlobRect( kSobBlob, 3, &srcRect );
		while( monitor >= (6*4) )
		{
			SurfaceDrawSprite( &myRect, 4, kSobBlob );
			myRect.x += kBlobHorizSize;

			monitor -= (6*4);
		}

		CalcBlobRect( kNoSuction, kGray-1, &srcRect );
		while( monitor >= 6 )
		{
			SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
			myRect.x += kBlobHorizSize;

			monitor -= 6;
		}

		if( monitor > 0 )
		{
			SurfaceDrawAlpha( &myRect, kGray, kLight, smallGrayList[monitor] );
			myRect.x += kBlobHorizSize;
			SurfaceDrawAlpha( &myRect, kGray, kLight, smallGrayList[monitor]+1 );
		}

		SDLU_ReleaseSurface( smallGrayDrawSurface );

		SDLU_BlitFrontSurface( smallGrayDrawSurface,
		                       &grayMonitorZRect,
		                       &grayMonitorRect[player] );
	}
}
