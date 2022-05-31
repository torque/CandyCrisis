// blitter.h
#pragma once

#include <SDL2/SDL.h>

#include "font.h"
#include "main.h"

// SDL type
void SurfaceBlitMask( SDL_Surface*    object,     SDL_Surface*    mask,     SDL_Surface*    dest,
                      const SDL_Rect* objectRect, const SDL_Rect* maskRect, const SDL_Rect* destRect );

void SurfaceBlitBlob( const SDL_Rect* blobRect, const SDL_Rect* destRect );

void SurfaceBlitColor( SDL_Surface*    mask,     SDL_Surface*    dest,
                       const SDL_Rect* maskRect, const SDL_Rect* destRect,
                       int r, int g, int b, int weight );

void SurfaceBlitAlpha( SDL_Surface*    back,     SDL_Surface*    source,     SDL_Surface*    alpha,     SDL_Surface*    dest,
                       const SDL_Rect* backRect, const SDL_Rect* sourceRect, const SDL_Rect* alphaRect, const SDL_Rect* destRect );

void SurfaceBlitWeightedDualAlpha( SDL_Surface*    back,     SDL_Surface*    source,  SDL_Surface*    mask,     SDL_Surface*    alpha,     SDL_Surface*    dest,
                                   const SDL_Rect* backRect, const SDL_Rect* srcRect, const SDL_Rect* maskRect, const SDL_Rect* alphaRect, const SDL_Rect* destRect,
                                   int inWeight );

void SurfaceBlitWeightedCharacter( SkittlesFontPtr font, unsigned char text, SDLU_Point *dPoint, int r, int g, int b, int alpha );

void SurfaceBlitCharacter( SkittlesFontPtr font, unsigned char text, SDLU_Point *dPoint, int r, int g, int b, int dropShadow );

void SurfaceBlitBlendOver( SDL_Surface*    source,     SDL_Surface*    dest,
                           const SDL_Rect* sourceRect, const SDL_Rect* destRect,
                           int r1, int g1, int b1,
                           int r2, int g2, int b2,
                           int r3, int g3, int b3,
                           int r4, int g4, int b4,
                           int weight );

void SurfaceBlitColorOver( SDL_Surface*    source,     SDL_Surface*    dest,
                           const SDL_Rect* sourceRect, const SDL_Rect* destRect,
                           int r, int g, int b, int weight );

void SetUpdateRect( int player, SDL_Rect *where );
void UpdatePlayerWindow( int player );
void InitBlitter( void );

extern bool       update[2][kGridAcross][kGridDown];
extern bool       refresh[2];
extern SDLU_Point topLeft[2];
