// keyselect.h
#pragma once

typedef union PlayerControls {
    struct {
        SDL_Scancode left;
        SDL_Scancode right;
        SDL_Scancode drop;
        SDL_Scancode rotate;
    };
    SDL_Scancode array[4];
} PlayerControls;

extern PlayerControls playerKeys[2];
extern const PlayerControls defaultPlayerKeys[2];

int SDLTypingFilter(const SDL_Event *event);

void StartWatchingTyping();
void StopWatchingTyping();
bool CheckTyping( char* ascii, SDL_KeyCode* sdl );
void CheckKeys();

