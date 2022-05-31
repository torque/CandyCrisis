// level.c

#include <math.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "level.h"

#include "blitter.h"
#include "control.h"
#include "gameticks.h"
#include "graphics.h"
#include "graymonitor.h"
#include "grays.h"
#include "gworld.h"
#include "hiscore.h"
#include "keyselect.h"
#include "main.h"
#include "music.h"
#include "next.h"
#include "opponent.h"
#include "pause.h"
#include "players.h"
#include "random.h"
#include "score.h"
#include "soundfx.h"
#include "tutorial.h"
#include "tweak.h"
#include "victory.h"
#include "zap.h"

static SDL_Rect stageWindowZRect = { .x = 0, .y = 0, .w = 64, .h = 32};
static SDL_Rect stageWindowRect  = { .x = 0, .y = 0, .w = 64, .h = 32};
Character character[2];
int level, players, credits, difficulty[2] = {kHardLevel, kHardLevel};
int difficultyTicks, backdropTicks, backdropFrame;

#define kNumSplats 16
#define kIdleSplat -2
#define kFallingSplat -1
#define kTitleItems 7
#define kTitleDefaultGlow 24
#define kIncrementPerFrame 2
#define kNewBlobFrequency 10
#define kSplatType 4

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))

const  int      startSkip = 1;
static bool     shouldFullRepaint = false;
static int      startMenuTime = 0;
static int      splatState[kNumSplats], splatColor[kNumSplats], splatSide[kNumSplats];
static SDL_Rect splatBlob[kNumSplats];
static int      newBlobCounter = 0;
static int      titleGlow[kTitleItems] = { kTitleDefaultGlow, kTitleDefaultGlow, kTitleDefaultGlow, kTitleDefaultGlow, kTitleDefaultGlow, kTitleDefaultGlow, kTitleDefaultGlow };
static SDL_Rect titleRect[kTitleItems] = {
	{ .y = 155, .x = 203, .h = 52, .w = 223 }, // tutorial
	{ .y = 225, .x = 179, .h = 56, .w = 272 }, // 1p
	{ .y = 297, .x = 182, .h = 55, .w = 272 }, // 2p
	{ .y = 358, .x = 183, .h = 70, .w = 275 }, // solitaire
	{ .y = 429, .x = 280, .h = 49, .w = 110 }, // high scores
	{ .y = 433, .x = 390, .h = 44, .w =  56 }, // quit
	{ .y = 430, .x = 187, .h = 49, .w =  93 }, // controls
};

const int kCursorWidth  = 32;
const int kCursorHeight = 32;

static void InsertCursor( SDL_Point mouseHere, SDL_Surface* scratch, SDL_Surface* surface )
{
	SkittlesFontPtr cursorFont = GetFont( picFont );
	// INVESTIGATE
	// See if cursorFrontSDLRect is actually redundant.
	SDL_Rect        cursorBackSDLRect =  { .x = 0, .y = 0, .w = kCursorWidth, .h = kCursorHeight };
	SDL_Rect        cursorFrontSDLRect = { .x = 0, .y = 0, .w = kCursorWidth, .h = kCursorHeight };
	SDL_Point      mouseHereToo = mouseHere;

	cursorFrontSDLRect.x = mouseHere.x;
	cursorFrontSDLRect.y = mouseHere.y;

	SDLU_BlitSurface( surface, &cursorFrontSDLRect,
	                  scratch, &cursorBackSDLRect   );

	SDLU_AcquireSurface( surface );
	SurfaceBlitCharacter( cursorFont, '^', &mouseHere,    0,  0,  0, 0 );
	SurfaceBlitCharacter( cursorFont, '}', &mouseHereToo, 31, 31, 31, 0 );
	SDLU_ReleaseSurface( surface );
}

static void RemoveCursor( SDL_Point mouseHere, SDL_Surface* scratch, SDL_Surface* surface )
{
	SDL_Rect      cursorBackSDLRect =  { .x = 0, .y = 0, .w = kCursorWidth, .h = kCursorHeight };
	SDL_Rect      cursorFrontSDLRect = { .x = 0, .y = 0, .w = kCursorWidth, .h = kCursorHeight };

	cursorFrontSDLRect.x = mouseHere.x;
	cursorFrontSDLRect.y = mouseHere.y;

	SDLU_BlitSurface( scratch, &cursorBackSDLRect,
	                  surface, &cursorFrontSDLRect );
}

static void GameStartMenuRepaint()
{
	shouldFullRepaint = true;
}

