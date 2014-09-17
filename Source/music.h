// music.h
#pragma once

void PauseMusic( void );
void ResumeMusic( void );
void FastMusic( void );
void SlowMusic( void );
void ChooseMusic( short which );

#define kSongs 14

extern bool musicOn;
extern int  musicSelection;

