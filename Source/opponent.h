// opponent.h
#pragma once

#include "level.h" // kglows

void InitOpponent( void );
void BeginOpponent( int which );
void UpdateOpponent( void );
void DrawFrozenOpponent( void );
void OpponentChatter( bool on );
void OpponentPissed( void );

enum
{
	kBored=0,
	kLookRight,
	kLookLeft,
	kBlink1,
	kBlink2
};

#define kOppFrames (kBlink2+1)
#define kGlowArraySize 30

extern int opponentMood, opponentFrame;
extern int opponentTime, glowTime[kGlows], glowFrame[kGlows], panicTime, panicFrame;
extern int glowArray[kGlowArraySize], lightGlowArray[kGlowArraySize];

extern SDL_Rect opponentWindowZRect, opponentWindowRect;
