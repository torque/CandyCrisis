//  SDLU.h
#pragma once

#include "MTypes.h"

typedef struct SDLU_Point
{
	int x;
	int y;
} SDLU_Point;

SDL_Rect*    SDLU_MRectToSDLRect( const MRect* in, SDL_Rect* out );
MRect*       SDLU_SDLRectToMRect( const SDL_Rect* in, MRect* out );
int          SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect );
void         SDLU_GetPixel( SDL_Surface* surface, int x, int y, SDL_Color* pixel );
void         SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth );
SDL_Surface* SDLU_InitSurface( SDL_Rect* rect, int depth );
void         SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect );
void         SDLU_AcquireSurface( SDL_Surface* surface );
SDL_Surface* SDLU_GetCurrentSurface();
void         SDLU_ReleaseSurface( SDL_Surface* surface );
void         SDLU_GetMouse( SDLU_Point* pt );
int          SDLU_Button();
void         SDLU_Yield();
void         SDLU_PumpEvents();
int          SDLU_EventFilter( const SDL_Event *event );
void         SDLU_StartWatchingTyping();
void         SDLU_StopWatchingTyping();
bool         SDLU_CheckTyping( char* ascii, SDLKey* sdl );
bool         SDLU_IsForeground();
bool         SDLU_PointInRect( SDLU_Point p, MRect* r );
