///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "SDLU.h"

#include "gameticks.h"
#include "main.h" // for Error
#include "music.h"

// for acquiresurface
const int           k_acquireMax = 10;
static int          s_acquireHead = -1;
static SDL_Surface* s_acquireList[k_acquireMax];

// for button and getmouse
static int          s_mouseButton;
static SDL_Point   s_mousePosition;

// for event loop
static bool         s_isForeground = true;

// for checktyping
static bool         s_interestedInTyping = false;
static char         s_keyBufferASCII[16] = { 0 };
static SDL_Scancode s_keyBufferSDL[16]   = { [0 ... 15] = SDL_SCANCODE_UNKNOWN };
static int          s_keyBufferPullFrom = 0;
static int          s_keyBufferPutAt = 0;
static int          s_keyBufferFilled = 0;

static void	SDLUi_Blit8BitTo1Bit( SDL_Surface* surface8, SDL_Rect* rect8,
                                  SDL_Surface* surface1, SDL_Rect* rect1  )
{
	// NOTE: for now this copy assumes that we're copying the whole thing.
	// That's probably true for everything I'm doing. If it turns out that
	// I need partial 8->1 copies, this won't be too hard to fix.

	int          x, y, across, down;
	SDL_Color*   palette8;

//	ASSERTN( surface8->format->BitsPerPixel == 8, surface8->format->BitsPerPixel );
//	ASSERTN( surface1->format->BitsPerPixel == 1, surface8->format->BitsPerPixel );
//	ASSERT( rect8->w == rect1->w );
//	ASSERT( rect8->h == rect1->h );

	palette8 = surface8->format->palette->colors;
	down     = rect1->h;
	across   = (rect1->w + 7) & ~7;

	for( y=0; y<down; y++ )
	{
		unsigned char* src = (unsigned char*) surface8->pixels + (y * surface8->pitch);
		unsigned char* dst = (unsigned char*) surface1->pixels + (y * surface1->pitch);

		for( x=0; x<across; x+=8 )
		{
			*dst = (palette8[src[0]].r? 0: 0x80) |
			       (palette8[src[1]].r? 0: 0x40) |
			       (palette8[src[2]].r? 0: 0x20) |
			       (palette8[src[3]].r? 0: 0x10) |
			       (palette8[src[4]].r? 0: 0x08) |
			       (palette8[src[5]].r? 0: 0x04) |
			       (palette8[src[6]].r? 0: 0x02) |
			       (palette8[src[7]].r? 0: 0x01)   ;

			dst += 1;
			src += 8;
		}
	}
}

static void SDLUi_SetGrayscalePalette( SDL_Surface* surface )
{
	// save allocating/freeing another palette by knowing that it gets allocated
	// when the surface is created

	// SDL_Palette *palette = SDL_AllocPalette(256);

	for ( unsigned int index = 0; index < 256; index++ ) {
		uint8_t gray = 255 - index;
		surface->format->palette->colors[index] = (SDL_Color){
			.r = gray, .g = gray, .b = gray,
		};
	}

	// SDL_SetSurfacePalette( surface, palette );
	// SDL_FreePalette( palette );
}

int SDLU_BlitSurface( SDL_Surface* src, SDL_Rect* srcrect,
                      SDL_Surface* dst, SDL_Rect* dstrect  )
{
	if( src->format->BitsPerPixel == 8 && dst->format->BitsPerPixel == 1 )
	{
		// SDL BUG!! SDL cannot blit 8-bit to 1-bit surfaces.
		SDLUi_Blit8BitTo1Bit( src, srcrect,
		                      dst, dstrect  );
		return 0;
	}

	// Let SDL handle this.
	return SDL_BlitSurface( src, srcrect,
	                        dst, dstrect  );
}

void SDLU_GetPixel(	SDL_Surface* surface, int x, int y, SDL_Color* pixel )
{
	unsigned long px;
	unsigned char* ptr;

	switch( surface->format->BytesPerPixel )
	{
		case 1:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x);
			px = *(unsigned char*) ptr;
			break;

		case 2:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * 2);
			px = *(unsigned short*) ptr;
			break;

		case 4:
			ptr = (unsigned char*)surface->pixels + (y * surface->pitch) + (x * 4);
			px = *(unsigned long*) ptr;
			break;
	}

	return SDL_GetRGB( px, surface->format, &pixel->r, &pixel->g, &pixel->b );
}


