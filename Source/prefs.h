// prefs.h
#pragma once

void LoadPrefs( void );
void SavePrefs( void );

typedef struct
{
	unsigned char itemName[4];
	void *itemPointer;
	short size;
}
PrefList;
