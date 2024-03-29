// victory.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "victory.h"

#include "blitter.h"
#include "control.h"
#include "font.h"
#include "gameticks.h"
#include "graphics.h"
#include "grays.h"
#include "gworld.h"
#include "hiscore.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "music.h"
#include "pause.h"
#include "players.h"
#include "random.h"
#include "score.h"
#include "soundfx.h"
#include "tweak.h"
#include "zap.h"

unsigned long winTime, loseTime;
long winStage, loseStage;
float drop[kGridAcross], last[kGridAcross];
SkittlesFontPtr victoryFont;

void InitVictory( void )
{
	victoryFont = GetFont( picVictoryFont );
}

void EndRound( int player )
{
	int count;

	loseTime = GameTickCount( );
	loseStage = 0;

	role[player] = kLosing;
	emotions[player] = kEmotionPanic;

	for( count=0; count<kGridAcross; count++ )
	{
		last[count] = 0.1;
		drop[count] = 0.4 + RandomBefore(1000)/20000;
		rowBounce[player][count] = 99;
	}

	if( player == 0 )
	{
		ChooseMusic( -1 );
		PlayMono( kLoss );
	}
}

void BeginVictory( int player )
{
	int count;

	endTime = GameTickCount( );
	winTime = GameTickCount( );
	winStage = 0;

	role[player] = kWinning;
	emotions[player] = kEmotionHappy;

	EraseSpriteBlobs( player );

	for( count=0; count<kGridAcross; count++ )
	{
		rowBounce[player][count] = 99;
	}

	if( player == 0 )
	{
		ChooseMusic( -1 );
		PlayMono( kVictory );
	}
}

void Lose( int player )
{
	int       gameTime = GameTickCount();
	int       skip = 1;
	SDL_Rect  boardRect;

	if( gameTime < loseTime )
		return;

	if( gameTime > loseTime )
	{
		skip = 2;
	}

	loseTime  += skip;
	loseStage += skip;

	if( loseStage < 120 )
	{
		DropLosers( player, skip );
	}
	else if( loseStage == 120 || loseStage == 121 )
	{
		loseStage = 122;

		boardRect.y = boardRect.x = 0;
		boardRect.h = playerSurface[player]->h;
		boardRect.w = playerSurface[player]->w;

		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &boardRect );
		SDLU_ReleaseSurface( playerSurface[player] );

		CleanSpriteArea( player, &boardRect );
	}
	else if( loseStage == 240 || loseStage == 241 )
	{
		loseStage = 242;
		if( players == 1 && control[player] == kPlayerControl )
		{
			if( --credits > 0 )
			{
				HandleDialog( kContinueDialog );
			}
			else
			{
				AddHiscore( score[player] );
				ShowGameOverScreen( );

				showStartMenu = true;
			}
		}
		else if( players == 2 )
		{
			AddHiscore( score[player] );
		}
	}
}

void DropLosers( int player, int skip )
{
	int x, y, suck;
	int beginDrop[] = { 28, 14, 0, 7, 21, 35 };
	float thisDrop;
	SDL_Rect myRect;

	SDLU_AcquireSurface( playerSpriteSurface[player] );

	for( x=0; x<kGridAcross; x++ )
	{
		if( loseStage >= beginDrop[x] )
		{
			thisDrop = last[x] + ( (float)(skip) * ( 0.7 + last[x] / 12.0 ) );

			CalcBlobRect( x, 0, &myRect );
			myRect.y = (int) last[x];
			// INVESTIGATE
			myRect.h = kGridDown * kBlobVertSize - myRect.y;
			SurfaceDrawBoard( player, &myRect );
			SetUpdateRect( player, &myRect );

			if( thisDrop <  (kGridDown*kBlobVertSize) )
			{
				myRect.y = (int) thisDrop;
				myRect.h = kBlobVertSize;

				y=0;
				while( myRect.y < (kGridDown*kBlobVertSize) )
				{
					if( grid[player][x][y] >= kFirstBlob &&
						grid[player][x][y] <= kLastBlob )
					{
						suck = suction[player][x][y] & (kUpDown);
						if( suck == kNoSuction ) suck = kDying;
						SurfaceDrawBlob( player, &myRect, grid[player][x][y], suck, charred[player][x][y] );
					}
					else if( grid[player][x][y] == kGray )
					{
						SurfaceDrawAlpha( &myRect, kGray, kLight, kGrayNoBlink );
					}

					SDLU_OffsetRect( &myRect, 0, kBlobVertSize );
					y++;
				}

				last[x] = thisDrop;
			}
		}
	}

	SDLU_ReleaseSurface( playerSpriteSurface[player] );
}

