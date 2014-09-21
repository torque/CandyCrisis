// next.h
#pragma once

#include <SDL/SDL.h>


void InitNext( void );
void ShowNext( int player );
void RefreshNext( int player );
void UpdateNext( int player );
void PullNext( int player );
void ShowPull( int player );

extern SDL_Surface* nextSurface;
extern SDL_Surface* nextDrawSurface;

extern bool nextWindowVisible[2];
extern SDL_Rect nextWindowZRect, nextWindowRect[2];
