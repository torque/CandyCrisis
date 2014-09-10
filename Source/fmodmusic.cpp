// fmodmusic.c

#include <string.h>

#include "main.h"
#include "music.h"

#include <fmod.h>
#include <fmod_errors.h>

const int kNoMusic = -1;

MBoolean musicOn = true, musicFast = false;
int musicLevel = 0, musicSelection = kNoMusic;

extern FMOD_SYSTEM *fmodSystem;
static FMOD_SOUND   *music = NULL;
static FMOD_CHANNEL *musicChannel = NULL;
static FMOD_RESULT   result;

void FastMusic( void )
{
	if( music && !musicFast )
	{
		FMOD_Sound_SetMusicSpeed( music, 1.3f );
		musicFast = true;
	}
}

void SlowMusic( void )
{
	if( music && musicFast )
	{
		FMOD_Sound_SetMusicSpeed( music, 1.0f );
		musicFast = false;
	}
}

void PauseMusic( void )
{
	if( musicSelection >= 0 && musicSelection <= kSongs )
	{
		FMOD_Channel_SetPaused( musicChannel, true );
	}
}

void ResumeMusic( void )
{
	if( musicSelection >= 0 && musicSelection <= kSongs && musicOn )
	{
		FMOD_Channel_SetPaused( musicChannel, false );
	}
}

void ChooseMusic( short which )
{
	if( musicSelection >= 0 && musicSelection <= kSongs && music )
	{
		// releasing automatically stops the song.
		result = FMOD_Sound_Release( music );
		music = NULL;
	// 	musicModule = NULL;
	}

	if( which >= 0 && which <= kSongs )
	{
		result = FMOD_System_CreateStream( fmodSystem, QuickResourceName( "mod", which+128, ".mod" ), FMOD_SOFTWARE | FMOD_LOOP_NORMAL, 0, &music );
		if( result == FMOD_OK )
		{
			result = FMOD_System_PlaySound( fmodSystem, FMOD_CHANNEL_FREE, music, !musicOn, &musicChannel );

			if( result == FMOD_OK )
			{
				musicSelection = which;
			}
		}
	}
}
