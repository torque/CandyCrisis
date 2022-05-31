// gworld.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDLU.h"

#include "gworld.h"

#include "blitter.h"
#include "graphics.h"
#include "main.h"

SDL_Surface* blobSurface;
SDL_Surface* maskSurface;
SDL_Surface* charMaskSurface;
SDL_Surface* boardSurface[2];
SDL_Surface* blastSurface;
SDL_Surface* blastMaskSurface;
SDL_Surface* playerSurface[2];
SDL_Surface* playerSpriteSurface[2];

void GetBlobGraphics()
{
	boardSurface[0] = LoadPICTAsSurface( picBoard, 16 );

	if ( PICTExists( picBoardRight ) ) {
		boardSurface[1] = LoadPICTAsSurface( picBoardRight, 16 );
	} else {
		boardSurface[1] = LoadPICTAsSurface( picBoard, 16 );
	}
	// Get blob worlds

	blobSurface = LoadPICTAsSurface( picBlob, 16 );
	maskSurface = LoadPICTAsSurface( picBlobMask, 1 );
	charMaskSurface = LoadPICTAsSurface( picCharMask, 1 );

	// Get blast worlds

	blastSurface = LoadPICTAsSurface( picBlast, 16 );
	blastMaskSurface = LoadPICTAsSurface( picBlastMask, 16 );
}

void InitPlayerWorlds()
{
	SDL_Rect  gridRect;
	int       count;

	gridRect.y = gridRect.x = 0;
	gridRect.w = kGridAcross * kBlobHorizSize;
	gridRect.h = kGridDown * kBlobVertSize;

	for( count=0; count<=1; count++ )
	{
		playerSurface[count]       = SDLU_InitSurface( &gridRect, 16 );
		playerSpriteSurface[count] = SDLU_InitSurface( &gridRect, 16 );
	}
}

void SurfaceDrawBoard( int player, const SDL_Rect *myRect )
{
	SDL_Rect srcRect, offsetRect;

	srcRect = *myRect;
	// INVESTIGATE
	if( (srcRect.y + srcRect.h) <= kBlobVertSize ) {
		return;
	}
	if( srcRect.y < kBlobVertSize ) {
		srcRect.y = kBlobVertSize;
	}

	offsetRect = srcRect;
	SDLU_OffsetRect( &offsetRect, 0, -kBlobVertSize );

	SDLU_BlitSurface( boardSurface[player],     &offsetRect,
	                  SDLU_GetCurrentSurface(), &srcRect     );
}

void SurfaceDrawBlob( int player, const SDL_Rect *myRect, int blob, int state, int charred )
{
	SurfaceDrawBoard( player, myRect );
	SurfaceDrawSprite( myRect, blob, state );

	if( charred & 0x0F )
	{
		SDL_Rect blobRect, charRect, alphaRect;

		CalcBlobRect( (charred & 0x0F), kBombTop-1, &charRect );
		CalcBlobRect( (charred & 0x0F), kBombBottom-1, &alphaRect );
		CalcBlobRect( state, blob-1, &blobRect );

		SurfaceBlitWeightedDualAlpha(  SDLU_GetCurrentSurface(),  blobSurface,  charMaskSurface,  blobSurface,  SDLU_GetCurrentSurface(),
		                               myRect,                   &charRect,    &blobRect,        &alphaRect,    myRect,
		                              (charred & 0xF0)>>3 );
	}
}

void SurfaceDrawShadow( const SDL_Rect *myRect, int blob, int state )
{
	int x;
	SDL_Point offset[4] = {
		{ .y = -2, .x =  0 },
		{ .y =  0, .x = -2 },
		{ .y =  2, .x =  0 },
		{ .y =  0, .x =  2 }
	};

	if( blob > kEmpty )
	{
		SDL_Rect blobRect, destRect;

		for( x=0; x<4; x++ )
		{
			destRect = *myRect;
			SDLU_OffsetRect( &destRect, offset[x].x, offset[x].y );

			CalcBlobRect( state, blob-1, &blobRect );
			SurfaceBlitColor(  maskSurface,  SDLU_GetCurrentSurface(),
			                  &blobRect,    &destRect,
			                   0, 0, 0, 3 );
		}
	}
}

void SurfaceDrawColor( const SDL_Rect *myRect, int blob, int state, int r, int g, int b, int w )
{
	SDL_Rect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitColor(  charMaskSurface,  SDLU_GetCurrentSurface(),
		                  &blobRect,         myRect,
		                   r, g, b, w );
	}
}

void SurfaceDrawAlpha( const SDL_Rect *myRect, int blob, int mask, int state )
{
	if( blob > kEmpty )
	{
		SDL_Rect blobRect, alphaRect;

		CalcBlobRect( state, blob-1, &blobRect );
		CalcBlobRect( state, mask-1, &alphaRect );

		SurfaceBlitAlpha( SDLU_GetCurrentSurface(),  blobSurface,  blobSurface, SDLU_GetCurrentSurface(),
		                  myRect,                   &blobRect,    &alphaRect,   myRect );
	}
}

void SurfaceDrawSprite( const SDL_Rect *myRect, int blob, int state )
{
	SDL_Rect blobRect;
	if( blob > kEmpty )
	{
		CalcBlobRect( state, blob-1, &blobRect );
		SurfaceBlitBlob( &blobRect, myRect );
	}
}

bool PICTExists( int pictID )
{
	if( FileExists( QuickResourceName( "PICT", pictID, ".jpg" ) ) )
		return true;

	if( FileExists( QuickResourceName( "PICT", pictID, ".png" ) ) )
		return true;

	return false;
}

SDL_Surface* LoadPICTAsSurface( int pictID, int depth )
{
	const char*  filename;
	SDL_Surface* surface;

	filename = QuickResourceName( "PICT", pictID, ".jpg" );
	if( FileExists( filename ) )
	{
		surface = IMG_Load( filename );
	}
	else
	{
		filename = QuickResourceName( "PICT", pictID, ".png" );
		if( FileExists( filename ) )
		{
			surface = IMG_Load( filename );
		}
		else
		{
			// Fail
			return NULL;
		}
	}

	if( depth != 0 )
	{
		SDLU_ChangeSurfaceDepth( &surface, depth );
	}

	return surface;
}

void DrawPICTInSurface( SDL_Surface* surface, int pictID )
{
	SDL_Surface* image;

	image = LoadPICTAsSurface( pictID, 0 );
	if( image != NULL )
	{
		SDLU_BlitSurface( image,    &image->clip_rect,
		                  surface,  &surface->clip_rect );

		SDL_FreeSurface( image );
	}
}
