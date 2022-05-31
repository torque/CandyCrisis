// pause.cpp

// All of this code is fugly. I really needed a dialog manager, but I didn't know it at the time,
// and instead I cobbled this together. It is just barely good enough to work. Fortunately it looks
// decent to the end user...

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include "SDLU.h"

#include "pause.h"

#include "blitter.h"
#include "font.h"
#include "gameticks.h"
#include "graphics.h"
#include "gworld.h"
#include "hiscore.h"
#include "keyselect.h"
#include "level.h"
#include "main.h"
#include "music.h"
#include "random.h"
#include "score.h"
#include "soundfx.h"
#include "victory.h"

const char kEscapeKey = 0x1B;

typedef struct
{
	float red, green, blue;
}
FRGBColor;

SDL_Surface* backSurface;
SDL_Surface* drawSurface;
SDL_Surface* logoSurface;
SDL_Surface* logoMaskSurface;
SDL_Surface* logoAlphaSurface;

SkittlesFontPtr smallFont, bigFont, dashedLineFont, continueFont, tinyFont, batsuFont;
FRGBColor backColor[4];
bool continueTimeOut;

static int dialogType, dialogStage, dialogTimer, dialogTarget, dialogShade, dialogItem;
static float colorWrap = 0, colorInc;
static SDL_Rect logoRect = { .x = 0, .y = 0, .h = 111, .w = 246}, lastPauseRect;
static bool dialogStageComplete;
static bool timeToRedraw = false;

// for the controls dialog
static int controlToReplace = -1;

static void ItsTimeToRedraw()
{
	timeToRedraw = true;
}

enum
{
	kTextRainbow,
	kTextBrightRainbow,
	kTextWhite,
	kTextBlueGlow,
	kTextGray,
	kTextAlmostWhite
};

