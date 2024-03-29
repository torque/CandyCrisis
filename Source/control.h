// control.h
#pragma once

#include <stdbool.h>

#include "main.h"

void PlayerControl( int player );
void AIControl( int player );
void ChooseAIDestination( int player );
int TestAIDestination( int player, int testX, int testR );
int GetRowHeight( int player, int row );
int DetermineEmotion( int player );
int TestTemporaryGrid( void );
void QuickRemove( signed char myGrid[kGridAcross][kGridDown], int x, int y, int color );
bool CanReach( int player, int testX );
int ScoreTemporaryGrid( void );
int BestColor( int player, int BlobX, int BlobY );
void LogText( char string[] );
void LogGrid( signed char myGrid[kGridAcross][kGridDown] );
void AutoControl( int player );

extern int destinationX[2], destinationR[2];
extern signed char tempGrid[kGridAcross][kGridDown];
extern long timeAI[2];
extern bool moveQuick[2];
enum
{
	kEmotionNeutral = 0,
	kEmotionSad,
	kEmotionHappy,
	kEmotionPanic
};