void GameStartMenu( void )
{
	// NOTE: be wary of initializing variables here! This function can run top-to-bottom
	// multiple times in a row, thanks to "redo". Put initializations after redo.
	SDL_Surface*    gameStartSurface;
	SDL_Surface*    gameStartDrawSurface;
	SDL_Surface*    cursorBackSurface;
	SDL_Rect        backdropSDLRect = { .x = 0, .y = 0, .w = 640, .h = 480 };
	SDL_Rect        cursorBackSDLRect = { .x = 0, .y = 0, .w = kCursorWidth, .h = kCursorHeight };
	SDL_Rect        meterRect[2] = {
		{ .x =  30, .y = 360, .w = 110, .h = 20 },
		{ .x = 530, .y = 360, .w = 110, .h = 20 }
	};
	SDL_Rect        drawRect[4], chunkRect;
	int             blob, count, oldGlow, splat, chunkType, selected;
	int             skip;
	SDL_Point      mouse;
	SDL_Point      dPoint;
	unsigned long   black;
	int             currentID;
	int             combo[2], comboBright[2], missBright[2];
	SkittlesFontPtr smallFont = GetFont( picFont );
	SkittlesFontPtr tinyFont = GetFont( picTinyFont );

	const int       kLeftSide = 0, kRightSide = 1, kGlow = 2, kCursor = 3;

redo:

	combo[0] = combo[1] = 0;
	comboBright[0] = comboBright[1] = 0;
	missBright[0] = missBright[1] = 0;

	skip = 1;
	selected = -1;
	mouse.x = mouse.y = 0;

	if( finished ) return;

	if( musicSelection != 13 ) ChooseMusic( 13 );

	for( count=0; count<kTitleItems; count++ )
	{
		titleGlow[count] = kTitleDefaultGlow;
	}

	for( count=0; count<kNumSplats; count++ )
	{
		splatState[count] = kIdleSplat;
	}

	// make background surface
	gameStartSurface     = LoadPICTAsSurface( picGameStart, 16 );
	black = SDL_MapRGB( gameStartSurface->format, 0, 0, 0 );

	// make cursor backing store
	cursorBackSurface    = SDLU_InitSurface( &cursorBackSDLRect, 16 );
	SDL_FillRect( cursorBackSurface, &cursorBackSDLRect, black );

	// make drawing surface
	gameStartDrawSurface = SDLU_InitSurface( &backdropSDLRect, 16 );
	SDLU_BlitSurface( gameStartSurface,     &gameStartSurface->clip_rect,
	                  gameStartDrawSurface, &gameStartDrawSurface->clip_rect );

	// darken menu items
	for( count=0; count<kTitleItems; count++ )
	{
		SurfaceBlitColorOver(  gameStartSurface,  gameStartDrawSurface,
		                      &titleRect[count], &titleRect[count],
		                       0, 0, 0, titleGlow[count] );
	}

	SDLU_BlitFrontSurface( gameStartDrawSurface, &backdropSDLRect, &backdropSDLRect );

	WaitForRelease();

	QuickFadeIn();

	DoFullRepaint = GameStartMenuRepaint;

	startMenuTime = MTickCount( );
	while( ( selected == -1 || !SDLU_Button() ) && !finished )
	{
		startMenuTime += skip;

		// Add a new falling blob
		if( newBlobCounter == 0 )
		{
			for( blob=0; blob<kNumSplats; blob++ )
			{
				if( splatState[blob] == kIdleSplat )
				{
					splatSide[blob] = RandomBefore(2);
					splatBlob[blob].y = -24 - RandomBefore(15);
					splatBlob[blob].x = (splatSide[blob] == 0)? RandomBefore( 110 ): 640 - kBlobHorizSize - RandomBefore( 110 );
					splatBlob[blob].h = kBlobVertSize;
					splatBlob[blob].w = kBlobHorizSize;
					splatColor[blob] = ((startMenuTime >> 2) % kBlobTypes) + 1;
					splatState[blob] = kFallingSplat;

					break;
				}
			}
		}
		newBlobCounter = (newBlobCounter + 1) % kNewBlobFrequency;

		// Erase and redraw falling blobs and chunks

		SDLU_AcquireSurface( gameStartDrawSurface );

		// Take the cursor out of the scene
		RemoveCursor( mouse, cursorBackSurface, gameStartDrawSurface );
		drawRect[kCursor].y = mouse.y;
		drawRect[kCursor].x = mouse.x;
		drawRect[kCursor].h = kCursorHeight;
		drawRect[kCursor].w = kCursorWidth;

		// INVESTIGATE: does this actually do anything?
		drawRect[kLeftSide].y = drawRect[kRightSide].y = drawRect[kGlow].y =
		drawRect[kLeftSide].x = drawRect[kRightSide].x = drawRect[kGlow].x = 0;
		drawRect[kLeftSide].h = drawRect[kRightSide].h = drawRect[kGlow].h =
		drawRect[kLeftSide].w = drawRect[kRightSide].w = drawRect[kGlow].w = -1;

		// Get cursor position
		SDLU_GetMouse( &mouse );
		if( mouse.y > 460 ) {
			mouse.y = 460;
		}

		// Erase falling blobs
		for( blob=0; blob<kNumSplats; blob++ )
		{
			if( splatState[blob] == kFallingSplat )
			{
				// Copy blob rect here because it gets modified by SDL_FillRect.
				// x: 583, y: -26, w: 24, h: 24 becomes
				// x: 583, y:   0, w: 24, h:  0
				SDL_Rect blobCopy = splatBlob[blob];
				SDL_FillRect( gameStartDrawSurface, &blobCopy, black );
				SDLU_UnionRect( &drawRect[splatSide[blob]], &blobCopy, &drawRect[splatSide[blob]] );
				SDLU_OffsetRect( &splatBlob[blob], 0, startSkip * (6 + (splatBlob[blob].y + splatBlob[blob].h) / 20) );
			}
			else if( splatState[blob] >= kIncrementPerFrame )
			{
				for( splat=-3; splat<=3; splat++ )
				{
					if( splat )
					{
						chunkRect = splatBlob[blob];
						GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob]-kIncrementPerFrame, kSplatType );
						SDL_FillRect( gameStartDrawSurface, &chunkRect, black );
						SDLU_UnionRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
					}
				}

				SDL_FillRect( gameStartDrawSurface, &splatBlob[blob], black );
				SDLU_UnionRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
			}
		}

		// Draw combo meters
		for( count=0; count<2; count++ )
		{
			int bright = comboBright[count];
			int mBright = missBright[count];
			if( bright || mBright )
			{
				SDL_FillRect( gameStartDrawSurface, &meterRect[count], black );
				SDLU_UnionRect( &drawRect[count], &meterRect[count], &drawRect[count] );

				if( mBright > 1 )
				{
					dPoint.y = meterRect[count].y;
					dPoint.x = meterRect[count].x + 10;
					SurfaceBlitCharacter( smallFont, 'M', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
					SurfaceBlitCharacter( smallFont, 'I', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
					SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
					SurfaceBlitCharacter( smallFont, 'S', &dPoint, mBright, mBright >> 2, mBright >> 2, 1 );
					missBright[count]--;
				}
				else if( (combo[count] >= 2) && (bright > 1) )
				{
					char  number[16] = { 0 };
					char* scan;
					sprintf( number, "%d", combo[count] );

					dPoint.y = meterRect[count].y + 3;
					dPoint.x = meterRect[count].x;

					SurfaceBlitCharacter( tinyFont, 'C', &dPoint, bright, bright, bright, 1 );
					SurfaceBlitCharacter( tinyFont, 'O', &dPoint, bright, bright, bright, 1 );
					SurfaceBlitCharacter( tinyFont, 'M', &dPoint, bright, bright, bright, 1 );
					SurfaceBlitCharacter( tinyFont, 'B', &dPoint, bright, bright, bright, 1 );
					SurfaceBlitCharacter( tinyFont, 'O', &dPoint, bright, bright, bright, 1 );
					SurfaceBlitCharacter( tinyFont, ' ', &dPoint, bright, bright, bright, 1 );
					dPoint.y -= 3;

					for( scan = number; *scan; scan++ )
					{
						SurfaceBlitCharacter( smallFont, *scan, &dPoint, bright>>2, bright>>2, bright, 1 );
					}

					comboBright[count] -= 2;
				}
				else
				{
					comboBright[count] = 0;
				}
			}
		}

		// Redraw falling blobs
		for( blob=0; blob<kNumSplats; blob++ )
		{
			if( splatState[blob] == kFallingSplat )
			{
				// splatBlob[blob].h is kBlobVertSize
				if( splatBlob[blob].y >= 480 - kBlobVertSize )
				{
					splatBlob[blob].y = 480 - kBlobVertSize;
					splatState[blob] = 1;

					// Process combos
					if( mouse.y > 420 &&
						mouse.x >= (splatBlob[blob].x - 30) &&
						// splatBlob[blob].w is kBlobHorizSize
						mouse.x <= (splatBlob[blob].x + kBlobHorizSize + 10)    )
					{
						combo[splatSide[blob]]++;
						comboBright[splatSide[blob]] = 31;
					}
					else
					{
						if( combo[splatSide[blob]] >= 2 ) {
							missBright[splatSide[blob]] = 31;
						}
						combo[splatSide[blob]] = 0;
						comboBright[splatSide[blob]] = 0;
					}
				}
				else
				{
					SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], kNoSuction );
					SDLU_UnionRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
				}
			}

			if( splatState[blob] >= 0 && splatState[blob] <= kZapFrames )
			{
				if( splatState[blob] <= (kZapFrames - kIncrementPerFrame) )
				{
					for( splat=-3; splat<=3; splat++ )
					{
						if( splat )
						{
							chunkRect = splatBlob[blob];
							GetZapStyle( 0, &chunkRect, &splatColor[blob], &chunkType, splat, splatState[blob], kSplatType );
							SurfaceDrawSprite( &chunkRect, splatColor[blob], chunkType );
							SDLU_UnionRect( &drawRect[splatSide[blob]], &chunkRect, &drawRect[splatSide[blob]] );
						}
					}

					SurfaceDrawSprite( &splatBlob[blob], splatColor[blob], chunkType );
					SDLU_UnionRect( &drawRect[splatSide[blob]], &splatBlob[blob], &drawRect[splatSide[blob]] );
				}

				splatState[blob] += kIncrementPerFrame;
				if( splatState[blob] > kZapFrames ) splatState[blob] = kIdleSplat;
			}
		}

		SDLU_ReleaseSurface( gameStartDrawSurface );

		// Find mouse coords

		selected = -1;
		for( count=0; count<kTitleItems; count++ )
		{
			if( SDL_PointInRect( &mouse, &titleRect[count] ) )
			{
				selected = count;
				break;
			}
		}

		// update glows
		for( int titleItem = 0; titleItem < kTitleItems; ++titleItem ) {

			oldGlow = titleGlow[titleItem];

			if( selected == titleItem && titleGlow[titleItem] >= 0 ) {
				titleGlow[titleItem] -= 1;

				if( titleGlow[titleItem] < 0 ) {
					titleGlow[titleItem] = 0;
				}

			} else if( titleGlow[titleItem] < kTitleDefaultGlow ) {
				titleGlow[titleItem] += 1;

				if( titleGlow[titleItem] > kTitleDefaultGlow ) {
					titleGlow[titleItem] = kTitleDefaultGlow;
				}
			}

			if( titleGlow[titleItem] != oldGlow )
			{
				SurfaceBlitColorOver(  gameStartSurface,       gameStartDrawSurface,
				                      &titleRect[titleItem], &titleRect[titleItem],
				                       0, 0, 0, titleGlow[titleItem] );

				drawRect[kGlow] = titleRect[titleItem];
			}
		}

		// Reinsert the cursor into the scene
		InsertCursor( mouse, cursorBackSurface, gameStartDrawSurface );
		drawRect[kCursor].y = min( drawRect[kCursor].y, mouse.y );
		drawRect[kCursor].x = min( drawRect[kCursor].x, mouse.x );
		// INVESTIGATE: cursor rect probably doesn't need to change size

		// Copy down everything
		if( shouldFullRepaint )
		{
			SDLU_BlitFrontSurface( gameStartDrawSurface, &gameStartDrawSurface->clip_rect, &gameStartDrawSurface->clip_rect );
			shouldFullRepaint = false;
		}
		else
		{
			for( count=0; count<4; count++ )
			{
				// INVESTIGATE: is this related to setting rects to large
				// positive/negative values above, and if so, should it be
				// changed?
				if( drawRect[count].w > 0 )
				{
					SDLU_BlitFrontSurface( gameStartDrawSurface, &drawRect[count], &drawRect[count] );
				}
			}
		}

		// Skip frames? Or delay?
		if( startMenuTime <= MTickCount( ) )
		{
			startMenuTime = MTickCount( );
			skip = 2;
		}
		else
		{
			skip = 1;
			while( startMenuTime > MTickCount( ) )
			{
				SDLU_Yield();
			}
		}
	}

	DoFullRepaint = NoPaint;

	if( finished )
	{
		selected = 5; // quit
	}

	switch( selected )
	{
		case 0:
		case 1:
		case 2:
		case 3:
			PlayMono( kChime );
			break;
	}

	SDL_FreeSurface( gameStartSurface );
	SDL_FreeSurface( gameStartDrawSurface );
	SDL_FreeSurface( cursorBackSurface );

	QuickFadeOut();

	switch( selected )
	{
		case 0:
			InitGame( kAutoControl, kNobodyControl );
			level = kTutorialLevel;
			BeginRound( true );
			InitTutorial( );
			QuickFadeIn();
			break;

		case 1:
		case 2:
		case 3:
			{
				int player2[] = { 0, kAIControl, kPlayerControl, kNobodyControl };

				InitGame( kPlayerControl, player2[selected] );
				BeginRound( true );
				QuickFadeIn();
				break;
			}

		case 4:
			ShowHiscore();
			ShowBestCombo();
			break;

		case 5:
			finished = true;
			break;

		case 6:
			currentID = RandomBefore( kLevels ) * 100;

			DrawPICTInSurface( boardSurface[0], picBoard + currentID );
			DrawPICTInSurface( frontSurface, picBackdrop + currentID );
			SDL_UpdateWindowSurface( mainWindow );

			QuickFadeIn();
			HandleDialog( kControlsDialog );
			QuickFadeOut();
			goto redo;
	}
}