static SDLU_Point DrawRainbowText( SkittlesFontPtr font, const char *line, SDLU_Point dPoint, float wave, int bright )
{
	int   length, current;
	int   r,g,b;
	float s;

	current = 0;
	length = strlen(line);

	switch( bright )
	{
			case kTextGray:
				r = g = b = 12;
				break;

			case kTextBlueGlow:
				s = sin(wave);
				r = (int)(11.0 + 15.0 * s * s);
				g = r;
				b = 31;
				break;

			case kTextWhite:
				r = g = b = 31;
				break;

			case kTextAlmostWhite:
				r = g = b = 28;
				break;

	}

	while( line[current] )
	{
		switch( bright )
		{
			case kTextBrightRainbow:
				r = (int)(26.0 + 5.0 * sin(wave                    ));
				g = (int)(26.0 + 5.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(26.0 + 5.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;

			case kTextRainbow:
				r = (int)(16.0 + 12.0 * sin(wave                    ));
				g = (int)(16.0 + 12.0 * sin(wave + ((2.*pi) * 1./3.)));
				b = (int)(16.0 + 12.0 * sin(wave + ((2.*pi) * 2./3.)));
				break;
		}

		SurfaceBlitCharacter( font, line[current], &dPoint, r, g, b, 1 );

		wave += 0.2;
		current++;
	}

	return dPoint;
}


#define kEdgeSize 8
static short edge[4][kEdgeSize][kEdgeSize];

void SurfaceGetEdges( SDL_Surface* edgeSurface, const SDL_Rect *rect )
{
	unsigned char* src[4];
	int            srcRowBytes, height, count;

	src[0] = src[1] = src[2] = src[3] = (unsigned char*) edgeSurface->pixels;
	srcRowBytes = edgeSurface->pitch;

	// INVESTIGATE: can this be simplified?
	src[0] += (srcRowBytes * rect->y) + (rect->x * 2);
	src[1] += (srcRowBytes * rect->y) + ((rect->x + rect->w - kEdgeSize) * 2);
	src[2] += (srcRowBytes * (rect->y + rect->h - kEdgeSize)) + (rect->x * 2);
	src[3] += (srcRowBytes * (rect->y + rect->h - kEdgeSize)) + ((rect->x + rect->w - kEdgeSize) * 2);

	for( count=0; count<4; count++ )
	{
		for( height=0; height<kEdgeSize; height++ )
		{
			memcpy( edge[count][height], src[count], kEdgeSize * 2 );
			src[count] += srcRowBytes;
		}
	}
}


void SurfaceCurveEdges( SDL_Surface* edgeSurface, const SDL_Rect *rect )
{
	unsigned char* src[4];
	int srcRowBytes, width, height, count;
	char edgeMap[4][kEdgeSize][kEdgeSize+1]={
		{ "      --",
		  "    -...",
		  "   -.xxX",
		  "  -.xXXX",
		  " -.xXXXX",
		  " .xXXXXX",
		  "-.xXXXXX",
		  "-.XXXXXX" },
		{ "--      ",
		  "...-    ",
		  "Xxx.-   ",
		  "XXXx.-  ",
		  "XXXXx.- ",
		  "XXXXXx. ",
		  "XXXXXx.-",
		  "XXXXXX.-" },
		{ "-.XXXXXX",
		  "-.xXXXXX",
		  " .xXXXXX",
		  " -.xXXXX",
		  "  -.xXXX",
		  "   -.xxX",
		  "    -...",
		  "      --" },
		{ "XXXXXX.-",
		  "XXXXXx.-",
		  "XXXXXx. ",
		  "XXXXx.- ",
		  "XXXx.-  ",
		  "Xxx.-   ",
		  "...-    ",
		  "--      " }
	};

	src[0] = src[1] = src[2] = src[3] = (unsigned char*) edgeSurface->pixels;
	srcRowBytes = edgeSurface->pitch;

	src[0] += (srcRowBytes * rect->y) + (rect->x * 2);
	src[1] += (srcRowBytes * rect->y) + ((rect->x + rect->w - kEdgeSize) * 2);
	src[2] += (srcRowBytes * (rect->y + rect->h - kEdgeSize)) + (rect->x * 2);
	src[3] += (srcRowBytes * (rect->y + rect->h - kEdgeSize)) + ((rect->x + rect->w - kEdgeSize) * 2);

	// Draw top/bottom border
	{
		short *srcT1 = (short*) (src[0]) + kEdgeSize;
		short *srcB1 = (short*) (src[2] + (srcRowBytes*(kEdgeSize-1))) + kEdgeSize;
		short *srcT2 = srcT1 + (srcRowBytes/2);
		short *srcB2 = srcB1 - (srcRowBytes/2);

		for( width = rect->w - (kEdgeSize * 2); width > 0; width-- )
		{
			*srcT1 = 0; srcT1++;
			*srcB1 = 0; srcB1++;
			*srcT2 = (*srcT2 >> 1) & 0x3DEF; srcT2++;
			*srcB2 = (*srcB2 >> 1) & 0x3DEF; srcB2++;
		}
	}

	// Draw left/right border
	{
		unsigned char *srcL1 = (src[0] + (srcRowBytes * kEdgeSize));
		unsigned char *srcR1 = (src[1] + (srcRowBytes * kEdgeSize)) + 2*(kEdgeSize-1);

		unsigned char *srcL2 = srcL1 + 2;
		unsigned char *srcR2 = srcR1 - 2;

		for( height = rect->h - (kEdgeSize * 2); height > 0; height-- )
		{
			*(short*)srcL1 = 0;
			*(short*)srcR1 = 0;
			*(short*)srcL2 = (*(short*)srcL2 >> 1) & 0x3DEF;
			*(short*)srcR2 = (*(short*)srcR2 >> 1) & 0x3DEF;

			srcL1 += srcRowBytes;
			srcR1 += srcRowBytes;
			srcL2 += srcRowBytes;
			srcR2 += srcRowBytes;
		}
	}

	// Draw curved edges
	for( count=0; count<4; count++ )
	{
		short *srcS = (short*) src[count];

		for( height=0; height<kEdgeSize; height++ )
		{
			for( width=0; width<kEdgeSize; width++ )
			{
				switch( edgeMap[count][height][width] )
				{
					case ' ': *srcS = edge[count][height][width]; break;
					case '.': *srcS = 0; break;
					case 'x': *srcS = (*srcS >> 1) & 0x3DEF; break;
					case '-': *srcS = (edge[count][height][width] >> 1) & 0x3DEF; break;
					case 'X': break;
				}
				srcS++;
			}
			srcS += (srcRowBytes / 2) - kEdgeSize;
		}
	}
}


#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))
#define arrsize(x) (sizeof(x)/sizeof(x[0]))

enum
{
	kOpening = 0,
	kClosing
};

static bool DrawDialogBox( bool larger, int animationType, int *target, int skip, float *colorWrap, float colorInc, SDL_Rect *pauseRect )
{
	bool animationStageComplete = false;
	SDL_Rect normalRect[2][19]  = { { { .y = 240 - 10,  .x = 320 - 30,  .h = 2 * 10,  .w = 2 * 30  },
	                                  { .y = 240 - 40,  .x = 320 - 120, .h = 2 * 40,  .w = 2 * 120 },
	                                  { .y = 240 - 60,  .x = 320 - 180, .h = 2 * 60,  .w = 2 * 180 },
	                                  { .y = 240 - 70,  .x = 320 - 210, .h = 2 * 70,  .w = 2 * 210 },
	                                  { .y = 240 - 80,  .x = 320 - 230, .h = 2 * 80,  .w = 2 * 230 },
	                                  { .y = 240 - 88,  .x = 320 - 245, .h = 2 * 88,  .w = 2 * 245 },
	                                  { .y = 240 - 95,  .x = 320 - 252, .h = 2 * 95,  .w = 2 * 252 },
	                                  { .y = 240 - 101, .x = 320 - 255, .h = 2 * 101, .w = 2 * 255 },
	                                  { .y = 240 - 106, .x = 320 - 252, .h = 2 * 106, .w = 2 * 252 },
	                                  { .y = 240 - 110, .x = 320 - 245, .h = 2 * 110, .w = 2 * 245 },
	                                  { .y = 240 - 113, .x = 320 - 238, .h = 2 * 113, .w = 2 * 238 },
	                                  { .y = 240 - 115, .x = 320 - 232, .h = 2 * 115, .w = 2 * 232 },
	                                  { .y = 240 - 116, .x = 320 - 228, .h = 2 * 116, .w = 2 * 228 },
	                                  { .y = 240 - 118, .x = 320 - 232, .h = 2 * 118, .w = 2 * 230 },
	                                  { .y = 240 - 118, .x = 320 - 238, .h = 2 * 118, .w = 2 * 232 },
	                                  { .y = 240 - 119, .x = 320 - 242, .h = 2 * 119, .w = 2 * 242 },
	                                  { .y = 240 - 119, .x = 320 - 244, .h = 2 * 119, .w = 2 * 244 },
	                                  { .y = 240 - 119, .x = 320 - 242, .h = 2 * 119, .w = 2 * 242 },
	                                  { .y = 240 - 120, .x = 320 - 240, .h = 2 * 120, .w = 2 * 240 }  },
	                                { { .y = 240 - 110, .x = 320 - 220, .h = 2 * 110, .w = 2 * 220 },
	                                  { .y = 240 - 105, .x = 320 - 210, .h = 2 * 105, .w = 2 * 210 },
	                                  { .y = 240 - 100, .x = 320 - 200, .h = 2 * 100, .w = 2 * 200 },
	                                  { .y = 240 - 95,  .x = 320 - 190, .h = 2 * 95,  .w = 2 * 190 },
	                                  { .y = 240 - 90,  .x = 320 - 180, .h = 2 * 90,  .w = 2 * 180 },
	                                  { .y = 240 - 85,  .x = 320 - 170, .h = 2 * 85,  .w = 2 * 170 },
	                                  { .y = 240 - 80,  .x = 320 - 160, .h = 2 * 80,  .w = 2 * 160 },
	                                  { .y = 240 - 75,  .x = 320 - 150, .h = 2 * 75,  .w = 2 * 150 },
	                                  { .y = 240 - 70,  .x = 320 - 140, .h = 2 * 70,  .w = 2 * 140 },
	                                  { .y = 240 - 65,  .x = 320 - 130, .h = 2 * 65,  .w = 2 * 130 },
	                                  { .y = 240 - 60,  .x = 320 - 120, .h = 2 * 60,  .w = 2 * 120 },
	                                  { .y = 240 - 55,  .x = 320 - 110, .h = 2 * 55,  .w = 2 * 110 },
	                                  { .y = 240 - 50,  .x = 320 - 100, .h = 2 * 50,  .w = 2 * 100 },
	                                  { .y = 240 - 45,  .x = 320 - 90,  .h = 2 * 45,  .w = 2 * 90  },
	                                  { .y = 240 - 40,  .x = 320 - 80,  .h = 2 * 40,  .w = 2 * 80  },
	                                  { .y = 240 - 35,  .x = 320 - 70,  .h = 2 * 35,  .w = 2 * 70  },
	                                  { .y = 240 - 30,  .x = 320 - 60,  .h = 2 * 30,  .w = 2 * 60  },
	                                  { .y = 240 - 25,  .x = 320 - 50,  .h = 2 * 25,  .w = 2 * 50  },
	                                  { .y = 240 - 20,  .x = 320 - 40,  .h = 2 * 20,  .w = 2 * 40  }  }
	                              };

	SDL_Rect largerRect[2][19]  = { { { .y = 240 - 11,  .x = 320 - 30,  .h = 2 * 11,  .w = 2 * 30  },
	                                  { .y = 240 - 44,  .x = 320 - 120, .h = 2 * 44,  .w = 2 * 120 },
	                                  { .y = 240 - 66,  .x = 320 - 180, .h = 2 * 66,  .w = 2 * 180 },
	                                  { .y = 240 - 77,  .x = 320 - 210, .h = 2 * 77,  .w = 2 * 210 },
	                                  { .y = 240 - 88,  .x = 320 - 230, .h = 2 * 88,  .w = 2 * 230 },
	                                  { .y = 240 - 97,  .x = 320 - 245, .h = 2 * 97,  .w = 2 * 245 },
	                                  { .y = 240 - 104, .x = 320 - 252, .h = 2 * 104, .w = 2 * 252 },
	                                  { .y = 240 - 111, .x = 320 - 255, .h = 2 * 111, .w = 2 * 255 },
	                                  { .y = 240 - 117, .x = 320 - 252, .h = 2 * 117, .w = 2 * 252 },
	                                  { .y = 240 - 121, .x = 320 - 245, .h = 2 * 121, .w = 2 * 245 },
	                                  { .y = 240 - 124, .x = 320 - 238, .h = 2 * 124, .w = 2 * 238 },
	                                  { .y = 240 - 126, .x = 320 - 232, .h = 2 * 126, .w = 2 * 232 },
	                                  { .y = 240 - 128, .x = 320 - 228, .h = 2 * 128, .w = 2 * 228 },
	                                  { .y = 240 - 130, .x = 320 - 232, .h = 2 * 130, .w = 2 * 230 },
	                                  { .y = 240 - 130, .x = 320 - 238, .h = 2 * 130, .w = 2 * 232 },
	                                  { .y = 240 - 131, .x = 320 - 242, .h = 2 * 131, .w = 2 * 242 },
	                                  { .y = 240 - 131, .x = 320 - 244, .h = 2 * 131, .w = 2 * 244 },
	                                  { .y = 240 - 131, .x = 320 - 242, .h = 2 * 131, .w = 2 * 242 },
	                                  { .y = 240 - 132, .x = 320 - 240, .h = 2 * 132, .w = 2 * 240 }  },
	                                { { .y = 240 - 121, .x = 320 - 220, .h = 2 * 121, .w = 2 * 220 },
	                                  { .y = 240 - 115, .x = 320 - 210, .h = 2 * 115, .w = 2 * 210 },
	                                  { .y = 240 - 110, .x = 320 - 200, .h = 2 * 110, .w = 2 * 200 },
	                                  { .y = 240 - 104, .x = 320 - 190, .h = 2 * 104, .w = 2 * 190 },
	                                  { .y = 240 - 99,  .x = 320 - 180, .h = 2 * 99,  .w = 2 * 180 },
	                                  { .y = 240 - 93,  .x = 320 - 170, .h = 2 * 93,  .w = 2 * 170 },
	                                  { .y = 240 - 88,  .x = 320 - 160, .h = 2 * 88,  .w = 2 * 160 },
	                                  { .y = 240 - 82,  .x = 320 - 150, .h = 2 * 82,  .w = 2 * 150 },
	                                  { .y = 240 - 77,  .x = 320 - 140, .h = 2 * 77,  .w = 2 * 140 },
	                                  { .y = 240 - 71,  .x = 320 - 130, .h = 2 * 71,  .w = 2 * 130 },
	                                  { .y = 240 - 66,  .x = 320 - 120, .h = 2 * 66,  .w = 2 * 120 },
	                                  { .y = 240 - 60,  .x = 320 - 110, .h = 2 * 60,  .w = 2 * 110 },
	                                  { .y = 240 - 55,  .x = 320 - 100, .h = 2 * 55,  .w = 2 * 100 },
	                                  { .y = 240 - 49,  .x = 320 - 90,  .h = 2 * 49,  .w = 2 * 90  },
	                                  { .y = 240 - 44,  .x = 320 - 80,  .h = 2 * 44,  .w = 2 * 80  },
	                                  { .y = 240 - 38,  .x = 320 - 70,  .h = 2 * 38,  .w = 2 * 70  },
	                                  { .y = 240 - 33,  .x = 320 - 60,  .h = 2 * 33,  .w = 2 * 60  },
	                                  { .y = 240 - 27,  .x = 320 - 50,  .h = 2 * 27,  .w = 2 * 50  },
	                                  { .y = 240 - 22,  .x = 320 - 40,  .h = 2 * 22,  .w = 2 * 40  }  }
	                              };

	int      colorInt, shading;
	float    colorFrac, nColorFrac;
	SDL_Rect newRect;

	if( *target > 18 )
	{
		*target = 18;
		animationStageComplete = true;
	}

	colorInt  = (int) floor( *colorWrap );
	colorFrac = *colorWrap - colorInt;

	// newRect is the current animation frame
	newRect = larger? largerRect[animationType][*target]: normalRect[animationType][*target];
	shading = ((animationType == 0) ? (*target * 24 / 18): (24 - (*target * 2 / 3)));

	{
		float r1 = backColor[colorInt      ].red, g1 = backColor[colorInt      ].green, b1 = backColor[colorInt      ].blue,
		      r2 = backColor[(colorInt+1)&3].red, g2 = backColor[(colorInt+1)&3].green, b2 = backColor[(colorInt+1)&3].blue,
		      r3 = backColor[(colorInt+2)&3].red, g3 = backColor[(colorInt+2)&3].green, b3 = backColor[(colorInt+2)&3].blue,
		      r4 = backColor[(colorInt+3)&3].red, g4 = backColor[(colorInt+3)&3].green, b4 = backColor[(colorInt+3)&3].blue;

		nColorFrac = 1 - colorFrac;

		SDLU_AcquireSurface( drawSurface );

		SurfaceBlitBlendOver(  backSurface,  drawSurface,
		                      &newRect,     &newRect,
		                      (int)((r1 * nColorFrac) + (r2 * colorFrac)),
		                      (int)((g1 * nColorFrac) + (g2 * colorFrac)),
		                      (int)((b1 * nColorFrac) + (b2 * colorFrac)),
		                      (int)((r2 * nColorFrac) + (r3 * colorFrac)),
		                      (int)((g2 * nColorFrac) + (g3 * colorFrac)),
		                      (int)((b2 * nColorFrac) + (b3 * colorFrac)),
		                      (int)((r4 * nColorFrac) + (r1 * colorFrac)),
		                      (int)((g4 * nColorFrac) + (g1 * colorFrac)),
		                      (int)((b4 * nColorFrac) + (b1 * colorFrac)),
		                      (int)((r3 * nColorFrac) + (r4 * colorFrac)),
		                      (int)((g3 * nColorFrac) + (g4 * colorFrac)),
		                      (int)((b3 * nColorFrac) + (b4 * colorFrac)),
		                       shading );

		if( pauseRect->x < newRect.x )
		{
			SDL_Rect eraseRect = *pauseRect;
			// INVESTIGATE: ugh
			// pauseRect->w -= newRect.x - pauseRect->x;
			// pauseRect->x = newRect.x;
			eraseRect.w = newRect.x - eraseRect.x;

			SDLU_BlitSurface( backSurface, &eraseRect,
			                  drawSurface, &eraseRect  );
		}

		if( (pauseRect->x + pauseRect->w) > (newRect.x + newRect.w) )
		{
			SDL_Rect eraseRect = *pauseRect;
			// pauseRect->w = newRect.w + newRect.x - pauseRect->x;
			eraseRect.w -= newRect.x - eraseRect.x;
			eraseRect.x = newRect.w + newRect.x;

			SDLU_BlitSurface( backSurface, &eraseRect,
			                  drawSurface, &eraseRect  );
		}

		if( pauseRect->y < newRect.y )
		{
			SDL_Rect eraseRect = *pauseRect;
			// pauseRect->h -= newRect.y - pauseRect->y
			// pauseRect->y = newRect.y;
			eraseRect.h = newRect.y - eraseRect.y;

			SDLU_BlitSurface( backSurface, &eraseRect,
			                  drawSurface, &eraseRect  );
		}

		if( (pauseRect->y + pauseRect->h) > (newRect.y + newRect.h) )
		{
			SDL_Rect eraseRect = *pauseRect;
			eraseRect.h -= newRect.y - eraseRect.y;
			eraseRect.y = newRect.y + newRect.h;

			SDLU_BlitSurface( backSurface, &eraseRect,
			                  drawSurface, &eraseRect  );
		}

		SDLU_ReleaseSurface( drawSurface );
	}

	*pauseRect = newRect;

	*colorWrap += colorInc * skip;
	if( *colorWrap >= 4 ) *colorWrap -= 4;

	*target += skip;

	return animationStageComplete;
}

static void DrawDialogCursor( SDL_Rect *pauseRect, int *shade )
{
	SDLU_Point p, q;

	SDLU_GetMouse( &p );

	if( p.x < pauseRect->x ) {
		p.x = pauseRect->x;
	} else if( p.x > (pauseRect->x + pauseRect->w - 5) ) {
		p.x = pauseRect->x + pauseRect->w - 5;
	}

	if( p.y < pauseRect->y ) {
		p.y = pauseRect->y;
	} else if( p.y > (pauseRect->y + pauseRect->h - 5) ) {
		p.y = pauseRect->y + pauseRect->h - 5;
	}
	q = p;

	SDLU_AcquireSurface( drawSurface );

	SurfaceBlitCharacter( smallFont, '^', &p,  0,  0,  0, 0 );
	SurfaceBlitCharacter( smallFont, '}', &q, 31, 31, 31, 0 );

	SDLU_ReleaseSurface( drawSurface );
}

static void DrawDialogLogo( SDL_Rect *pauseRect, int shade )
{
	SDL_Rect drawRect;
	int alpha;

	drawRect.x = pauseRect->x + (pauseRect->w - logoRect.w) / 2;
	drawRect.y = pauseRect->y + 14;
	drawRect.h = logoRect.h;
	drawRect.w = logoRect.w;

	SDLU_AcquireSurface( drawSurface );

	alpha = (shade > 63)? 31: (shade / 2);

	SurfaceBlitWeightedDualAlpha(  drawSurface,  logoSurface,  logoMaskSurface,  logoAlphaSurface,  drawSurface,
                                  &drawRect,    &logoRect,    &logoRect,        &logoRect,         &drawRect,
                                   alpha );

	SDLU_ReleaseSurface( drawSurface );
}


enum
{
	kNothing = -1,

// main pause screen (kEndGame is reused in continue)
	kMusic = 0, kEndGame,
	kSound,     kPauseGame,
	kControls,  kResume,
	kSecret,    kWarp,
	kSoundTest,

// continue screen
	kContinue,

// controls screen
	k1PLeft,        k2PLeft,
	k1PRight,       k2PRight,
	k1PDrop,        k2PDrop,
	k1PRotate,      k2PRotate,
	kControlsOK,    kControlsReset,
};

static void DrawContinueContents( int *item, int shade )
{
	char line[4][50] = {
		"Do you want to continue?",
		"Yes",
		"No",
		""
	};
	SDLU_Point dPoint[4] = {
		{ .y = 233, .x = 210},
		{ .y = 280, .x = 220},
		{ .y = 280, .x = 400},
		{ .y = 335, .x = 400}
	};
	SDLU_Point hPoint = { .y = 255, .x = 320};
	static int lastCountdown = 0;
	int index, countdown, fade;
	int r, g, b;

	sprintf( line[3], "%d credit%c", credits, (credits != 1)? 's': ' ' );

	SDLU_AcquireSurface( drawSurface );

	for( index=0; index<4; index++ )
	{
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade),
		                 ((index == 0)                          ||
		                 ((index == 1) && (*item == kContinue)) ||
		                 ((index == 2) && (*item == kEndGame ))    )? kTextBrightRainbow: kTextRainbow );
	}

	countdown = shade / 100;
	if( countdown < 10 )
	{
		continueTimeOut = false;

		if( (countdown != 0) && (countdown != lastCountdown) )
		{
			PlayMono( kContinueSnd );
		}
		lastCountdown = countdown;

		if( countdown < 5 )
		{
			r = (countdown * 31) / 5;
			g = 31;
		}
		else
		{
			r = 31;
			g = ((10 - countdown) * 31) / 5;
		}

		fade = shade % 100;
		if( fade > 50 ) fade = 50;
		r = ((31 * (49 - fade)) + (r * fade)) / 49;
		g = ((31 * (49 - fade)) + (g * fade)) / 49;
		b = ((31 * (49 - fade))) / 49;

		countdown = '9' - countdown;
		hPoint.x -= continueFont->width[countdown] / 2;

		for( shade = 4; shade > 0; shade-- )
		{
			SDLU_Point hP = hPoint;

			hP.x += 2 * shade;
			hP.y += 2 * shade;

			SurfaceBlitWeightedCharacter( continueFont, countdown, &hP, 0, 0, 0, 20 - 4*shade );
		}

		SurfaceBlitCharacter( continueFont, countdown, &hPoint, r, g, b, 0 );
	}
	else
	{
		continueTimeOut = true;
	}

	SDLU_ReleaseSurface( drawSurface );
}

