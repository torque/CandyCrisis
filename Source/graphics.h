// graphics.h
#pragma once

#include <SDL/SDL.h>

void DrawSpriteBlobs( int player, int type );
void EraseSpriteBlobs( int player );
void CleanSpriteArea( int player, SDL_Rect *myRect );
void CalcBlobRect( int x, int y, SDL_Rect *myRect );
void DrawBackdrop( void );
void ShowTitle( void );
void InitBackdrop( void );

extern SDL_Surface* backdropSurface;

enum
{
	blobBlinkAnimation = 0,
	blobJiggleAnimation,
	blobCryAnimation
};