void ShowGameOverScreen( void )
{
	unsigned long timer = MTickCount() + (60*3);

	QuickFadeOut();

	DrawPICTInSurface( frontSurface, picGameOver );
	SDL_UpdateWindowSurface( mainWindow );

	QuickFadeIn();
	do
	{
		if( MTickCount() > timer ) break;
		SDLU_Yield();
	}
	while( !AnyKeyIsPressed( ) && !SDLU_Button() );
	QuickFadeOut();
}

void InitStage( void )
{
	CenterRectOnScreen( &stageWindowRect, 0.5, 0.65 );
}

void DrawStage( void )
{
	SDL_Surface* levelSurface;
	SDL_Rect     numberRect = { .y = 0, .x = kNumberHorizSize/8, .h = kNumberVertSize, .w = kNumberHorizSize };

	switch( players )
	{
		case 0:
		case 2:
			break;

		case 1:
			levelSurface = SDLU_InitSurface( &stageWindowZRect, 16 );

			SDLU_AcquireSurface( levelSurface );

			SDLU_BlitSurface( boardSurface[0], &stageWindowZRect,
			                  levelSurface,    &stageWindowZRect   );

			if( level < 10 )
			{
				SDLU_OffsetRect( &numberRect, kNumberHorizSize*3/8, 0 );
			}

			DrawCharacter( kCharacterStage,   &numberRect );
			SDLU_OffsetRect( &numberRect, kNumberHorizSize, 0 );
			DrawCharacter( kCharacterStage+1, &numberRect );

			if( level < 10 )
			{
				SDLU_OffsetRect( &numberRect, kNumberHorizSize, 0 );
				DrawCharacter( level + '0', &numberRect );
			}
			else
			{
				SDLU_OffsetRect( &numberRect, kNumberHorizSize*3/4, 0 );
				DrawCharacter( (level / 10) + '0', &numberRect );
				SDLU_OffsetRect( &numberRect, kNumberHorizSize, 0 );
				DrawCharacter( (level % 10) + '0', &numberRect );
			}

			SDLU_BlitFrontSurface( levelSurface, &stageWindowZRect, &stageWindowRect );

			SDL_FreeSurface( levelSurface );

			break;
	}
}