static void DrawHiScoreContents( int *item, int shade )
{
	SDLU_Point dPoint[3] = {
		{ .y = 240, .x = 640},
		{ .y = 260, .x = 640},
		{ .y = 335, .x = 400}
	};
	SDLU_Point hPoint = { .y = 294, .x = 145};
	SDLU_Point dashedLinePoint = { .y = 320, .x = 140 };
	int    index;
	int    nameLength;
	char  *line[3], *scan;

	line[0] = highScoreText;
	line[1] = (char *)"Please enter your name and press return:";
	line[2] = highScoreRank;

	for( index=0; index<2; index++ )
	{
		scan = line[index];
		while( *scan )
			dPoint[index].x -= smallFont->width[(int)*scan++];

		dPoint[index].x /= 2;
	}

	SDLU_AcquireSurface( drawSurface );

	while( dashedLinePoint.x < 490 )
	{
		SurfaceBlitCharacter( dashedLineFont, '.', &dashedLinePoint, 0, 0, 0, 0 );
	}

	nameLength = strlen(highScoreName);
	for( index = 0; index < nameLength; index++ )
	{
		SurfaceBlitCharacter( bigFont, highScoreName[index], &hPoint, 31, 31, 31, 1 );
		if( hPoint.x >= 475 )
		{
			highScoreName[index] = '\0';
			break;
		}
	}

	index = (int)(( 1.0 + sin( MTickCount() / 7.5 ) ) * 15.0);
	SurfaceBlitCharacter( bigFont, '|', &hPoint, index, index, 31, 1 );

	for( index=0; index<3; index++ )
	{
		DrawRainbowText( smallFont, line[index], dPoint[index], (0.25 * index) + (0.075 * shade), (index != 2)? kTextBrightRainbow: kTextRainbow );
	}

	SDLU_ReleaseSurface( drawSurface );
}