void SDLU_ChangeSurfaceDepth( SDL_Surface** surface, int depth )
{
	SDL_Surface* newSurface;

	newSurface = SDLU_InitSurface( &surface[0]->clip_rect, depth );

	SDLU_BlitSurface( *surface,    &surface[0]->clip_rect,
	                   newSurface, &newSurface->clip_rect  );

	SDL_FreeSurface( *surface );

	*surface = newSurface;
}


SDL_Surface* SDLU_InitSurface( const SDL_Rect* rect, int depth )
{
	SDL_Surface*    surface = NULL;

	switch( depth )
	{
		case 16:
			surface = SDL_CreateRGBSurfaceWithFormat(0, rect->w, rect->h, 16, SDL_PIXELFORMAT_XRGB1555);
			break;

		case 8:
			surface = SDL_CreateRGBSurfaceWithFormat(0, rect->w, rect->h, 8, SDL_PIXELFORMAT_INDEX8);
			SDLUi_SetGrayscalePalette( surface );
			break;

		case 1:
			// this automatically sets up the palette
			surface = SDL_CreateRGBSurfaceWithFormat(0, rect->w, rect->h, 1, SDL_PIXELFORMAT_INDEX1LSB);
			break;
	}

	if( surface == NULL )
	{
		Error( "SDLU_InitSurface: SDL_CreateRGBSurface" );
		return NULL;
	}

	return surface;
}


void SDLU_BlitFrontSurface( SDL_Surface* source, SDL_Rect* sourceSDLRect, SDL_Rect* destSDLRect )
{
	extern SDL_Surface* frontSurface;
	static unsigned long lastTick = 0;
	SDLU_BlitSurface( source,       sourceSDLRect,
	                  frontSurface, destSDLRect );

	unsigned long thisTick = MTickCount();
	if ( thisTick > lastTick + 1 ) {
		SDL_UpdateWindowSurface( mainWindow );
		lastTick = thisTick;
	}
}

void SDLU_Yield( void )
{
	SDL_Delay( 2 );
	SDL_PumpEvents();
}

void SDLU_PumpEvents( void )
{
	static unsigned long lastPump = 0;
	unsigned long time = MTickCount();

	if( lastPump != time )
	{
		SDL_Event evt;
		while( SDL_PollEvent( &evt ) ) { }
		lastPump = time;
	}
}

bool SDLU_IsForeground( void )
{
	return s_isForeground;
}

int SDLU_EventFilter( void *userdata, SDL_Event *const event )
{
	switch( event->type ) {
		// Put keydowns in a buffer
		case SDL_KEYDOWN:
			if(    s_interestedInTyping
			    && event->key.keysym.sym < 300
			    && s_keyBufferFilled < sizeof(s_keyBufferASCII) )
			{
				s_keyBufferFilled++;
				// TODO SDL2: this is wrong because it ignores modifiers and
				// therefore letter case. SDL2 provides the SDL_TEXTINPUT
				// event (and corresponding SDL_TextInputEvent struct) which
				// contains a buffer of (one? null terminated? UTF-8?)
				// character(s).
				s_keyBufferASCII[s_keyBufferPutAt] = event->key.keysym.sym;
				s_keyBufferSDL  [s_keyBufferPutAt] = event->key.keysym.scancode;
				s_keyBufferPutAt = (s_keyBufferPutAt + 1) % sizeof(s_keyBufferASCII);
			}

			if( ((event->key.keysym.sym == SDLK_F4) &&
			    (event->key.keysym.mod & KMOD_ALT)) ||
			    ((event->key.keysym.sym == SDLK_q) &&
			    (event->key.keysym.mod & KMOD_GUI)) )
			{
				finished = true;
			}

			break;

		case SDL_TEXTINPUT:
			fprintf(stderr, "text input: %zu: %s\n", strnlen(event->text.text, SDL_TEXTINPUTEVENT_TEXT_SIZE), event->text.text);
			// TODO: just check consecutive bytes < 128 because our fonts etc
			// only accept ascii characters. Maybe we should do a lookup table
			// based on the drawable characters. This is annoyingly coupled
			// because pause.c has logic based on SDL keycodes mixed with ascii
			// input (specifically, return and backspace for the high score
			// name input). Those may get passed through as ascii characters
			// here, but even if they do, it will require overhauling the
			// dialog handling to be able to work with this, since the other
			// keyboard dialog, the controls selector, uses only the keycodes.
			break;

		case SDL_MOUSEBUTTONDOWN:
			if( event->button.button == SDL_BUTTON_LEFT )
				s_mouseButton = true;

			s_mousePosition.y = event->button.y;
			s_mousePosition.x = event->button.x;
			break;

		case SDL_MOUSEBUTTONUP:
			if( event->button.button == SDL_BUTTON_LEFT )
				s_mouseButton = false;

			s_mousePosition.y = event->button.y;
			s_mousePosition.x = event->button.x;
			break;

		case SDL_MOUSEMOTION:
			s_mousePosition.y = event->motion.y;
			s_mousePosition.x = event->motion.x;
			s_mouseButton = event->motion.state & SDL_BUTTON(1);
			break;

		case SDL_QUIT:
			finished = true;
			break;

		// Handle gaining and losing focus (kind of cheesy)
		case SDL_APP_WILLENTERBACKGROUND:
			if (s_isForeground) {
				FreezeGameTickCount();
				PauseMusic();
				s_isForeground = false;
			}
			break;

		case SDL_APP_DIDENTERFOREGROUND:
			if (!s_isForeground) {
				UnfreezeGameTickCount();
				ResumeMusic();
				s_isForeground = true;

				DoFullRepaint();
			}
			break;
	}
	return 0;
}