void InitGame( int player1, int player2 )
{
	playerWindowVisible[0] = true;
	nextWindowVisible[0] = true;
	scoreWindowVisible[0] = true;
	grayMonitorVisible[0] = true;

	if( player2 == kNobodyControl )
	{
		playerWindowVisible[1] = false;
		nextWindowVisible[1] = false;
		scoreWindowVisible[1] = false;
		grayMonitorVisible[1] = false;

		CenterRectOnScreen( &playerWindowRect[0], 0.5, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[0],  0.5, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[0],  0.5, 0.11 );
		CenterRectOnScreen( &nextWindowRect[0],   0.3, 0.25 );

		CenterRectOnScreen( &stageWindowRect,	  0.3, 0.65 );
		CenterRectOnScreen( &opponentWindowRect,  0.3, 0.5 );
	}
	else
	{
		playerWindowVisible[1] = true;
		nextWindowVisible[1] = true;
		scoreWindowVisible[1] = true;
		grayMonitorVisible[1] = true;

		CenterRectOnScreen( &playerWindowRect[0], kLeftPlayerWindowCenter, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[0],  kLeftPlayerWindowCenter, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[0],  kLeftPlayerWindowCenter, 0.11 );
		CenterRectOnScreen( &nextWindowRect[0],   0.46, 0.25 );

		CenterRectOnScreen( &playerWindowRect[1], kRightPlayerWindowCenter, 0.5  );
		CenterRectOnScreen( &scoreWindowRect[1],  kRightPlayerWindowCenter, 0.89 );
		CenterRectOnScreen( &grayMonitorRect[1],  kRightPlayerWindowCenter, 0.11 );
		CenterRectOnScreen( &nextWindowRect[1],   0.54, 0.25 );

		CenterRectOnScreen( &stageWindowRect,    0.5, 0.65 );
		CenterRectOnScreen( &opponentWindowRect, 0.5, 0.5 );
	}

	nextWindowVisible[0] = ( player1 == kAutoControl )? false: true;

	players = (player1 == kPlayerControl) + (player2 == kPlayerControl);

	if( players < 2 )
	{
		difficulty[0] = difficulty[1] = kHardLevel;
	}

	control[0] = player1;
	control[1] = player2;

	score[0] = score[1] = displayedScore[0] = displayedScore[1] = 0;
	roundStartScore[0] = roundStartScore[1] = 0;

	level = 1;
	credits = (player2 == kNobodyControl)? 1: 5;
}

