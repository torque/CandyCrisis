// main.h
#pragma once

#include <stdbool.h>

#include <SDL2/SDL.h>

void Initialize( void );
void RetrieveResources( void );
void Error( const char* extra );
void CenterRectOnScreen( SDL_Rect *rect, double locationX, double locationY );
bool AnyKeyIsPressed( void );
void RefreshAll( void );
void ReserveMonitor( void );
void ReleaseMonitor( void );
void MaskRect( SDL_Rect *r );
void RefreshPlayerWindow( short player );
int  Warp( void );
void WaitForRelease( void );
void QuickFadeIn( void );
void QuickFadeOut( void );
bool FileExists( const char* name );
void GoToBackground();
bool ControlKeyIsPressed( void );
bool OptionKeyIsPressed( void );
void NoPaint();
void NeedRefresh();
const char* QuickResourceName( const char* prefix, int id, const char* extension );
void WaitForRegainFocus();


#define flashyAnimation true


typedef struct
{
	short left, right, drop, rotate;
} KeyList;


#define kGridAcross 6
#define kGridDown 13

enum
{
	eNoState = 0,
	eStartMenu,
	eInGame,
	eInGameOver,
	eVictory,
	eFinished
};


enum
{
	dFatalErrorAlert = 128,
	dAbout = 130,
	dInformation,
	dSoundTest,
	dWarp,
	dOptions = 500,
	dKeySelect = 800
};

enum
{
	mBar = 128,
	mApple = 128,
	mFile,
	iAbout = 1,
	iQuit = 1
};

enum
{
	picBoard = 1000,
	picBackdrop,
	picNext,
	picVictory,
	picSelectDifficulty,
	picBoardRight,
	picBlob = 200,
	picBlobMask,
	picCharMask,
	picNumber,
	picNumberMask,
	picBlast,
	picBlastMask,
	picFont = 250,
	picHiScoreFont,
	picContinueFont,
	picBalloonFont,
	picZapFont,
	picZapOutlineFont,
	picVictoryFont,
	picBubbleFont,
	picTinyFont,
	picDashedLineFont,
	picBatsuFont,
	picTitle = 300,
	picGameStart = 302,
	picGameOver,
	picVictory1,
	picVictory2,
	picVictory3,
	picVictory4,
	picVictory5,
	picVictory6,
	picLogo = 500,
	picLogoAlpha,
	picLogoMask
};

enum
{
	errNoMonitor = 1,
	errNoDrawSprocket,
	errNoMemory,
	errNoQuickTime,
	errUnknown
};

enum
{
	winPlayer = 128,
	winBackdrop,
	winNext,
	winScore,
	winOpponent,
	winTitle,
	winVictory,
	winLevel
};

enum
{
	rightRotate = 0,
	downRotate,
	leftRotate,
	upRotate
};

enum
{
	kEmpty = 0,
	kBlob,
	kBlob2,
	kBlob3,
	kBlob4,
	kBlob5,
	kBlob6,
	kBlob7,
	kBombTop,
	kBombBottom,
	kGray,
	kLight,
	kSun
};

enum
{
	kNoSuction       = 0,
	kUp              = 1,
	kRight           = 2,
	kUpRight         = 3,
	kDown            = 4,
	kUpDown          = 5,
	kRightDown       = 6,
	kUpRightDown     = 7,
	kLeft            = 8,
	kLeftUp          = 9,
	kLeftRight       = 10,
	kLeftUpRight     = 11,
	kLeftDown        = 12,
	kLeftUpDown      = 13,
	kLeftRightDown   = 14,
	kLeftUpRightDown = 15,
	kDying           = 16,
	kSquish          = 17,
	kSquash          = 18,
	kSquish1         = 19,
	kSquish2         = 20,
	kSquish3         = 21,
	kSquish4         = 22,
	kBlinkBlob       = 23,
	kSobBlob         = 24,
	kSob2Blob        = 25,
	kFlashDarkBlob   = 26,
	kFlashBrightBlob = 27,
	kJiggle1         = 28,
	kJiggle2         = 29,
	kJiggle3         = 30,
	kJiggle4         = 31,
	kJiggle5         = 32,
	kJiggle6         = 33,
	kJiggle7         = 34,
	kJiggle8         = 35,
	kInDoubt         = 36,
	kInDeath         = 37
};

enum
{
	kNoCharring      = 0,
	kBombFuse1       = 0,
	kBombFuse2       = 1,
	kBombFuse3       = 2,
	kBlinkBomb1      = 3,
	kBlinkBomb2      = 4,
	kBlinkBomb3      = 5,
	kChar11,
	kChar31,
	kChar12,
	kChar32,
	kChar13,
	kChar33,
	kChar14,
	kChar24,
	kChar34
};

enum
{
	kDarkChar = 0xF0,
	kLightestChar = 0x00
};

enum
{
	kFlashAnimation  = 0,
	kJiggleAnimation
};

#define kBlobFrames (kFlashBrightBlob+1)

enum
{
	kGrayNoBlink = 0,
	kGrayBlink1,
	kGrayBlink2,
	kGrayBlink3,
	kSunGlow1,
	kSunGlow2,
	kSunGlow3,
	kSunGlow4,
	kSmallGray1,
	kSmallGray1b,
	kSmallGray2,
	kSmallGray2b,
	kSmallGray3,
	kSmallGray3b,
	kSmallGray4,
	kSmallGray4b,
	kSmallGray5,
	kSmallGray5b,
	kEasyGray = 25,
	kHardGray,
	kStageGray
};
#define kGrayFrames (kGrayDying4+1)

#define kFirstBlob kBlob
#define kLastBlob kBlob7
#define kBlobTypes (kLastBlob - kFirstBlob + 1)

#define pi 3.1415926535898

extern SDL_Surface* frontSurface;
extern SDL_Window *mainWindow;

extern signed char nextA[2], nextB[2], nextM[2], nextG[2], colorA[2], colorB[2],
    blobX[2], blobY[2], blobR[2], blobSpin[2], speed[2], role[2], halfway[2],
    control[2], dropping[2], magic[2], grenade[2], anim[2];
extern int chain[2];
extern long blobTime[2], startTime, endTime;
extern bool finished, pauseKey, showStartMenu;
extern signed char grid[2][kGridAcross][kGridDown], suction[2][kGridAcross][kGridDown],
    charred[2][kGridAcross][kGridDown], glow[2][kGridAcross][kGridDown];
extern SDL_Rect playerWindowZRect, playerWindowRect[2];
extern bool playerWindowVisible[2];
extern KeyList hitKey[2];
extern int backgroundID;
extern void (*DoFullRepaint)();
