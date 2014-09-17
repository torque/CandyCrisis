// font.c

#include <SDL/SDL.h>
#include "SDLU.h"

#include "main.h"
#include "font.h"
#include "gworld.h"


#define kNumFonts (picBatsuFont-picFont+1)

static SkittlesFont s_font[kNumFonts] = {{0}};


static SkittlesFontPtr LoadFont( SkittlesFontPtr font, int pictID, const char *letterMap )
{
	unsigned char* lastLine;
	unsigned char  white;
	MBoolean       success = false;
	int            start, across, skip;
	SDL_Surface*   temporarySurface;
	SDL_Rect       sdlRect;

	temporarySurface = LoadPICTAsSurface( pictID, 8 );

	if( temporarySurface )
	{
		sdlRect.x = 0;
		sdlRect.y = 0;
		sdlRect.h = temporarySurface->h;
		sdlRect.w = temporarySurface->w;

		font->surface = SDLU_InitSurface( &sdlRect, 8 );

		SDLU_BlitSurface(  temporarySurface, &temporarySurface->clip_rect,
		                   font->surface,    &temporarySurface->clip_rect  );

		SDL_FreeSurface( temporarySurface );

		white    = SDL_MapRGB( font->surface->format, 0xFF, 0xFF, 0xFF );
		lastLine = (unsigned char*) font->surface->pixels + (font->surface->pitch * (font->surface->h - 1));
		across   = 0;

		// Measure empty space between character breaks
		while( lastLine[across] == white ) across++;
		skip = across;

		success = true;

		// Measure character starts and widths
		while( *letterMap )
		{
			while( lastLine[across] != white ) across++;
			if( across > font->surface->pitch )
			{
				success = false;
				break;
			}

			start = across;
			font->across[(int)*letterMap] = across + (skip/2);

			while( lastLine[across] == white ) across++;
			font->width [(int)*letterMap] = across - start - skip;

			letterMap++;
		}
	}

	if( success )
	{
		return font;
	}
	else
	{
		Error( "LoadFont: files are missing or corrupt" );
		return NULL;
	}
}


void InitFont( void )
{
	// [ = box with x, ] = box, { = bullet, } = small cursor, ^ = big cursor
	LoadFont( &s_font[0],  picFont,           "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!:,.()*?0123456789'|-[]{}^ " );
	LoadFont( &s_font[1],  picHiScoreFont,    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*().,/-=_+<>?|'\":; " );
	LoadFont( &s_font[2],  picContinueFont,   "0123456789" );
	LoadFont( &s_font[3],  picBalloonFont,    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+;:,./<>? []'\"" );
	LoadFont( &s_font[4],  picZapFont,        "0123456789*PS" );
	LoadFont( &s_font[5],  picZapOutlineFont, "0123456789*" );
	LoadFont( &s_font[6],  picVictoryFont,    "AB" );
	LoadFont( &s_font[7],  picBubbleFont,     "*" );
	LoadFont( &s_font[8],  picTinyFont,       "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789,.! " );
	LoadFont( &s_font[9],  picDashedLineFont, "." );
	LoadFont( &s_font[10], picBatsuFont,      "X" );
}


SkittlesFontPtr GetFont( int pictID )
{
	int fontID = pictID - picFont;

	if( (fontID < 0) || (fontID > kNumFonts) )
		Error( "GetFont: fontID" );

	return &s_font[fontID];
}


int GetTextWidth( SkittlesFontPtr font, const char *text )
{
	int width = 0;
	while( *text )
	{
		width += font->width[(int)*text++];
	}

	return width;
}

