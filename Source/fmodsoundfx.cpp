// soundfx.c

#include "main.h"
#include "soundfx.h"
#include "music.h"

#include <fmod.h>
#include <fmod_errors.h>

MBoolean        soundOn = true;

FMOD_SYSTEM         *fmodSystem;
static FMOD_RESULT   result;
static FMOD_SOUND   *sound[kNumSounds];
static FMOD_CHANNEL *soundChannel[3];

void InitSound( void )
{
	if( FMOD_System_Create( &fmodSystem ) != FMOD_OK )
	{
		musicOn = soundOn = false;
		return;
	}
	result = FMOD_System_Init( fmodSystem, 32, FMOD_INIT_NORMAL, NULL );

	for( int index = 0; index < kNumSounds; ++index )
	{
		result = FMOD_System_CreateStream( fmodSystem, QuickResourceName( "snd", index+128, ".wav" ), FMOD_SOFTWARE | FMOD_LOOP_OFF, NULL, &sound[index] );
		if( result != FMOD_OK )
		{
			musicOn = soundOn = false;
			return;
		}
	}
}


void PlayMono( short which )
{
	if( soundOn )
	{
		FMOD_System_PlaySound( fmodSystem, FMOD_CHANNEL_FREE, sound[which], false, &soundChannel[2]);
	}
}

void PlayStereo( short player, short which )
{
	PlayStereoFrequency( player, which, 0 );
}

void PlayStereoFrequency( short player, short which, short freq )
{
	if( soundOn )
	{
		float oldFreq;
		FMOD_System_PlaySound( fmodSystem, FMOD_CHANNEL_FREE, sound[which], true, &soundChannel[player] );
		FMOD_Channel_SetPan( soundChannel[player], 1.0f );
		FMOD_Channel_GetFrequency( soundChannel[player], &oldFreq );
		FMOD_Channel_SetFrequency( soundChannel[player], oldFreq * (16 + freq)/ 16 );
		FMOD_Channel_SetPaused( soundChannel[player], false );
	}
}

void UpdateSound()
{
	// no-op!
}