static void DrawControlsContents( int *item, int shade )
{
	bool    highlight;
	SDLU_Point      dPoint;
	int         index;
	const char* controlName;
	int         r, g, b;
	const char  label[8][20] = { "1P Left",   "2P Left",
	                             "1P Right",  "2P Right",
	                             "1P Drop",   "2P Drop",
	                             "1P Rotate", "2P Rotate" };


	SDLU_AcquireSurface( drawSurface );

	for( index=0; index<8; index++ )
	{
		highlight = (index == (*item - k1PLeft));

		dPoint.y = 229 + ((index & ~1) * 13);
		dPoint.x = (index & 1)? 325: 130;
		DrawRainbowText( smallFont, label[index], dPoint, (0.25 * index) + (0.075 * shade), highlight? kTextBrightRainbow: kTextRainbow );

		dPoint.y = 245 + ((index & ~1) * 13);
		dPoint.x = (index & 1)? 420: 225;
		r = (int)(highlight? 31.0: 0.0);
		g = b = (int)(highlight? 31.0 - (11.0 * (sin(shade * 0.2) + 1.0)): 0.0);
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );
		SurfaceBlitCharacter( dashedLineFont, '.', &dPoint, r, g, b, 0 );  // 80 pixels across

		controlName = SDL_GetKeyName( playerKeys[index & 1][index >> 1] );
		if( controlName == NULL ) controlName = "{";

		dPoint.y = 231 + ((index & ~1) * 13);
		dPoint.x = (index & 1)? 460: 265;
		dPoint.x -= GetTextWidth( tinyFont, controlName ) / 2;
		DrawRainbowText( tinyFont, controlName, dPoint, (0.1 * shade), (controlToReplace == index)? kTextBlueGlow: kTextWhite );
	}

	dPoint.x = 200;
	dPoint.y = 340;
	DrawRainbowText( smallFont, "{ OK", dPoint, 8.0 + (0.075 * shade), (*item == kControlsOK)? kTextBrightRainbow: kTextRainbow );

	dPoint.x = 365;
	dPoint.y = 340;
	DrawRainbowText( smallFont, "{ Reset", dPoint, 8.25 + (0.075 * shade), (*item == kControlsReset)? kTextBrightRainbow: kTextRainbow );

	SDLU_ReleaseSurface( drawSurface );
}

