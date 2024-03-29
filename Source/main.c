// main.c

#if OSXBUNDLE && !__APPLE__
#error You can't build an OS X bundle if you aren't on OS X.
#endif

#if _WIN32
#include <windows.h>
#include <io.h> // for _chdir
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDLU.h"

#include "main.h"

#include "blitter.h"
#include "control.h"
#include "gameticks.h"
#include "graphics.h"
#include "graymonitor.h"
#include "grays.h"
#include "gworld.h"
#include "hiscore.h"
#include "keyselect.h"
#include "level.h"
#include "music.h"
#include "next.h"
#include "opponent.h"
#include "pause.h"
#include "players.h"
#include "prefs.h"
#include "random.h"
#include "score.h"
#include "soundfx.h"
#include "tutorial.h"
#include "tweak.h"
#include "victory.h"
#include "zap.h"


SDL_Surface* frontSurface;
SDL_Window *mainWindow;
signed char  nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
             blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
             control[2], dropping[2], magic[2], grenade[2], anim[2];
int          chain[2];
long         blobTime[2], startTime, endTime;
bool         finished = false, pauseKey = false, showStartMenu = true;
signed char  grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown], charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
SDL_Rect     playerWindowZRect, playerWindowRect[2];
bool         playerWindowVisible[2] = { true, true };
KeyList      hitKey[2];
int          backgroundID = -1;
SDL_Point   blobWindow[8][2];
void         (*DoFullRepaint)() = NoPaint;
bool         needsRefresh = false;

static char  candyCrisisResources[512];

int main(int argc, char *argv[])
{

	Initialize( );
	LoadPrefs( );

	ReserveMonitor( );
	ShowTitle( );

	ChooseMusic( 13 );

	while( !finished )
	{
		if( showStartMenu )
		{
			GameStartMenu( );
			showStartMenu = false;
		}

		if( !finished )
		{
			DoFullRepaint = NeedRefresh;
			CheckKeys( );
			HandlePlayers( );
			UpdateOpponent( );
			UpdateBalloon( );
			UpdateSound( );
			DoFullRepaint = NoPaint;

			if( needsRefresh )
			{
				RefreshAll();
				needsRefresh = false;
			}

			if( !showStartMenu && pauseKey )
			{
				FreezeGameTickCount( );
				PauseMusic( );
				MaskRect( &playerWindowRect[0] );
				MaskRect( &playerWindowRect[1] );
				WaitForRelease( );

				HandleDialog( kPauseDialog );

				WaitForRelease( );
				RefreshPlayerWindow( 0 );
				RefreshPlayerWindow( 1 );
				ResumeMusic( );
				UnfreezeGameTickCount( );
			}
		}
	}

	SavePrefs( );
	ReleaseMonitor( );

	return 0;
}

void NoPaint( void )
{
}

void MaskRect( SDL_Rect *r )
{
	SDLU_BlitFrontSurface( backdropSurface, r, r );
}

void RefreshPlayerWindow( short player )
{
	SDL_Rect fullUpdate = { .x = 0, .y = 0, .h = kGridDown * kBlobVertSize, .w = kGridAcross * kBlobHorizSize };

	if( control[player] == kNobodyControl )
	{
		MaskRect( &playerWindowRect[player] );
	}
	else
	{
		SetUpdateRect( player, &fullUpdate );
		UpdatePlayerWindow( player );
	}
}

void NeedRefresh()
{
	needsRefresh = true;
}

void RefreshAll( void )
{
	DrawBackdrop( );

	ShowGrayMonitor( 0 );
	ShowGrayMonitor( 1 );

	RefreshNext( 0 );
	RefreshNext( 1 );

	RefreshPlayerWindow( 0 );
	RefreshPlayerWindow( 1 );

	DrawFrozenOpponent( );
	DrawStage( );

	ShowScore( 0 );
	ShowScore( 1 );
}

void Error( const char* extra )
{
	// MessageBox( NULL, message, "Candy Crisis", MB_OK );
	fprintf(stderr, "Bye: %s\n", extra);
	exit(0);
}

void WaitForRelease( void )
{
	do
	{
		SDLU_Yield();
	}
	while( AnyKeyIsPressed( ) || SDLU_Button() );
}

