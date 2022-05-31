// keyselect.c

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "keyselect.h"

#include "main.h"
#include "players.h"


#define DEFAULT_PLAYER_CONTROLS { \
	{ .left = SDL_SCANCODE_A, .right = SDL_SCANCODE_D, .drop = SDL_SCANCODE_S, .rotate = SDL_SCANCODE_W }, \
	{ .left = SDL_SCANCODE_LEFT, .right = SDL_SCANCODE_RIGHT, .drop = SDL_SCANCODE_DOWN, .rotate = SDL_SCANCODE_UP } \
}

PlayerControls playerKeys[2] = DEFAULT_PLAYER_CONTROLS;
const PlayerControls defaultPlayerKeys[2] = DEFAULT_PLAYER_CONTROLS;

void CheckKeys()
{
	int player;
	int arraySize;
	const unsigned char* pressedKeys;

	SDLU_PumpEvents();
	pressedKeys = SDL_GetKeyboardState( &arraySize );

	// Check for game keys
	for( player = 0; player < 2; player++ ) {
		if( pressedKeys[ playerKeys[player].left ] ) {
			hitKey[player].left++;
		} else {
			hitKey[player].left = 0;
		}

		if( pressedKeys[ playerKeys[player].right ] ) {
			hitKey[player].right++;
		} else {
			hitKey[player].right = 0;
		}

		if( pressedKeys[ playerKeys[player].drop ] ) {
			hitKey[player].drop++;
		} else {
			hitKey[player].drop = 0;
		}

		if( pressedKeys[ playerKeys[player].rotate ] ) {
			hitKey[player].rotate++;
		} else {
			hitKey[player].rotate = 0;
		}
	}

	pauseKey = pressedKeys[ SDL_SCANCODE_ESCAPE ];
}