static void DrawPauseContents( int *item, int shade )
{
	SDLU_Point dPoint;
	int itemCount = 6;
	int index;
	const char *line[7] = { "{ Music [",           "{ End Game",
	                        "{ Sound [",           "{ Hide Game",
	                        "{ Controls",        "{ Resume" };


	if( level == kTutorialLevel ) line[1] = "{ Skip Tutorial";

	if( !musicOn ) line[0] = "{ Music ]";
	if( !soundOn ) line[2] = "{ Sound ]";

	SDLU_AcquireSurface( drawSurface );

	for( index=0; index<itemCount; index++ )
	{
		dPoint.x = (index & 1)? 340: 180;
		dPoint.y = 240 + ((index & ~1) * 15);

		DrawRainbowText( smallFont, line[index], dPoint, (0.25 * index) + (0.075 * shade), (*item == index)? kTextBrightRainbow: kTextRainbow );
	}

	SDLU_ReleaseSurface( drawSurface );
}

static bool ContinueSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	SDL_Rect yes = { .y = 280, .x = 220, .h = 20, .w = 40 };
	SDL_Rect no  = { .y = 280, .x = 400, .h = 20, .w = 40 };
	SDLU_Point p;

	if( continueTimeOut )
	{
		*item = kEndGame;
		return true;
	}

	if( inKey == kEscapeKey )
	{
		*item = kContinue;
		return true;
	}

	SDLU_GetMouse( &p );

	     if( SDLU_PointInRect( p, &yes ) ) *item = kContinue;
	else if( SDLU_PointInRect( p, &no  ) ) *item = kEndGame;
	else *item = kNothing;

	return( SDLU_Button( ) && (*item != kNothing) );
}

