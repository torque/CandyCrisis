// hiscore.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "SDLU.h"

#include "hiscore.h"

#include "blitter.h"
#include "font.h"
#include "gameticks.h"
#include "graphics.h"
#include "graymonitor.h"
#include "gworld.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "music.h"
#include "pause.h"
#include "players.h"
#include "random.h"
#include "score.h"
#include "soundfx.h"
#include "tutorial.h"

Combo defaultBest =
{
	/*bestGrid[kGridAcross][kGridDown] = */
	.grid = { { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	          { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	          { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,      1,      1,      1,      2,      2 },
	          { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty,      1,      1 },
	          { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty },
	          { kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty, kEmpty } },
	.a  = 2, // blob type of a segment
	.b  = 2, // blob type of b segment
	.m  = false,
	.g  = false,
	.lv = kTutorialLevel, // level to be displayed on high score playback.
	.x  = 1,
	.r  = upRotate,
	.player = 0,
	.value = (40*1) + (50*9),
	.name = "Tutorial"
};

Combo best = defaultBest;

Combo evenBetter;
Combo potentialCombo[2];

AutoPattern hiScorePattern[] =
{
	{ .command = kIdleTicks,          .d1 = 60, .d2 = 0, .message = NULL },
	{ .command = kBlockUntilDrop,     .d1 =  0, .d2 = 0, .message = NULL },
	{ .command = kBlockUntilComplete, .d1 =  0, .d2 = 0, .message = NULL },
	{ .command = kIdleTicks,          .d1 = 60, .d2 = 0, .message = NULL },
	{ .command = kComplete,           .d1 =  0, .d2 = 0, .message = NULL }
};

HighScore defaultScores[] = {
	{ .name = "Leviathan",  .score = 40000 },
	{ .name = "Dr. Crisis", .score = 36000 },
	{ .name = "Angel",      .score = 32000 },
	{ .name = "Spike",      .score = 28000 },
	{ .name = "Fox",        .score = 24000 },
	{ .name = "Raguel",     .score = 20000 },
	{ .name = "Kumo",       .score = 16000 },
	{ .name = "Patty",      .score = 12000 },
	{ .name = "Yuurei",     .score =  8000 },
	{ .name = "Glurp",      .score =  4000 }
};

HighScore scores[] = {
	{ .name = "Leviathan",  .score = 40000 },
	{ .name = "Dr. Crisis", .score = 36000 },
	{ .name = "Angel",      .score = 32000 },
	{ .name = "Spike",      .score = 28000 },
	{ .name = "Fox",        .score = 24000 },
	{ .name = "Raguel",     .score = 20000 },
	{ .name = "Kumo",       .score = 16000 },
	{ .name = "Patty",      .score = 12000 },
	{ .name = "Yuurei",     .score =  8000 },
	{ .name = "Glurp",      .score =  4000 }
};

char highScoreName[256];
char *highScoreText, *highScoreRank;

#define min(x,y) (((x)<(y))?(x):(y))

static void FadeScreen( SDL_Surface* hiScoreSurface, SDL_Surface* fadeSurface, int start, int end )
{
	int       skip, timer, frame, fade, color, direction, fadeStart, fadeEnd;
	SDL_Rect  destSDLRect;
	SDL_Rect  fullSDLRect = { 0, 0, 640, 480 };
	int       black;

	black = SDL_MapRGB( fadeSurface->format, 0, 0, 0 );

	if( start < end )
	{
		direction = 1;
		fadeStart = 0;
		fadeEnd = 32;
	}
	else
	{
		direction = -1;
		fadeStart = 32;
		fadeEnd = 0;
	}

	skip = 1;
	timer = MTickCount( ) + 1;
	while( timer > MTickCount() ) { }

	for( frame = start; (direction>0)? (frame <= end): (frame >= end); frame += direction )
	{
		MRect drawRect = {0, 0, 15, 640};
		timer += skip;

		for( fade = fadeStart; fade != fadeEnd; fade += direction )
		{
			color = frame + fade;
			if( color <  0 ) color = 0;
			if( color > 31 ) color = 31;

			SDLU_MRectToSDLRect( &drawRect, &destSDLRect );

			switch( color )
			{
				case 0:
					SDLU_BlitSurface( hiScoreSurface, &destSDLRect,
					                  fadeSurface,    &destSDLRect  );
					break;

				case 31:
					SDL_FillRect( fadeSurface, &destSDLRect, black );
					break;

				default:
					SurfaceBlitColorOver(  hiScoreSurface,  fadeSurface,
					                      &drawRect,       &drawRect,
					                       0, 0, 0, color );
					break;
			}

			OffsetMRect( &drawRect, 0, 15 );
		}

		SDLU_BlitFrontSurface( fadeSurface, &fullSDLRect, &fullSDLRect );

		if( timer <= MTickCount( ) )
		{
			skip = 2;
		}
		else
		{
			skip = 1;
			while( timer > MTickCount( ) )
			{
				SDLU_Yield();
			}
		}
	}
}

void ShowHiscore( void )
{
	short            count, length;
	char             myString[256];
	int              stringLength;
	SDL_Surface*     hiScoreSurface;
	SDL_Surface*     fadeSurface;
	SDL_Rect         fullSDLRect = { 0, 0, 640, 480 };
	SkittlesFontPtr  font;
	SDL_Color        anyColor;
	SDLU_Point           dPoint;
	const char*      highScores = "HIGH SCORES";
	int              r, g, b;

	if( /* the delete key is held down */ 0 ) // <-- MUST BE OBSOLETED ... ACTUALLY MUST BE FIXED, BUT I ALWAYS SEARCH FOR OBSOLETE :)
	{
		// If the user holds delete while opening the high scores,
		// clear the high score table.

		memcpy( &scores, &defaultScores, sizeof( scores ) );
		memcpy( &best,   &defaultBest,   sizeof( best   ) );
	}

	hiScoreSurface = LoadPICTAsSurface( picBackdrop + (100 * RandomBefore( kLevels )), 16 );
	fadeSurface    = SDLU_InitSurface( &fullSDLRect, 16 );

	font = GetFont( picHiScoreFont );

	SDLU_AcquireSurface( hiScoreSurface );

	SDLU_GetPixel( hiScoreSurface, RandomBefore( fullSDLRect.w ), RandomBefore( fullSDLRect.h ), &anyColor );

	anyColor.r = anyColor.r * 32 / 256;
	anyColor.g = anyColor.g * 32 / 256;
	anyColor.b = anyColor.b * 32 / 256;
	anyColor.r = min( 31, anyColor.r + 14 );
	anyColor.g = min( 31, anyColor.g + 14 );
	anyColor.b = min( 31, anyColor.b + 14 );

	dPoint.y = 20;
	dPoint.x = 225;
	for( count=0; highScores[count]; count++ )
	{
		SurfaceBlitCharacter( font, highScores[count], &dPoint, 31, 31, 31, 1 );
	}

	for( count=0; count<=9; count++ )
	{
		r = ((31 * (10-count)) + (anyColor.r * count)) / 10;
		g = ((31 * (10-count)) + (anyColor.g * count)) / 10;
		b = ((31 * (10-count)) + (anyColor.b * count)) / 10;

		dPoint.y = 75 + count * 38;
		dPoint.x = 85;

		if( count<9 )
		{
			SurfaceBlitCharacter( font, count + '1', &dPoint, r, g, b, 1 );
		}
		else
		{
			SurfaceBlitCharacter( font, '1', &dPoint, r, g, b, 1 );
			SurfaceBlitCharacter( font, '0', &dPoint, r, g, b, 1 );
		}

		SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 1 );
		SurfaceBlitCharacter( font, ' ', &dPoint, r, g, b, 1 );

		length = 0;
		while( scores[count].name[length] && dPoint.x < 430 )
		{
			SurfaceBlitCharacter( font, scores[count].name[length++], &dPoint, r, g, b, 1 );
		}


		while( dPoint.x < 450 )
		{
			SurfaceBlitCharacter( font, '.', &dPoint, r, g, b, 1 );
		}

		dPoint.x = 470;

		stringLength = sprintf( myString, "%ld", scores[count].score );
		for( length=0; length < stringLength; length++ )
		{
			SurfaceBlitCharacter( font, myString[length], &dPoint, r, g, b, 1 );
		}
	}

	SDLU_ReleaseSurface( hiScoreSurface );

	SDL_FillRect( frontSurface, &frontSurface->clip_rect, SDL_MapRGB( frontSurface->format, 0, 0, 0 ));
	SDL_Flip( frontSurface );

	FadeScreen( hiScoreSurface, fadeSurface, 31, -32 );
	do
	{
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );
	FadeScreen( hiScoreSurface, fadeSurface, -31, 32 );

	SDL_FreeSurface( hiScoreSurface );
	SDL_FreeSurface( fadeSurface );
}

void InitPotentialCombos( void )
{
	memset( &potentialCombo[0], 0, sizeof(Combo) );
	memset( &potentialCombo[1], 0, sizeof(Combo) );
	potentialCombo[0].player = 0;
	potentialCombo[1].player = 1;
}

void SubmitCombo( Combo *in )
{
	if( in->value > best.value && in->value > evenBetter.value )
	{
		PlayMono( kContinueSnd );
		memcpy( &evenBetter, in, sizeof( Combo ) );
	}
}

// Please note: This function may well be the biggest kludge in Candy Crisis.
// It relies on tons of insider knowledge. But it works.
void ShowBestCombo( void )
{
	SkittlesFontPtr font;
	const char *bestCombo = "BEST COMBO", *constScan;
	char bestInfo[256], *scan;
	SDLU_Point dPoint;
	int levelCap;

	font = GetFont( picHiScoreFont );

	StopBalloon( );
	InitGame( kAutoControl, kNobodyControl );
	scoreWindowVisible[0] = false;
	grayMonitorVisible[0] = false;

	level = best.lv;
	levelCap = kLevels;
	if( (level < 1 || level > levelCap) && level != kTutorialLevel )
	{
		memcpy( &best, &defaultBest, sizeof(best) );
		showStartMenu = true;
		return;
	}

	BeginRound( false );
	character[0].dropSpeed = 12;

	SDLU_AcquireSurface( backdropSurface );

	dPoint.y = 40;
	dPoint.x = 225;
	for( constScan = bestCombo; *constScan; constScan++ )
	{
		SurfaceBlitCharacter( font, *constScan, &dPoint, 31, 31, 31, 1 );
	}

	sprintf( bestInfo, "%s (%d points)", best.name, best.value );
	dPoint.y = 410;
	dPoint.x = 320 - (GetTextWidth( font, bestInfo ) / 2);

	for( scan = bestInfo; *scan; scan++ )
	{
		SurfaceBlitCharacter( font, *scan, &dPoint, 31, 31, 31, 1 );
	}

	SDLU_ReleaseSurface( backdropSurface );

	memcpy( grid[0], best.grid, kGridAcross * kGridDown );
	ResolveSuction( 0 );
	RedrawBoardContents( 0 );
	RefreshAll( );

	nextA[0] = best.a;
	nextB[0] = best.b;
	nextG[0] = best.g;
	nextM[0] = best.m;
	RetrieveBlobs( 0 );

	EraseSpriteBlobs( 0 );
	blobX[0] = best.x;
	blobR[0] = best.r;
	DrawSpriteBlobs( 0, kNoSuction );

	QuickFadeIn();
	blobTime[0] = animTime[0] = GameTickCount( );

	autoPattern = hiScorePattern;
	tutorialTime = 0;
}

void AddHiscore( long score )
{
	short count, item;
	char rank[50];
	char text[256];
	const char *playerName = "You", *twoPlayerNames[] = { "Player 1", "Player 2" };
	int highScoreLevel = -1;


	// Check for high score
	// (only do high scores if it's player vs CPU)

	if( players == 1 &&
	    control[0] == kPlayerControl &&
	    control[1] == kAIControl        )
	{
		for( count=0; count<=9; count++ )
		{
			if( score >= scores[count].score )
			{
				sprintf( rank, "%ld points", score );
				highScoreLevel = count;
				break;
			}
		}
	}

	// Determine player's name for best combo

	if( players == 2 ) playerName = twoPlayerNames[evenBetter.player];


	// See if both best combo AND high score

	if( evenBetter.value > best.value && highScoreLevel != -1 )
	{

		sprintf( text, "You got a high score and the best combo!" );

		highScoreText = text;
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );

		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			memcpy( &best, &evenBetter, sizeof(Combo) );
			strcpy( best.name, highScoreName );

			for( item=8; item>=highScoreLevel; item-- )
			{
				memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}

			scores[highScoreLevel].score = score;
			strcpy( scores[highScoreLevel].name, highScoreName );
		}
	}

	// See if best combo has been won

	else if( evenBetter.value > best.value )
	{
		char rank[1];
		sprintf( text, "Congratulations! %s got best combo!", playerName );
		// trying to write c-style c++ is pretty dumb.
		sprintf( rank, "" );

		highScoreText = text;
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );

		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			memcpy( &best, &evenBetter, sizeof(Combo) );
			strcpy( best.name, highScoreName );
		}
	}

	// See if high score has been won

	else if( highScoreLevel != -1 )
	{
		sprintf( text, "Congratulations! You got a high score!" );

		highScoreText = text;
		highScoreRank = rank;

		HandleDialog( kHiScoreDialog );

		if( !finished )
		{
			highScoreName[kNameLength] = '\0';

			for( item=8; item>=highScoreLevel; item-- )
			{
				memmove( &scores[item+1], &scores[item], sizeof( HighScore ) );
			}

			scores[highScoreLevel].score = score;
			strcpy( scores[highScoreLevel].name, highScoreName );
		}
	}
}
