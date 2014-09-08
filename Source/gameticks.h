// gameticks.h
#pragma once

unsigned long MTickCount();
void InitGameTickCount( void );
void FreezeGameTickCount( void );
void UnfreezeGameTickCount( void );
unsigned long GameTickCount( void );