bool InitCharacter( int player, int level )
{
	const Character characterList[] = {
		{ -1 }, // no zero'th character
		{  0,  3, 1, {  8,  8,  8,  8,  8,  8 }, 13, 9,  0, 25, { {     0, 0 }, {     0, 0 } }, true },
		{  1,  6, 2, { 10,  9,  8,  8,  9, 10 }, 12, 7,  1, 20, { {   223, 7 }, {     0, 0 } }, true },
		{  2,  9, 3, {  7,  7,  7, 11,  7,  7 }, 10, 6,  2, 17, { {     0, 0 }, {     0, 0 } }, false },
		{  3, 12, 4, { 11, 10,  9,  8,  7,  6 },  8, 5,  3, 13, { { 32767, 4 }, { 16912, 4 } }, false },
		{  4, 15, 0, {  5,  9, 10, 10,  9,  5 },  7, 4,  4, 10, { { 32767, 1 }, {     0, 0 } }, false },
		{  5, 17, 1, {  4,  7, 11, 11,  6,  3 },  7, 2,  5,  8, { { 14835, 8 }, {     0, 0 } }, false },
		{  6, 18, 2, {  7,  9, 10, 10,  9,  7 },  6, 4,  6,  7, { {     0, 0 }, {     0, 0 } }, false },
		{  7, 20, 3, {  5, 10, 10, 10, 10,  5 },  5, 3,  7,  5, { {  9696, 2 }, { 21151, 3 } }, false },
		{  8, 21, 4, { 11, 11, 10, 10,  9,  9 },  4, 3,  8,  5, { { 32738, 5 }, {     0, 0 } }, false },
		{  9, 22, 0, { 11,  7, 11,  7, 11,  7 },  3, 1,  9,  4, { { 32356, 5 }, { 17392, 3 } }, false },
		{ 10, 23, 1, { 11, 11, 11, 11, 11, 11 },  2, 1, 10,  2, { {  6337, 1 }, {     0, 0 } }, false },
		{ 11, 24, 2, { 11, 11, 11, 11, 11, 11 },  2, 1, 11,  2, { {    -1, 7 }, {     0, 0 } }, false },
		{ -1 }, // skip
		{ 13, 24, 1, { 11, 11, 11, 11, 11, 11 }, 10, 5,  0, 30, { {     0, 0 }, {     0, 0 } }, true }
	};

	character[player] = characterList[level];
	return (character[player].picture != -1);
}