static bool HiScoreSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	int nameLength;

	nameLength = strlen(highScoreName);

	switch( inSDLKey ) {
		case SDLK_RETURN:
			if( nameLength > 0 )
			{
				*item = kResume;
				PlayMono( kSquishy );
				return true;
			}
			else
			{
				PlayMono( kClick );
			}
			break;

		case SDLK_BACKSPACE:
			if( nameLength > 0 )
			{
				highScoreName[ nameLength-1 ] = '\0';
				PlayMono( kClick );
			}
			break;

		default:
			if( bigFont->width[inKey] != 0 )
			{
				highScoreName[ nameLength++ ] = inKey;
				highScoreName[ nameLength   ] = '\0';
				PlayMono( kPlace );
			}
			break;
	}

	*item = kNothing;
	return false;
}


static bool ControlsSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	SDLU_Point  p;
	SDL_Rect    dRect;
	int         index;
	static bool lastDown = false;
	bool        down;
	SDL_Rect    okRect = { .y = 340, .x = 200, .h = 20, .w = 55 };
	SDL_Rect    resetRect = { .y = 340, .x = 365, .h = 20, .w = 85 };
	int         returnValue = 0;

	*item = kNothing;

	down = SDLU_Button();
	SDLU_GetMouse( &p );

	if( SDLU_PointInRect( p, &okRect ) )
	{
		*item = kControlsOK;
		if( down )
		{
			PlayMono( kClick );
			returnValue = 1;
			controlToReplace = -1;
		}
	}
	else if( SDLU_PointInRect( p, &resetRect ) )
	{
		*item = kControlsReset;
		if( down && !lastDown )
		{
			PlayMono( kClick );
			memcpy( playerKeys, defaultPlayerKeys, sizeof(playerKeys) );
		}
	}
	else
	{
		for( index=0; index<8; index++ )
		{
			dRect.y = 229 + ((index & ~1) * 13);
			dRect.x = (index & 1)? 325: 130;
			dRect.h = 24;
			dRect.w = 175;

			if( SDLU_PointInRect( p, &dRect ) )
			{
				*item = k1PLeft + index;
				if( down && !lastDown && !AnyKeyIsPressed() )
				{
					controlToReplace = (controlToReplace == index)? -1: index;
				}
				break;
			}
		}
	}

	if( inSDLKey != 0 && controlToReplace != -1 )
	{
		playerKeys[controlToReplace & 1][controlToReplace >> 1] = inSDLKey;
		controlToReplace = -1;
	}

	lastDown = down;

	return returnValue;
}


