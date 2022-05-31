//  SDLU.h
#pragma once

#include <stdbool.h>

int          SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect );
void         SDLU_GetPixel( SDL_Surface* surface, int x, int y, SDL_Color* pixel );
void         SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth );
SDL_Surface* SDLU_InitSurface( const SDL_Rect* rect, int depth );
void         SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect );
void         SDLU_AcquireSurface( SDL_Surface* surface );
SDL_Surface* SDLU_GetCurrentSurface( void );
void         SDLU_ReleaseSurface( SDL_Surface* surface );
void         SDLU_GetMouse( SDL_Point* pt );
int          SDLU_Button( void );
void         SDLU_Yield( void );
void         SDLU_PumpEvents( void );
int          SDLU_EventFilter( void *userdata, SDL_Event *const event );
void         SDLU_StartWatchingTyping( void );
void         SDLU_StopWatchingTyping( void );
bool         SDLU_CheckTyping( char* ascii, SDL_Scancode* sdl );
bool         SDLU_IsForeground( void );
bool         SDLU_SeparateRects( const SDL_Rect *a, const SDL_Rect *b );
void         SDLU_UnionRect( const SDL_Rect* a, const SDL_Rect* b, SDL_Rect* u );
void         SDLU_OffsetRect( SDL_Rect* r, int x, int y );
