// hiscore.h
#pragma once

#define kNameLength 40

typedef struct
{
	signed char grid[kGridAcross][kGridDown];
	signed char a, b, m, g, lv, x, r, player;
	int value;
	char name[kNameLength];
} Combo;

typedef struct
{
	char name[kNameLength];
	long score;
} HighScore;

void ShowHiscore( void );
void ShowBestCombo( void );
void AddHiscore( long score );
void SubmitCombo( Combo *in );
void InitPotentialCombos( void );

extern Combo best, potentialCombo[2];
extern HighScore scores[10];
extern char highScoreName[256];
extern char *highScoreText, *highScoreRank;
