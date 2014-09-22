//  SDLU.h
#pragma once

typedef struct SDLU_Point
{
	int x;
	int y;
} SDLU_Point;

int          SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect );
void         SDLU_GetPixel( SDL_Surface* surface, int x, int y, SDL_Color* pixel );
void         SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth );
SDL_Surface* SDLU_InitSurface( const SDL_Rect* rect, int depth );
void         SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect );
void         SDLU_AcquireSurface( SDL_Surface* surface );
SDL_Surface* SDLU_GetCurrentSurface( void );
void         SDLU_ReleaseSurface( SDL_Surface* surface );
void         SDLU_GetMouse( SDLU_Point* pt );
int          SDLU_Button( void );
void         SDLU_Yield( void );
void         SDLU_PumpEvents( void );
int          SDLU_EventFilter( const SDL_Event *event );
void         SDLU_StartWatchingTyping( void );
void         SDLU_StopWatchingTyping( void );
bool         SDLU_CheckTyping( char* ascii, SDLKey* sdl );
bool         SDLU_IsForeground( void );
bool         SDLU_PointInRect( SDLU_Point p, SDL_Rect* r );
bool         SDLU_RectOverlap( const SDL_Rect *a, const SDL_Rect *b );
void         SDLU_UnionRect( const SDL_Rect* a, const SDL_Rect* b, SDL_Rect* u );
void         SDLU_OffsetRect( SDL_Rect* r, int x, int y );
