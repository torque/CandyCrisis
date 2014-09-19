// keyselect.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "SDLU.h"

#include "keyselect.h"

#include "main.h"
#include "players.h"


SDLKey playerKeys[2][4] = {
	{ SDLK_a, SDLK_d, SDLK_x, SDLK_s },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};

const SDLKey defaultPlayerKeys[2][4] = {
	{ SDLK_a, SDLK_d, SDLK_x, SDLK_s },
	{ SDLK_LEFT, SDLK_RIGHT, SDLK_DOWN, SDLK_UP }
};


void CheckKeys()
{
	int player;
	int arraySize;
	unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyState( &arraySize );

	// Check for game keys
	for( player = 0; player < 2; player++ )
	{
		if( pressedKeys[ playerKeys[player][0] ] )
			hitKey[player].left++;
		else
			hitKey[player].left = 0;


		if( pressedKeys[ playerKeys[player][1] ] )
			hitKey[player].right++;
		else
			hitKey[player].right = 0;


		if( pressedKeys[ playerKeys[player][2] ] )
			hitKey[player].drop++;
		else
			hitKey[player].drop = 0;


		if( pressedKeys[ playerKeys[player][3] ] )
			hitKey[player].rotate++;
		else
			hitKey[player].rotate = 0;
	}

	pauseKey = pressedKeys[ SDLK_ESCAPE ];
}
