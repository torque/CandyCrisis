// tweak.h
#pragma once

void TweakFirstBlob( int player, SDL_Rect *first );
void TweakSecondBlob( int player, SDL_Rect *second );
void StartTweak( int player, int direction, int rotate, int fall );
void UpdateTweak( int player, int suction );
void InitTweak( void );

#define d2r(x) ((x)*(pi/180))

#define kTweakDelay 1