bool AnyKeyIsPressed( void )
{
	int index;
	int arraySize;
	const unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyboardState( &arraySize );

	// NUMLOCK, CAPSLOCK and friends are available as SDLK_[KEY] values
	// 300 and up.
	if( arraySize > 300 ) arraySize = 300;

	for( index = 0; index < arraySize; index++ )
	{
		if( pressedKeys[index] )
		{
			return true;
		}
	}

	return false;
}

bool ControlKeyIsPressed( void )
{
	SDLU_PumpEvents();
	SDL_Keymod mods = SDL_GetModState();

	return (bool)(mods & KMOD_CTRL);
}

bool OptionKeyIsPressed( void )
{
	SDLU_PumpEvents();
	SDL_Keymod mods = SDL_GetModState();

	return (bool)(mods & KMOD_ALT);
}

void RetrieveResources( void )
{
	InitSound( );

	InitBackdrop( );

	GetBlobGraphics( );

	InitNext( );

	InitScore( );

	InitGrayMonitors( );

	InitOpponent( );

	InitStage( );   // must run after backdrop window is open
	InitGameTickCount( );

	InitPlayers( ); // must run after backdrop window is open
	InitFont( );
	InitZapStyle( );// must run after fonts are inited


	InitBlitter( ); // must run after player windows are open
	InitPlayerWorlds( );

	InitVictory( );	// must run after fonts are inited
	InitTweak( );
}

void CenterRectOnScreen( SDL_Rect *rect, double locationX, double locationY )
{
	SDL_Point dest = { .x = 0, .y = 0 };

	dest.x = (int)(locationX * (640 - (rect->w)));
	dest.x &= ~3; // floor x position to mod4 for some reason.
	dest.y = (int)(locationY * (480 - (rect->h)));

	SDLU_OffsetRect( rect, -rect->x, -rect->y );
	SDLU_OffsetRect( rect, dest.x, dest.y );
}

void ReserveMonitor( void )
{

	// frontSurface = SDL_SetVideoMode( 640, 480, 16, SDL_HWSURFACE | SDL_DOUBLEBUF );
	mainWindow = SDL_CreateWindow("Candy Crisis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
	frontSurface = SDL_GetWindowSurface( mainWindow );

#if !OSXBUNDLE
	SDL_Surface* icon;
	// SDL_Surface* mask;
	icon = LoadPICTAsSurface( 10000, 16 );
	// mask = LoadPICTAsSurface( 10001, 1 );
	// TODO: need to blit mask onto icon, or actually modify the icon files to
	// be pre-masked with alpha.
	SDL_SetWindowIcon( mainWindow, icon );
	SDL_FreeSurface( icon );
	// SDL_FreeSurface( mask );
#endif

	SDL_ShowCursor( SDL_DISABLE );
}

void ReleaseMonitor( void )
{
	// frontSurface is released by SDL_Quit... we are not supposed to kill it
}

int Warp( void )
{
	return 8;
}

const char* QuickResourceName( const char* prefix, int id, const char* extension )
{
	static char name[512];
	if( id )
	{
		sprintf( name, "%s%s_%d%s", candyCrisisResources, prefix, id, extension );
	}
	else
	{
		sprintf( name, "%s%s%s", candyCrisisResources, prefix, extension );
	}

	return name;
}

void Initialize( void )
{
#if _WIN32
    HMODULE module;
    char    name[MAX_PATH+1], *lastBackslash;

    module = GetModuleHandle( NULL );
    GetModuleFileName( module, name, MAX_PATH );
    lastBackslash = strrchr( name, '\\' );
    if( lastBackslash != NULL )
    {
        *lastBackslash = '\0';
        strcpy( candyCrisisResources, name );
        strcat( candyCrisisResources, "\\CandyCrisisResources\\" );
    }
#elif OSXBUNDLE
	strcpy( candyCrisisResources, "../Resources/" );
#else
	strcpy( candyCrisisResources, "CandyCrisisResources/" );
#endif

	if( SDL_Init( SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO ) < 0 )
	{
		Error( "SDL_Init failed" );
	}

	atexit( SDL_Quit );

	SDL_SetEventFilter( SDLU_EventFilter, NULL );
}

void QuickFadeIn( void )
{
}

void QuickFadeOut( void )
{
}

bool FileExists( const char* name )
{
	FILE* f = fopen( name, "rb" );
	if( f == NULL )
	{
		return false;
	}

	fclose( f );
	return true;
}


void WaitForRegainFocus()
{
    do
    {
        SDLU_PumpEvents();
        SDL_Delay(50);
    }
    while( !SDLU_IsForeground() );

	DoFullRepaint();
}
