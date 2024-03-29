// score.c

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "score.h"

#include "blitter.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "hiscore.h"
#include "level.h"
#include "main.h"

SDL_Surface* scoreSurface;
SDL_Surface* numberSurface;
SDL_Surface* numberMaskSurface;

SDL_Rect scoreWindowZRect = { .x = 0, .y = 0, .w = 144, .h = 32 }, scoreWindowRect[2];
bool scoreWindowVisible[2] = {true, true};
long roundStartScore[2], score[2], displayedScore[2], scoreTime[2];
const char characterList[] =
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', '.', '0', '1', '2',
  '3', '4', '5', '6', '7', '8', '9', '!', '"', '#',
  '$' };


void InitScore( void )
{
	const double windowLoc[ ] = { 0.16, 0.84 };

	scoreWindowRect[0] = scoreWindowRect[1] = scoreWindowZRect;
	CenterRectOnScreen( &scoreWindowRect[0], windowLoc[0], 0.89 );
	CenterRectOnScreen( &scoreWindowRect[1], windowLoc[1], 0.89 );

	scoreSurface = SDLU_InitSurface( &scoreWindowZRect, 16 );
	DrawPICTInSurface( scoreSurface, picBoard );

	numberSurface = LoadPICTAsSurface( picNumber, 16 );

	numberMaskSurface = LoadPICTAsSurface( picNumberMask, 1 );

	displayedScore[0] = displayedScore[1] = 0;
	score[0]          = score[1]          = 0;
	scoreTime[0]      = scoreTime[1]      = 0;
}

void UpdateScore( int player )
{
	if( GameTickCount( ) >= scoreTime[player] )
	{
		scoreTime[player] = GameTickCount() + 1;

		if( displayedScore[player] < score[player] )
		{
			if( (score[player] - displayedScore[player]) > 5000 )
			{
				displayedScore[player] += 1525;
			}
			else if( (score[player] - displayedScore[player]) > 1000 )
			{
				displayedScore[player] += 175;
			}
			else
			{
				displayedScore[player] += 25;
			}

			if( displayedScore[player] > score[player] )
				displayedScore[player] = score[player];

			ShowScore( player );
		}
	}
}

void ShowScore( int player )
{
	SDL_Rect   myRect;
	char       myString[256];
	int        count;

	if( !scoreWindowVisible[player] || control[player] == kNobodyControl )
	{
		return;
	}

	sprintf( myString, "%ld", displayedScore[player] );

	SDLU_AcquireSurface( scoreSurface );

	SDLU_BlitSurface( boardSurface[player], &scoreSurface->clip_rect,
	                  scoreSurface,         &scoreSurface->clip_rect   );

	myRect.y = 0;
	myRect.x = 2;
	myRect.h = kNumberVertSize;
	myRect.w = kNumberHorizSize;
	DrawCharacter( kCharacterScore,   &myRect );
	SDLU_OffsetRect( &myRect, kNumberHorizSize, 0 );
	DrawCharacter( kCharacterScore+1, &myRect );

	myRect = scoreWindowZRect;
	myRect.x = myRect.w - 2 - kNumberHorizSize;
	myRect.w = kNumberHorizSize;
	for( count = strlen(myString) - 1; count >= 0; count-- )
	{
		DrawCharacter( myString[count], &myRect );
		SDLU_OffsetRect( &myRect, -kNumberHorizSize - 1, 0 );
	}

	SDLU_ReleaseSurface( scoreSurface );

	SDLU_BlitFrontSurface( scoreSurface, &scoreWindowZRect, &scoreWindowRect[player] );

}

void DrawCharacter( char which, const SDL_Rect *myRect )
{
	SDL_Rect srcRect;
	char     count, result;

	result = -1;
	for( count = 0; count < kNumberAmount; count++ )
	{
		if( characterList[(int)count] == which )
		{
			result = count;
			break;
		}
	}

	if( result == -1 ) return;

	srcRect.y = 0;
	srcRect.x = result * kNumberHorizSize;
	srcRect.h = kNumberVertSize;
	srcRect.w = kNumberHorizSize;

	SurfaceBlitMask(  numberSurface,  numberMaskSurface,  SDLU_GetCurrentSurface(),
	                 &srcRect,       &srcRect,            myRect );
}
