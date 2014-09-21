// graphics.c

#include <stdlib.h>

#include <SDL/SDL.h>
#include "SDLU.h"

#include "graphics.h"

#include "blitter.h"
#include "gameticks.h"
#include "grays.h"
#include "gworld.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "moving.h"
#include "players.h"
#include "tweak.h"
#include "victory.h"

SDL_Surface* backdropSurface = NULL;

void DrawSpriteBlobs( int player, int type )
{
	SDL_Rect firstRect, secondRect, thirdRect;
	const int repeat = 0xFF, forever = 0xFE;

	static const unsigned char blobAnimation[6][2][25] =
	{
		{ { kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob,
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob,
		    kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, kFlashBrightBlob, repeat },
		  { kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction,
		    kNoSuction,       kNoSuction,       kNoSuction,       kNoSuction, repeat } },
		{ { kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       forever },
		  { kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       kSquish,          kNoSuction,       kSquash,
		    kNoSuction,       forever } },
		{ { kSobBlob,         kSobBlob,         kSobBlob,         kSobBlob,
		    kSob2Blob,        kSob2Blob,        kSob2Blob,        kSob2Blob,
		    repeat },
		  { kSobBlob,         kSobBlob,         kSobBlob,         kSobBlob,
		    kSob2Blob,        kSob2Blob,        kSob2Blob,        kSob2Blob,
		    repeat } },
		{ { kBombFuse1,       kBombFuse2,       kBombFuse3,       repeat },
		  { kBombFuse1,       kBombFuse2,       kBombFuse3,       repeat } },
		{ { kBlinkBomb1,      kBombFuse2,       kBlinkBomb3,      kBombFuse1,
		    kBlinkBomb2,      kBombFuse3,       repeat },
		  { kBlinkBomb1,      kBombFuse2,       kBlinkBomb3,      kBombFuse1,
		    kBlinkBomb2,      kBombFuse3,       repeat } }
	};

	if( grenade[player] ) type += 3;

	SDLU_AcquireSurface( playerSpriteSurface[player] );

	if( blobAnimation[type][0][anim[player]] == forever ) anim[player]--;
	if( blobAnimation[type][0][anim[player]] == repeat  ) anim[player] = 0;

	CalcBlobRect( blobX[player], blobY[player], &firstRect );
	if( halfway[player] ) SDLU_OffsetRect( &firstRect, 0, kBlobVertSize / 2 );

	TweakFirstBlob ( player, &firstRect );
	secondRect = firstRect;
	TweakSecondBlob( player, &secondRect );

	thirdRect = firstRect;
	thirdRect.x -= kBlobShadowError;
	thirdRect.w += kBlobShadowDepth + 2*kBlobShadowError;
	thirdRect.y -= kBlobShadowError;
	thirdRect.h += kBlobShadowDepth + 2*kBlobShadowError;
	CleanSpriteArea( player, &thirdRect );

	thirdRect = secondRect;
	thirdRect.x -= kBlobShadowError;
	thirdRect.w += kBlobShadowDepth + 2*kBlobShadowError;
	thirdRect.y -= kBlobShadowError;
	thirdRect.h += kBlobShadowDepth + 2*kBlobShadowError;
	CleanSpriteArea( player, &thirdRect );

	thirdRect = firstRect;
	SDLU_OffsetRect( &thirdRect, shadowDepth[player], shadowDepth[player] );
	SurfaceDrawShadow( &thirdRect,  colorA[player], blobAnimation[type][0][anim[player]] );

	thirdRect = secondRect;
	SDLU_OffsetRect( &thirdRect, shadowDepth[player], shadowDepth[player] );
	SurfaceDrawShadow( &thirdRect, colorB[player], blobAnimation[type][1][anim[player]] );

	SurfaceDrawSprite( &firstRect,  colorA[player], blobAnimation[type][0][anim[player]] );

	SurfaceDrawSprite( &secondRect, colorB[player], blobAnimation[type][1][anim[player]] );

	SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void CleanSpriteArea( int player, SDL_Rect *myRect )
{
	SDLU_BlitSurface( playerSurface[player],       &myRect,
	                  playerSpriteSurface[player], &myRect  );

	SetUpdateRect( player, myRect );
}

void EraseSpriteBlobs( int player )
{
	SDL_Rect myRect, secondRect;

	CalcBlobRect( blobX[player], blobY[player], &myRect );
	if( halfway[player] ) SDLU_OffsetRect( &myRect, 0, kBlobVertSize / 2 );

	TweakFirstBlob( player, &myRect );
	secondRect = myRect;
	secondRect.x -= kBlobShadowError;
	secondRect.w += kBlobShadowDepth + 2*kBlobShadowError;
	secondRect.y -= kBlobShadowError;
	secondRect.h += kBlobShadowDepth + 2*kBlobShadowError;
	CleanSpriteArea( player, &secondRect );

	TweakSecondBlob( player, &myRect );
	myRect.x -= kBlobShadowError;
	myRect.w += kBlobShadowDepth + 2*kBlobShadowError;
	myRect.y -= kBlobShadowError;
	myRect.h += kBlobShadowDepth + 2*kBlobShadowError;
	CleanSpriteArea( player, &myRect );
}

void CalcBlobRect( int x, int y, SDL_Rect *blobRect )
{
	blobRect->x = x * kBlobHorizSize;
	blobRect->y = y * kBlobVertSize;
	blobRect->w = kBlobHorizSize;
	blobRect->h = kBlobVertSize;
}

void InitBackdrop( void )
{
	backdropSurface = LoadPICTAsSurface( picBackdrop, 16 );
}

void DrawBackdrop( void )
{
	SDL_Rect backdropRect = { .x = 0, .y = 0, .w = 640, .h = 480 };

	SDLU_BlitFrontSurface( backdropSurface, &backdropRect, &backdropRect );
}

void ShowTitle( void )
{
	int time;

	DrawPICTInSurface( frontSurface, picTitle );

	SDL_Flip( frontSurface );

	QuickFadeIn();

	time = MTickCount() + 180;

	RetrieveResources( );

	while( time > MTickCount() && !SDLU_Button() )
	{
		SDLU_Yield();
	}

	WaitForRelease();

	QuickFadeOut();

	SDL_FillRect( frontSurface, &frontSurface->clip_rect, SDL_MapRGB( frontSurface->format, 0, 0, 0 ) );
	SDL_Flip( frontSurface );
}

