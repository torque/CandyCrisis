// level.h
#pragma once

#include <stdbool.h>

void InitGame( int player1, int player2 );
bool InitCharacter( int player, int level );
void PrepareStageGraphics( int type );
void BeginRound( bool changeMusic );
void InitDifficulty( void );
void ChooseDifficulty( int player );
void SelectRandomLevel( void );
void IncrementLevel( void );
void TotalVictory( void );
void Victory( void );
void InitStage( void );
void DrawStage( void );
void GameStartMenu( void );
void ShowGameOverScreen( void );

#define kOpponentResource 'Levl'

#define kGlows 2

#define kLevels 12
#define kTutorialLevel 14

#define kEasyLevel   50
#define kMediumLevel 70
#define kHardLevel   90
#define kUltraLevel  110


typedef struct
{
	short color;
	short time;
} Glow;

typedef struct
{
	short picture;
	short intellect;
	short zapStyle;
	short autoSetup[6];
	short speedNormal;
	short speedRush;
	short music;
	short dropSpeed;
	Glow  glow[kGlows];
	short hints;
}
Character;

enum
{
	kPlayerControl = 0,
	kAIControl,
	kNobodyControl,
	kAutoControl
};

extern Character character[2];
extern int level, players, credits, difficulty[2];
extern int difficultyTicks, backdropTicks, backdropFrame;