static bool PauseSelected( int *item, unsigned char inKey, SDLKey inSDLKey )
{
	SDL_Rect targetRect[] =
	{
		{ .y = 240, .x = 180, .h = 20, .w = 140 },
		{ .y = 240, .x = 340, .h = 20, .w = 140 },
		{ .y = 270, .x = 180, .h = 20, .w = 140 },
		{ .y = 270, .x = 340, .h = 20, .w = 140 },
		{ .y = 300, .x = 180, .h = 20, .w = 140 },
		{ .y = 300, .x = 340, .h = 20, .w = 140 },
		{ .y = 330, .x = 180, .h = 20, .w = 140 },
		{ .y = 120, .x = 550, .h = 10, .w = 10 }
	};

	static bool lastDown = false;
	int trigger;
	int index;
	SDLU_Point p;

	SDLU_GetMouse( &p );

	trigger = SDLU_Button();
	if( inKey == kEscapeKey )
	{
		*item = kResume;
		trigger = true;
	}
	else
	{
		*item = kNothing;
		for( index=0; index<arrsize(targetRect); index++ )
		{
			if( SDLU_PointInRect( p, &targetRect[index] ) )
			{
				*item = index;
			}
		}
	}

	if( trigger )
	{
		if( !lastDown )
		{
			lastDown = true;

			switch( *item )
			{
				case kSound:     PlayMono( kClick ); soundOn = !soundOn; PlayMono( kClick );     return false;
				case kMusic:     PlayMono( kClick ); musicOn = !musicOn;                         return false;
				case kEndGame:   PlayMono( kClick );                                             return true;
				case kResume:    PlayMono( kClick );                                             return true;

				case kPauseGame:
					PlayMono( kClick );
					SDL_WM_IconifyWindow();
                    WaitForRegainFocus();
					ItsTimeToRedraw();
					return false;

				case kControls:
					PlayMono( kClick );
					return true;

				case kSecret:
					if( ControlKeyIsPressed( ) )
					{
						*item = kWarp;
						level = Warp( );
						return true;
					}
					else if( OptionKeyIsPressed( ) )
					{
						//SoundTest( );
						ItsTimeToRedraw();
					}
					return false;
			}
		}
	}
	else
	{
		lastDown = false;
	}

	return false;
}