void PrepareStageGraphics( int type )
{
	int player;

	SDL_Rect blobBoard = { .x = 0, .y = 0, .h = kGridDown * kBlobVertSize, .w = kGridAcross * kBlobHorizSize };

	backgroundID = type * 100;

	DrawPICTInSurface( boardSurface[0], picBoard + backgroundID );

	// This redraws the opponent's board surface with the image that was
	// just loaded.
	SDLU_BlitSurface( boardSurface[0], &boardSurface[0]->clip_rect,
	                  boardSurface[1], &boardSurface[1]->clip_rect  );

	DrawPICTInSurface( boardSurface[1], picBoardRight + backgroundID );

	DrawPICTInSurface( backdropSurface, picBackdrop + backgroundID );

	DrawPICTInSurface( nextSurface, picNext + backgroundID );

	for( player=0; player<=1; player++ )
	{
		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		SDLU_ReleaseSurface( playerSurface[player] );

		CleanSpriteArea( player, &blobBoard );
	}

	BeginOpponent( type );

	RedrawBoardContents( 0 );
	RedrawBoardContents( 1 );

	RefreshAll( );

	backdropTicks = MTickCount( );
	backdropFrame = 0;
}

void BeginRound( bool changeMusic )
{
	int player, count, count2;

	InitGrays( );
	InitPotentialCombos( );

	switch( players )
	{
		case 0:
		case 1:
			if( InitCharacter( 1, level ) )
			{
				score[1] = roundStartScore[1] = displayedScore[1] = 0;
				character[0] = character[1];
				character[0].zapStyle = RandomBefore(5);
			}
			else
			{
				TotalVictory( );
				return;
			}

			if( control[1] == kNobodyControl )
			{
				InitRandom( 3 );
			}
			else
			{
				InitRandom( 5 );
			}
			break;

		case 2:
			score[0] = score[1] = roundStartScore[0] = roundStartScore[1] = displayedScore[0] = displayedScore[1] = 0;

			InitRandom( 5 );

			SelectRandomLevel( );
			InitCharacter( 0, level );

			SelectRandomLevel( );
			InitCharacter( 1, level );

			character[0].hints = (difficulty[0] == kEasyLevel) || (difficulty[0] == kMediumLevel);
			character[1].hints = (difficulty[1] == kEasyLevel) || (difficulty[1] == kMediumLevel);
			break;
	}

	for( player=0; player<=1; player++ )
	{
		for( count=0; count<kGridAcross; count++ )
		{
			grays[player][count] = 0;

			for( count2=0; count2<kGridDown; count2++ )
			{
				grid[player][count][count2] = kEmpty;
				suction[player][count][count2] = kNoSuction;
				charred[player][count][count2] = kNoCharring;
				glow[player][count][count2] = false;
			}
		}

		nextA[player] = GetPiece( player );
		nextB[player] = GetPiece( player );
		nextM[player] = false;
		nextG[player] = false;

		halfway[player] = false;

		unallocatedGrays[player] = 0;
		anim[player] = 0;
		lockGrays[player] = 0;
		roundStartScore[player] = score[player];

		RedrawBoardContents(player);

		if( control[player] != kNobodyControl )
		{
			role[player] = kWaitForRetrieval;
		}
		else
		{
			role[player] = kIdlePlayer;
		}
	}

	PrepareStageGraphics( character[1].picture );
	if( changeMusic ) ChooseMusic( character[1].music );

	blobTime[0]     = blobTime[1]     =
	boredTime[0]    = boredTime[1]    =
	hintTime[0]     = hintTime[1]     =
	timeAI[0]       = timeAI[1]       =
	fadeCharTime[0] = fadeCharTime[1] =
	messageTime     = startTime       =
	blinkTime[0]    = blinkTime[1]    = GameTickCount( );

	blinkTime[1] += 60;

	if( players == 2 )
		InitDifficulty( );
}

void IncrementLevel( void )
{
	level++;
}

void SelectRandomLevel( void )
{
	level = RandomBefore( kLevels ) + 1;
}

