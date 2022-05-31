// tutorial.c

#include <string.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "tutorial.h"

#include "blitter.h"
#include "control.h"
#include "font.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "opponent.h"
#include "pause.h"
#include "soundfx.h"

#include "tutorialPattern.h"

SDL_Rect        balloonRect;
SkittlesFontPtr balloonFont;
SDLU_Point      balloonPt;
char*           balloonChar;
char            balloonMsg[256];
int             balloonTime, tutorialTime;
SDL_Surface*    balloonSurface = NULL;

void InitTutorial( void )
{
	// Balloon font
	balloonFont = GetFont( picBalloonFont );

	// Balloon backbuffer
	if( balloonSurface == NULL )
	{
		SDL_Rect surfaceRect = { 0, 0, backdropSurface->w, backdropSurface->h };
		balloonSurface = SDLU_InitSurface( &surfaceRect, 16 );
	}

	// Set up auto pattern
	autoPattern = tutorialPattern;
	tutorialTime = 0;
}

void EndTutorial( void )
{
	QuickFadeOut();

	showStartMenu = true;
}

static int CalculateBalloonWidth( char *message )
{
	int maxWidth = 40;
	int currentWidth = 0;

	for( ;; )
	{
		char in = *message++;

		switch(in)
		{
			case 0:
				return (currentWidth > maxWidth)? currentWidth: maxWidth;

			case '\n':
				maxWidth = (currentWidth > maxWidth)? currentWidth: maxWidth;
				currentWidth = 0;
				break;

			default:
				currentWidth += balloonFont->width[(int)in];
				break;
		}
	}
}

static int CalculateBalloonHeight( char *message )
{
	int lines = 2;
	char *scan = message;

	while( *scan ) lines += (*scan++ == '\n');

	return lines * 20;
}

void StopBalloon( void )
{
	balloonTime = 0x7FFFFFFF;
}

void StartBalloon( const char *message )
{
	SDLU_Point  balloonTip, balloonFill;
	int         replace;
	const char *match[] = { "~~", "||", "``", "{{" };
	char       *search;
	SDL_Rect    balloonContentsRect;

	strcpy( balloonMsg, message );
	for( replace=0; replace<4; replace++ )
	{
		search = strstr( balloonMsg, match[replace] );
		if( search )
		{
			char temp[256];

			search[0] = '%';
			search[1] = 's';
			sprintf( temp, balloonMsg, SDL_GetScancodeName( playerKeys[0].array[replace] ) );
			strcpy( balloonMsg, temp );
		}
	}

	// Erase previous balloons
	SDLU_BlitFrontSurface( backdropSurface, &balloonRect, &balloonRect );

	// Draw empty balloon outline
	SDLU_AcquireSurface( balloonSurface );

	const static int right = 210, bottom = 190;
	balloonRect.w = CalculateBalloonWidth ( balloonMsg ) + 25;
	balloonRect.h = CalculateBalloonHeight( balloonMsg ) + 25;
	balloonRect.x = right - balloonRect.w;
	balloonRect.y = bottom - balloonRect.h;

	SDLU_BlitSurface( backdropSurface, &balloonRect,
	                  balloonSurface,  &balloonRect  );

	balloonContentsRect = balloonRect;
	balloonContentsRect.h -= 25;

	SurfaceGetEdges( balloonSurface, &balloonContentsRect );
	SDL_FillRect( balloonSurface, &balloonContentsRect,
	              SDL_MapRGB( balloonSurface->format, 0xFF, 0xFF, 0xFF ) );
	SurfaceCurveEdges( balloonSurface, &balloonContentsRect );

	balloonTip.y = balloonContentsRect.y + balloonContentsRect.h - 2;
	balloonTip.x = balloonContentsRect.x + balloonContentsRect.w - 40;
	balloonFill = balloonTip;

	SurfaceBlitCharacter( balloonFont, '[', &balloonFill,  0,  0,  0,  0 );
	SurfaceBlitCharacter( balloonFont, ']', &balloonTip,  31, 31, 31,  0 );

	SDLU_ReleaseSurface( balloonSurface );

	// Blit empty balloon to screen
	SDLU_BlitFrontSurface( balloonSurface, &balloonRect, &balloonRect );

	balloonPt.x = balloonRect.x + 10;
	balloonPt.y = balloonRect.y + 10;
	balloonChar = balloonMsg;
	balloonTime = GameTickCount( );

	OpponentChatter( true );
}

void UpdateBalloon( void )
{
	if( control[0] != kAutoControl ) return;
	if( GameTickCount() < balloonTime ) return;

	if( balloonChar )
	{
		char in = *balloonChar++;

		switch( in )
		{
			case 0:
				OpponentChatter( false );
				balloonChar = NULL;
				balloonTime += 120;
				break;

			case '\n':
				balloonPt.x = balloonRect.x + 10;
				balloonPt.y += 20;
				break;

			default:
				if( balloonFont->width[(int)in] > 0 )
				{
					SDLU_AcquireSurface( balloonSurface );
					SurfaceBlitCharacter( balloonFont, in, &balloonPt, 0, 0, 0, 0 );
					SDLU_ReleaseSurface( balloonSurface );

					SDLU_BlitFrontSurface( balloonSurface, &balloonRect, &balloonRect );

					balloonTime += 2;
				}
				break;
		}
	}
	else
	{
		SDLU_BlitFrontSurface( backdropSurface, &balloonRect, &balloonRect );

		StopBalloon();
	}
}