void HandleDialog( int type )
{
	const float    lighten[4] = { 12.0f, 6.0f, 1.0f, 6.0f };
	const SDL_Rect boardWorldZRect = { .y = 0, .x = 0, .h = kBlobVertSize * (kGridDown-1), .w = kBlobHorizSize * kGridAcross };
	SDL_Rect       fullSDLRect = { .x = 0, .y = 0, .w = 640, .h = 480 };
	int            skip = 1;
	int            count;
	char           inASCII;
	SDLKey         inSDLKey;
	SDL_Rect       pauseRect, joinRect;

	// Clear state
	controlToReplace = -1;

	// Remember dialog info
	dialogType = type;
	dialogStage = kOpening;
	colorWrap = 0;
	colorInc = (RandomBefore(250) + 250.0) / 10000.0;

	smallFont      = GetFont( picFont );
	tinyFont       = GetFont( picTinyFont );
	bigFont        = GetFont( picHiScoreFont );
	dashedLineFont = GetFont( picDashedLineFont );
	continueFont   = GetFont( picContinueFont );
	batsuFont      = GetFont( picBatsuFont );

	// Pick some colors to animate.
	for( count=0; count<4; count++ )
	{
		SDL_Color inColor;

		SDLU_GetPixel( boardSurface[0], RandomBefore( boardWorldZRect.w ), RandomBefore( boardWorldZRect.h ), &inColor );

		backColor[count].red   = inColor.r * (32.0f / 256.0f);
		backColor[count].green = inColor.g * (32.0f / 256.0f);
		backColor[count].blue  = inColor.b * (32.0f / 256.0f);

		backColor[count].red   = min( 31.0f, backColor[count].red   + lighten[count] );
		backColor[count].green = min( 31.0f, backColor[count].green + lighten[count] );
		backColor[count].blue  = min( 31.0f, backColor[count].blue  + lighten[count] );
	}

	// Get some graphics that we're going to need
	logoSurface      = LoadPICTAsSurface( picLogo, 16 );
	logoAlphaSurface = LoadPICTAsSurface( picLogoAlpha, 16 );
	logoMaskSurface  = LoadPICTAsSurface( picLogoMask, 1 );

	// Get a copy of the current game window contents
	backSurface      = SDLU_InitSurface( &fullSDLRect, 16 );

	SDLU_BlitSurface( frontSurface, &frontSurface->clip_rect,
	                  backSurface,  &backSurface->clip_rect );

	drawSurface      = SDLU_InitSurface( &fullSDLRect, 16 );

	SDLU_BlitSurface( backSurface, &backSurface->clip_rect,
	                  drawSurface, &drawSurface->clip_rect  );

	PlayMono( kWhomp );
	dialogTimer = MTickCount();
	dialogTarget = 0;
	dialogShade = 0;
	dialogStageComplete = false;
	dialogItem = kNothing;
	// INVESTIGATE: this hack looks familiar
	lastPauseRect.y = lastPauseRect.x = 9999;
	lastPauseRect.h = lastPauseRect.w = -1;

	SDLU_StartWatchingTyping();

	DoFullRepaint = ItsTimeToRedraw;

	while( ((dialogStage != kClosing) || !dialogStageComplete) && !finished )
	{
		dialogTimer += skip;

		// Check mouse and keyboard
		SDLU_CheckTyping( &inASCII, &inSDLKey );

		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			bool (*DialogSelected[kNumDialogs])( int *item, unsigned char inKey, SDLKey inSDLKey ) =
			{
				PauseSelected,
				HiScoreSelected,
				ContinueSelected,
				ControlsSelected
			};

			if( DialogSelected[dialogType]( &dialogItem, inASCII, inSDLKey ) )
			{
				dialogStage = kClosing;
				dialogTarget = 0;
			}
		}

		// Do animation ...
		{
			const bool dialogIsLarge[kNumDialogs] = { false, false, false, true };

			pauseRect = lastPauseRect;
			dialogStageComplete = DrawDialogBox( dialogIsLarge[dialogType], dialogStage, &dialogTarget, skip, &colorWrap, colorInc, &pauseRect );
			SurfaceGetEdges( backSurface, &pauseRect );
		}

		if( (dialogStage == kOpening) && dialogStageComplete )
		{
			void (*DialogDraw[kNumDialogs])( int *item, int shade ) =
			{
				DrawPauseContents,
				DrawHiScoreContents,
				DrawContinueContents,
				DrawControlsContents
			};

			// Refresh screen if necessary
			if( timeToRedraw )
			{
				SDLU_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );
				timeToRedraw = false;
			}

			// ... and fade in the logo

			dialogShade += skip;

			DrawDialogLogo( &pauseRect, dialogShade );

			// ... and animation is complete so add content
			DialogDraw[dialogType]( &dialogItem, dialogShade );

			// ... and cursor
			DrawDialogCursor( &pauseRect, &dialogShade );
		}

		SurfaceCurveEdges( drawSurface, &pauseRect );

		// Draw new animation on screen
		SDLU_UnionRect( &lastPauseRect, &pauseRect, &joinRect );
		SDLU_BlitFrontSurface( drawSurface, &joinRect, &joinRect );

		lastPauseRect = pauseRect;

		// Wait for next frame
		if( dialogTimer <= MTickCount( ) )
		{
			dialogTimer = MTickCount( );
			skip = 2;
		}
		else
		{
			skip = 1;
			while( dialogTimer > MTickCount( ) )
			{
				SDLU_Yield();
			}
		}
	}

	DoFullRepaint = NoPaint;

	SDLU_StopWatchingTyping();

	// Bring back previous screen
	SDLU_BlitFrontSurface( backSurface, &fullSDLRect, &fullSDLRect );

	// Dispose the GWorlds and fonts we used
	SDL_FreeSurface( backSurface );
	SDL_FreeSurface( drawSurface );
	SDL_FreeSurface( logoSurface );
	SDL_FreeSurface( logoAlphaSurface );
	SDL_FreeSurface( logoMaskSurface );

	switch( dialogItem )
	{
		case kControls:
			HandleDialog( kControlsDialog );
			HandleDialog( kPauseDialog );
			break;

		case kEndGame:
			ChooseMusic( -1 );
			AddHiscore( score[0] );
			if( players == 1 )
			{
				ShowGameOverScreen( );
			}
			else
			{
				QuickFadeOut();
			}

			showStartMenu = true;
			break;

		case kContinue:
			displayedScore[0] = score[0] = roundStartScore[0];
			ShowScore( 0 );
			BeginRound( true );
			break;

		case kWarp:
		{
			int newLevel = level;

			InitGame( OptionKeyIsPressed()? kAIControl: kPlayerControl, kAIControl ); // this clears "level" ...
			level = newLevel;                                                         // so we need to set "level" afterwards
			BeginRound( true );
			break;
		}
	}
}