void InitDifficulty( )
{
	SDL_Rect blobBoard = { .x = 0, .y = 0, .h = kGridDown * kBlobVertSize, .w = kGridAcross * kBlobHorizSize };
	int player;
	const int selectionRow = 5;
	int count;
	SDL_Rect blobRect;

	for( player=0; player<=1; player++ )
	{
		// Set up variables
		role[player] = kChooseDifficulty;
		colorA[player] = RandomBefore(kBlobTypes)+1;
		colorB[player] = kEmpty;
		switch( difficulty[player] )
		{
			case kEasyLevel:      blobX[player] = 1; break;
			case kMediumLevel:    blobX[player] = 2; break;
			case kHardLevel:      blobX[player] = 3; break;
			case kUltraLevel:     blobX[player] = 4; break;
		}

		blobY[player] = selectionRow;
		blobR[player] = upRotate;
		blobTime[player] = GameTickCount( ) + (60*8);
		animTime[player] = GameTickCount( );
		shadowDepth[player] = kBlobShadowDepth;
		magic[player] = false;
		grenade[player] = false;

		DrawPICTInSurface( boardSurface[player], picSelectDifficulty + backgroundID );

		SDLU_AcquireSurface( playerSurface[player] );

		SurfaceDrawBoard( player, &blobBoard );

		grid[player][0][selectionRow] = kGray;
		suction[player][0][selectionRow] = kEasyGray;
		charred[player][0][selectionRow] = kNoCharring;
		CalcBlobRect( 0, selectionRow, &blobRect );
		SurfaceDrawBlob( player, &blobRect, kGray, kEasyGray, kNoCharring );

		grid[player][kGridAcross-1][selectionRow] = kGray;
		suction[player][kGridAcross-1][selectionRow] = kHardGray;
		charred[player][kGridAcross-1][selectionRow] = kNoCharring;
		CalcBlobRect( kGridAcross-1, selectionRow, &blobRect );
		SurfaceDrawBlob( player, &blobRect, kGray, kHardGray, kNoCharring );

		CalcBlobRect( 1, selectionRow, &blobRect );
		blobRect.y -= 4; blobRect.h += 8;
		blobRect.x += 4; blobRect.w -= 8;
		for( count=1; count<=4; count++ )
		{
			DrawCharacter( count + '0', &blobRect );
			SDLU_OffsetRect( &blobRect, kBlobHorizSize, 0 );
		}

		SDLU_ReleaseSurface( playerSurface[player] );

		DrawSpriteBlobs( player, kNoSuction );
		CleanSpriteArea( player, &blobBoard );
	}
}

void ChooseDifficulty( int player )
{
	SDL_Rect blobBoard = { .x = 0, .y = 0, .h = kGridDown * kBlobVertSize, .w = kGridAcross * kBlobHorizSize };
	const int selectionRow = 5;
	const int difficultyMap[kGridAcross] = {kEasyLevel, kEasyLevel, kMediumLevel, kHardLevel, kUltraLevel, kUltraLevel};
	const int fallingSpeed[kGridAcross] = {0, 15, 9, 7, 4, 0};
	const int startGrays[kGridAcross] = {0, 0,  0, 10, 20, 0};
	const int difficultyFrame[] = { kNoSuction, blobBlinkAnimation, blobBlinkAnimation,
	                                blobJiggleAnimation, blobCryAnimation, kNoSuction };
	int oldX = blobX[player];

	PlayerControl( player );
	if( blobX[player] != oldX ) anim[player] = 0;

	UpdateTweak( player, difficultyFrame[blobX[player]] );

	if( GameTickCount( ) >= blobTime[player] )
	{
		if( player == 1 && PICTExists( picBoardRight + backgroundID ) )
		{
			DrawPICTInSurface( boardSurface[player], picBoardRight + backgroundID );
		}
		else
		{
			DrawPICTInSurface( boardSurface[player], picBoard + backgroundID );
		}

		SDLU_AcquireSurface( playerSurface[player] );
		SurfaceDrawBoard( player, &blobBoard );
		SDLU_ReleaseSurface( playerSurface[player] );

		CleanSpriteArea( player, &blobBoard );

		grid[player][0][selectionRow] = kEmpty;
		grid[player][5][selectionRow] = kEmpty;

		suction[player][0][selectionRow] = kNoSuction;
		suction[player][5][selectionRow] = kNoSuction;

		difficulty[player]          = difficultyMap[ blobX[player] ];
		character[player].dropSpeed = fallingSpeed[ blobX[player] ];
		unallocatedGrays[player] = lockGrays[player] = startGrays[blobX[player]];
		character[player].hints     = (startGrays[blobX[player]] == 0);
		role[player] = kWaitingToStart;

		PlayStereoFrequency( player, kPause, player );
	}
}

const char *gameCredits[][6] =
{
	{ "Programming", "John Stiles", "", "", "", "" },
	{ "Artwork", "Kate Davis", "Leanne Stiles", "Arnauld de la Grandiere", "Bob Frasure", "Ryan Bliss" },
	{ "Music", "Leanne Stiles", "fmod", "Lizardking", "Armadon, Explizit", "Leviathan, Nemesis" },
	{ "Music", "Jester, Pygmy", "Siren", "Sirrus", "Scaven, FC", "Spring" },
	{ "Music", "Timewalker", "Jason, Silents", "Chromatic Dragon", "Ng Pei Sin", "" },
	{ "Open Source", "gcc, mingw", "SDL", "libpng", "IJG", "zlib" },
	{ "Special Thanks", "Sam Lantinga", "Carey Lening", "modarchive.com", "digitalblasphemy.com", "" }
};