void SDLU_StartWatchingTyping( void )
{
	s_interestedInTyping = true;
	SDL_StartTextInput();
}

void SDLU_StopWatchingTyping( void )
{
	s_interestedInTyping = false;
	SDL_StopTextInput();
}

bool SDLU_CheckTyping( char* ascii, SDL_Scancode* sdl )
{
	// TODO: rip out the ascii character stuff
	if( s_keyBufferFilled > 0 )
	{
		*ascii = s_keyBufferASCII[s_keyBufferPullFrom];
		*sdl   = s_keyBufferSDL  [s_keyBufferPullFrom];
		s_keyBufferPullFrom = (s_keyBufferPullFrom + 1) % sizeof(s_keyBufferASCII);
		s_keyBufferFilled--;
		return true;
	}

	*ascii = '\0';
	*sdl   = SDL_SCANCODE_UNKNOWN;
	return false;
}

void SDLU_GetMouse( SDL_Point* pt )
{
	SDLU_PumpEvents();
	*pt = s_mousePosition;
}

int SDLU_Button( void )
{
	SDLU_PumpEvents();
	return s_mouseButton;
}

void SDLU_AcquireSurface( SDL_Surface* surface )
{
	s_acquireList[++s_acquireHead] = surface;
}

SDL_Surface* SDLU_GetCurrentSurface( void )
{
	return s_acquireList[s_acquireHead];
}

void SDLU_ReleaseSurface( SDL_Surface* surface )
{
	if( s_acquireList[s_acquireHead] != surface )
		Error( "SDLU_ReleaseSurface: out of order" );

	if( s_acquireHead < 0 )
		Error( "SDLU_ReleaseSurface: underflow" );

	s_acquireHead--;
}

// Returns true if rects do not overlap
bool SDLU_SeparateRects( const SDL_Rect *a, const SDL_Rect *b ) {
	return (  a->x > (b->x + b->w) || // a is completely right of b
	         (a->x + a->w) < b->x  || // a is completely left of b
	          a->y > (b->y + b->h) || // a is completely below b
	         (a->y + a->h) < b->y  ); // a is completely above b
}

// Creates a new SDL_Rect from the largest dimensions of two SDL_Rects.
void SDLU_UnionRect( const SDL_Rect* a, const SDL_Rect* b, SDL_Rect* u )
{
	int a_right = a->x + a->w;
	int b_right = b->x + b->w;
	int a_bot   = a->y + a->h;
	int b_bot   = b->y + b->h;

	u->x = ( a->x < b->x )? a->x: b->x;
	u->y = ( a->y < b->y )? a->y: b->y;
	u->w = ( a_right > b_right )? a_right - u->x: b_right - u->x;
	u->h = ( a_bot > b_bot )? a_bot - u->y: b_bot - u->y;
}

void SDLU_OffsetRect( SDL_Rect* r, int x, int y )
{
	r->x += x;
	r->y += y;
}
