// next.c

#include <SDL/SDL.h>
#include "SDLU.h"

#include "next.h"

#include "blitter.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "level.h"
#include "main.h"
#include "random.h"

#define kJiggleFrames 8
#define kPulling 10
#define kPullingFrames 18

SDL_Surface* nextSurface;
SDL_Surface* nextDrawSurface;

SDL_Rect nextWindowZRect, nextWindowRect[2];
bool nextWindowVisible[2] = {true, true};
int nextTime[2][2], nextStage[2][2], pullA[2], pullB[2];

void InitNext( void )
{
	const double windowLoc[] = {0.46, 0.54};

	nextWindowZRect.y = nextWindowZRect.x = 0;
	nextWindowZRect.h = 72;
	nextWindowZRect.w = 32;

	nextWindowRect[0] = nextWindowRect[1] = nextWindowZRect;
	CenterRectOnScreen( &nextWindowRect[0], windowLoc[0], 0.25 );
	CenterRectOnScreen( &nextWindowRect[1], windowLoc[1], 0.25 );

	nextSurface = LoadPICTAsSurface( picNext, 16 );

	nextDrawSurface = SDLU_InitSurface( &nextWindowZRect, 16 );
}

void RefreshNext( int player )
{
	nextStage[player][0] = 0;
	nextStage[player][1] = 0;

	nextTime[player][0] = GameTickCount( ) + RandomBefore( 60 );
	nextTime[player][1] = GameTickCount( ) + RandomBefore( 60 );

	ShowNext( player );
}

void PullNext( int player )
{
	pullA[player] = nextA[player];
	pullB[player] = nextB[player];
	nextStage[player][0] = kPulling;
	nextTime[player][0] = GameTickCount( );
}

#define kNoDraw 999
void ShowPull( int player )
{
	SDL_Rect    srcRect;
	int      yank[8] = { 20, 18, 15, 8, -6, -26, -46, -66 };
	int      slide[8] = { kNoDraw, 66, 48, 36, 29, 26, 24, 23 };
	int      drawA, drawB, offset, count;

	if( !nextWindowVisible[player] ) return;

	SDLU_AcquireSurface( nextDrawSurface );

	SDLU_BlitSurface( nextSurface,     &nextSurface->clip_rect,
	                  nextDrawSurface, &nextDrawSurface->clip_rect );

	for( count=0; count<2; count++ )
	{
		offset = nextStage[player][0] - kPulling;

		switch( count )
		{
			case 0: drawA = pullA[player]; drawB = pullB[player]; offset = yank[offset];  break;
			case 1: drawA = nextA[player]; drawB = nextB[player]; offset = slide[offset]; break;
		}

		if( offset != kNoDraw )
		{
			SDL_Rect blobRect =   { .y = 0, .x = 4, .h = kBlobVertSize, .w = kBlobHorizSize };
			SDL_Rect shadowRect = { .y = 4, .x = 8, .h = kBlobVertSize, .w = kBlobHorizSize };

			SDLU_OffsetRect( &blobRect, 0, offset );
			SDLU_OffsetRect( &shadowRect, 0, offset );

			SurfaceDrawShadow( &shadowRect, drawB, kNoSuction );

			CalcBlobRect( kNoSuction, drawB-1, &srcRect );
			SurfaceBlitBlob( &srcRect, &blobRect );

			SDLU_OffsetRect( &blobRect, 0, kBlobVertSize );
			SDLU_OffsetRect( &shadowRect, 0, kBlobVertSize );

			SurfaceDrawShadow( &shadowRect, drawA, nextM[player]? kFlashDarkBlob: kNoSuction );

			CalcBlobRect( nextM[player]? kFlashDarkBlob: kNoSuction, drawA-1, &srcRect );
			SurfaceBlitBlob( &srcRect, &blobRect );
		}
	}

	SDLU_ReleaseSurface( nextDrawSurface );
	// INVESTIGATE: does not copying the rects wreck shit?
	SDLU_BlitFrontSurface( nextDrawSurface, &nextWindowZRect, &nextWindowRect[player] );
}

void UpdateNext( int player )
{
	bool changed = false;
	int blob;

	if( nextStage[player][0] >= kPulling )
	{
		if( GameTickCount() > nextTime[player][0] )
		{
			if( ++nextStage[player][0] >= kPullingFrames )
			{
				RefreshNext( player );
			}
			else
			{
				ShowPull( player );
				nextTime[player][0]++;
			}
		}
	}
	else
	{
		for( blob=0; blob<2; blob++ )
		{
			if( GameTickCount() > nextTime[player][blob] )
			{
				if( ++nextStage[player][blob] >= kJiggleFrames )
				{
					nextStage[player][blob] = 0;
					nextTime[player][blob] += 40 + RandomBefore( 80 );
				}
				else
				{
					nextTime[player][blob] += 2;
				}

				changed = true;
			}
		}

		if( changed ) ShowNext( player );
	}
}

void ShowNext( int player )
{
	int      jiggle[kJiggleFrames] = { kNoSuction,  kSquish,  kNoSuction,  kSquash,
	                                   kNoSuction,  kSquish,  kNoSuction,  kSquash   };
	int      nextFrame  = kNoSuction;
	SDL_Rect blobRect   = { .y = 22, .x = 4, .h = kBlobVertSize, .w = kBlobHorizSize };
	SDL_Rect shadowRect = { .y = 26, .x = 8, .h = kBlobVertSize, .w = kBlobHorizSize };
	SDL_Rect srcRect;

	if( !nextWindowVisible[player] || control[player] == kNobodyControl ) {
		return;
	}	else {
		SDLU_AcquireSurface( nextDrawSurface );

		SDLU_BlitSurface( nextSurface,     &nextSurface->clip_rect,
		                  nextDrawSurface, &nextDrawSurface->clip_rect );

		nextFrame = nextG[player]? kNoSuction: jiggle[nextStage[player][0]];

		SurfaceDrawShadow( &shadowRect, nextB[player], nextFrame );

		CalcBlobRect( nextFrame, nextB[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );

		SDLU_OffsetRect( &blobRect, 0, kBlobVertSize );
		SDLU_OffsetRect( &shadowRect, 0, kBlobVertSize );

		nextFrame = nextG[player]? kNoSuction:
						(nextM[player]? kFlashDarkBlob: jiggle[nextStage[player][1]]);

		SurfaceDrawShadow( &shadowRect, nextA[player], nextFrame );

		CalcBlobRect( nextFrame, nextA[player]-1, &srcRect );
		SurfaceBlitBlob( &srcRect, &blobRect );

		SDLU_ReleaseSurface( nextDrawSurface );

		SDLU_BlitFrontSurface( nextDrawSurface, &nextWindowZRect, &nextWindowRect[player] );
	}
}
