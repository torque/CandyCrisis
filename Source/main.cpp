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

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
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
signed char  nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
             blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
             control[2], dropping[2], magic[2], grenade[2], anim[2];
int          chain[2];
long         blobTime[2], startTime, endTime;
bool         finished = false, pauseKey = false, showStartMenu = true;
signed char  grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown], charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
MRect        playerWindowZRect, playerWindowRect[2];
bool         playerWindowVisible[2] = { true, true };
KeyList      hitKey[2];
int          backgroundID = -1;
SDLU_Point   blobWindow[8][2];
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

void MaskRect( MRect *r )
{
	SDL_Rect sdlRect;
	SDLU_MRectToSDLRect( r, &sdlRect );
	SDLU_BlitFrontSurface( backdropSurface, &sdlRect, &sdlRect );
}

void RefreshPlayerWindow( short player )
{
	MRect fullUpdate = {0, 0, kGridDown * kBlobVertSize, kGridAcross * kBlobHorizSize };

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
	char message[256];
	sprintf( message, "Sorry, a critical error has occurred. Please report the following error message:\n    %s", extra );
	#if WIN32
		MessageBox( NULL, message, "Candy Crisis", MB_OK );
	#else
		fprintf(stderr, "Candy Crisis: %s\n", message);
	#endif
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
	unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyState( &arraySize );

	// Only check ASCII keys. (Reason: some extended keys, like NUMLOCK or CAPSLOCK,
	// can be on all the time even if a key really isn't depressed.)
	if( arraySize > 128 ) arraySize = 128;

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
	int arraySize;
	unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyState( &arraySize );

	return pressedKeys[ SDLK_LCTRL ] || pressedKeys[ SDLK_RCTRL ];
}

bool OptionKeyIsPressed( void )
{
	int arraySize;
	unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyState( &arraySize );

	return pressedKeys[ SDLK_LALT ] || pressedKeys[ SDLK_RALT ];
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


void CenterRectOnScreen( MRect *rect, double locationX, double locationY )
{
	SDLU_Point dest = { .x = 0, .y = 0 };

	dest.x = (int)(locationX * (640 - (rect->right - rect->left)));
	dest.x &= ~3; // force last two bits to be 0? I'm not sure what this accomplishes.
	dest.y = (int)(locationY * (480 - (rect->bottom - rect->top)));

	OffsetMRect( rect, -rect->left, -rect->top );
	OffsetMRect( rect, dest.x, dest.y );
}

void ReserveMonitor( void )
{
	SDL_Surface* icon;
	SDL_Surface* mask;

	frontSurface = SDL_SetVideoMode( 640, 480, 16, SDL_HWSURFACE );

	icon = LoadPICTAsSurface( 10000, 16 );
	mask = LoadPICTAsSurface( 10001, 1 );
	SDL_WM_SetIcon( icon, (Uint8*) mask->pixels );
	SDL_FreeSurface( icon );
	SDL_FreeSurface( mask );

	SDL_ShowCursor( SDL_DISABLE );

	SDL_WM_SetCaption( "Candy Crisis", "CandyCrisis" );
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

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		Error( "SDL_Init failed" );
	}

	atexit( SDL_Quit );

	SDL_SetEventFilter( SDLU_EventFilter );
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