void Win( int player )
{
	int x, y;

	if( GameTickCount() >= winTime )
	{
		if( winStage < (kGridDown * kGridAcross) )
		{
			y = (kGridDown-1) - (winStage / kGridAcross);
			x = (winStage % kGridAcross);
			if( y & 2 ) x = (kGridAcross-1) - x;

			if( grid[player][x][y] == kGray )
			{
				suction[player][x][y] = kGrayBlink1;
				death[player][x][y] = 0;
				score[player] += 20;
			}
			else if( grid[player][x][y] >= kFirstBlob && grid[player][x][y] <= kLastBlob )
			{
				suction[player][x][y] = kInDeath;
				death[player][x][y] = 0;
				score[player] += 100;
			}
		}
		else if( winStage == 140 && control[player] == kPlayerControl )
		{
			DrawTimerCount( player );
		}
		else if( winStage == 200 && control[player] == kPlayerControl )
		{
			DrawTimerBonus( player );
		}

		winTime++;
		winStage++;
	}

	if( winStage < 140 )
	{
		KillBlobs( player );
	}

	if( winStage >= 280 )
	{
		if( control[player] == kPlayerControl )
		{
			IncrementLevel( );
			BeginRound( true );
		}
	}
}

void DrawTimerCount( int player )
{
	SDL_Rect playerRect;
	SDL_Point dPoint  = { .y = (kBlobVertSize * 3), .x = 15 };

	SDLU_AcquireSurface( playerSurface[player] );

	SurfaceBlitCharacter( victoryFont, 'A', &dPoint,  31, 31, 0, 1  );

	dPoint.y = (kBlobVertSize * 4);
	dPoint.x = kBlobHorizSize;
	char seconds[20];
	char *scan = seconds;

	sprintf( seconds, "%ld", (endTime - startTime) / 60 );
	while( *scan )
	{
		SurfaceBlitCharacter( zapFont, *scan++, &dPoint, 31, 31, 31, 1  );
		dPoint.x--;
	}

	dPoint.x += 6;
	SurfaceBlitCharacter( zapFont, 'S', &dPoint,  31, 31, 31, 1  );

	playerRect.y = playerRect.x = 0;
	playerRect.h = playerSurface[player]->h;
	playerRect.w = playerSurface[player]->w;

	CleanSpriteArea( player, &playerRect );
	PlayStereo( player, kSplop );

	SDLU_ReleaseSurface( playerSurface[player] );
}

void DrawTimerBonus( int player )
{
	SDL_Rect playerRect;
	SDL_Point dPoint  = { .y = (kBlobVertSize * 6), .x = 15 };
	int timer, bonus;

	SDLU_AcquireSurface( playerSurface[player] );


	SurfaceBlitCharacter( victoryFont, 'B', &dPoint, 31, 31, 0, 1  );

	timer = (endTime - startTime) / 60;
	     if( timer <=  10 ) bonus = 30000;
	else if( timer <=  20 ) bonus = 10000;
	else if( timer <=  30 ) bonus =  5000;
	else if( timer <=  45 ) bonus =  4000;
	else if( timer <=  60 ) bonus =  3000;
	else if( timer <=  80 ) bonus =  2000;
	else if( timer <= 100 ) bonus =  1000;
	else if( timer <= 120 ) bonus =   500;
	else                    bonus =     0;

	if( players == 1 ) bonus *= level;

	score[player] += bonus;

	dPoint.y = (kBlobVertSize * 7);
	dPoint.x = kBlobHorizSize;
	char points[20];
	char *scan = points;

	sprintf( points, "%d", bonus );
	while( *scan )
	{
		SurfaceBlitCharacter( zapFont, *scan++, &dPoint, 31, 31, 31, 1  );
		dPoint.x--;
	}

	dPoint.x += 6;
	SurfaceBlitCharacter( zapFont, 'P', &dPoint,  31, 31, 31, 1  );

	playerRect.y = playerRect.x = 0;
	playerRect.h = playerSurface[player]->h;
	playerRect.w = playerSurface[player]->w;

	CleanSpriteArea( player, &playerRect );
	PlayStereo( player, kSplop );

	SDLU_ReleaseSurface( playerSurface[player] );
}