void Victory( void )
{
	SkittlesFontPtr textFont, titleFont, bubbleFont;
	SDL_Surface*    backBuffer;
	SDL_Surface*    frontBuffer;
	SDL_Point      dPoint[] ={
		{ .y = 230, .x = 340 },
		{ .y = 230, .x =  30 },
		{ .y = 230, .x =  30 },
		{ .y =  30, .x =  30 },
		{ .y =  30, .x = 340 },
		{ .y = 230, .x = 340 },
		{ .y = 230, .x =  30 }
	};
	SDL_Point      bubblePoint, textPoint, shadowPoint;
	SDL_Point      setPoint[7][6];
	SDL_Point      msgSetPoint[7][2];
	long            ticks;
	int             vertScroll, picture, weight, line, minimum;
	int             scrollDir[] = {1, -1, 1, -1, 1, -1, -1};
	int             spacing[] = {40, 19, 19, 19, 23, 19, 23 };
	const char*     text;
	SDL_Rect        fullSDLRect = { 0,   0, 640, 480 };
	SDL_Rect        highSDLRect = { 0,   0, 640, 480 };
	SDL_Rect        lowSDLRect  = { 0, 250, 640, 480 };
	SDL_Rect        backBufferSDLRect = { 0, 0, 640, 730 };
	SDL_Rect        scrollSDLRect;

	const char *messages[7][2] =
	{
		{ "Congratulations!", "" },
		{ "You've managed to vaporize all", "of the rampaging candies!" },
		{ "Your quick thinking and sharp", "reflexes have saved the day." },
		{ "", "" },
		{ "", "" },
		{ "", "" },
		{ "Thanks for playing Candy Crisis!", "" },
	};

	textFont = GetFont( picFont );
	titleFont = GetFont( picHiScoreFont );
	bubbleFont = GetFont( picBubbleFont );

	ChooseMusic( 14 );

	for( picture=0; picture<7; picture++ )
	{
		for( line=0; line<2; line++ )
		{
			msgSetPoint[picture][line].y = ((dPoint[picture].y == 230)? 100: 400) + (line * 30);
			msgSetPoint[picture][line].x = 320 - (GetTextWidth( titleFont, messages[picture][line] ) / 2);
		}

		for( line=0; line<6; line++ )
		{
			SkittlesFontPtr font;

			if( line == 0 )
			{
				font = titleFont;
				textPoint.y = 45;
			}
			else
			{
				font = textFont;
				textPoint.y = 65 + (spacing[picture] * line);
			}

			textPoint.x = (bubbleFont->width[(int)'*'] - GetTextWidth( font, gameCredits[picture][line] )) / 2;

			setPoint[picture][line].y = dPoint[picture].y + textPoint.y;
			setPoint[picture][line].x = dPoint[picture].x + textPoint.x;
		}

		minimum = 640;
		for( line=1; line<6; line++ )
		{
			if( setPoint[picture][line].x < minimum ) minimum = setPoint[picture][line].x;
		}

		for( line=1; line<6; line++ )
		{
			setPoint[picture][line].x = minimum;
		}
	}

	backBuffer  = SDLU_InitSurface( &backBufferSDLRect,  16 );
	frontBuffer = SDLU_InitSurface( &fullSDLRect, 16 );

	for( picture = 0; picture<7; picture++ )
	{
		scrollSDLRect = ( scrollDir[picture] > 0 )? highSDLRect: lowSDLRect;

		DrawPICTInSurface( backBuffer, picture + picVictory1 );

		SDLU_BlitFrontSurface( backBuffer, &scrollSDLRect, &fullSDLRect );

		QuickFadeIn();

		ticks = MTickCount();
		for( vertScroll = 0; vertScroll < 250; vertScroll++ )
		{
			SDLU_AcquireSurface( frontBuffer );

			SDLU_BlitSurface( backBuffer,  &scrollSDLRect,
			                  frontBuffer, &fullSDLRect );

			weight = vertScroll - 20;
			for( line=0; line<2; line++ )
			{
				textPoint = msgSetPoint[picture][line];
				shadowPoint.y = textPoint.y + 1;
				shadowPoint.x = textPoint.x + 1;

				text = messages[picture][line];

				while( *text && weight > 0 )
				{
					int fixedWeight = (weight > 31)? 31: weight;

					SurfaceBlitWeightedCharacter( titleFont, *text, &shadowPoint, 0,  0,  0,  fixedWeight );
					SurfaceBlitWeightedCharacter( titleFont, *text, &textPoint,   31, 31, 31, fixedWeight );
					weight--;
					text++;
				}
			}

			bubblePoint = dPoint[picture];
			weight = ( vertScroll <= 210 )? vertScroll - 10: 241 - vertScroll;
			if( weight < 0  ) weight = 0;
			if( weight > 31 ) weight = 31;

			if( weight > 0 )
			{
				SurfaceBlitWeightedCharacter( bubbleFont, '*', &bubblePoint, 31, 31, 31, (weight+1)>>1 );

				for( line=0; line<6; line++ )
				{
					SkittlesFontPtr font = (line == 0)? titleFont: textFont;

					textPoint = setPoint[picture][line];
					text = gameCredits[picture][line];

					while( *text )
					{
						SurfaceBlitWeightedCharacter( font, *text++, &textPoint, 0, 0, 0, weight );
					}
				}
			}

			SDLU_ReleaseSurface( frontBuffer );

			SDLU_BlitFrontSurface( frontBuffer, &fullSDLRect, &fullSDLRect );

			scrollSDLRect.y += scrollDir[picture];

			ticks += 4;
			do
			{
				if( SDLU_Button() ) vertScroll = 250;
				SDLU_Yield();
			}
			while( ticks >= MTickCount() );
		}

		QuickFadeOut();
	}

	SDL_FreeSurface( backBuffer  );
	SDL_FreeSurface( frontBuffer );
}

void TotalVictory( void )
{

	AddHiscore( score[0] );
	QuickFadeOut();

	DoFullRepaint = NoPaint;

	Victory( );

	showStartMenu = true;
}
