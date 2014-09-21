// keyselect.h
#pragma once

int SDLTypingFilter(const SDL_Event *event);

void StartWatchingTyping();
void StopWatchingTyping();
bool CheckTyping( char* ascii, SDLKey* sdl );
void CheckKeys();

extern SDLKey playerKeys[2][4];
extern const SDLKey defaultPlayerKeys[2][4];
