///
///  SDLU.c
///
///  SDL utilities.
///
///  John Stiles, 2002/10/12
///

#include <stdio.h>

#include <SDL/SDL.h>
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
static SDLU_Point   s_mousePosition;

// for event loop
static bool         s_isForeground = true;

// for checktyping
static bool         s_interestedInTyping = false;
static char         s_keyBufferASCII[16] = { 0 };
static SDLKey       s_keyBufferSDL[16]   = { (SDLKey) 0};
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

static void SDLUi_SetGrayscaleColors( SDL_Surface* surface )
{
	SDL_Color  grayscalePalette[256];
	int        index;

	for( index=0; index<256; index++ )
	{
		grayscalePalette[index].r =
		grayscalePalette[index].g =
		grayscalePalette[index].b = 255 - index;
		grayscalePalette[index].unused = 0;
	}

	SDL_SetColors( surface, grayscalePalette, 0, 256 );
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
	SDL_Color       k_oneBitPalette[2] = { { 0xFF, 0xFF, 0xFF, 0x00 },
	                                       { 0x00, 0x00, 0x00, 0x00 }  };

	switch( depth )
	{
		case 16:
			surface = SDL_CreateRGBSurface(
							SDL_HWSURFACE,
							rect->w,
							rect->h,
							15,
							0x7C00, 0x03E0, 0x001F, 0x0000 );
			break;

		case 8:
			surface = SDL_CreateRGBSurface(
							SDL_HWSURFACE,
							rect->w,
							rect->h,
							8,
							0, 0, 0, 0 );

			SDLUi_SetGrayscaleColors( surface );
			break;

		case 1:
			surface = SDL_CreateRGBSurface(
							SDL_HWSURFACE,
							rect->w,
							rect->h,
							1,
							0, 0, 0, 0 );

			SDL_SetColors( surface, k_oneBitPalette, 0, 2 );
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
	SDLU_BlitSurface( source,       sourceSDLRect,
	                  frontSurface, destSDLRect );

	SDL_UpdateRect( frontSurface, destSDLRect->x, destSDLRect->y, destSDLRect->w, destSDLRect->h );
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

int SDLU_EventFilter( const SDL_Event *event )
{
	// Put keydowns in a buffer
	if( event->type == SDL_KEYDOWN )
	{
		if(    s_interestedInTyping
		    && event->key.keysym.unicode <= 127
		    && s_keyBufferFilled < sizeof(s_keyBufferASCII) )
		{
			s_keyBufferFilled++;
			s_keyBufferASCII[s_keyBufferPutAt] = event->key.keysym.unicode;
			s_keyBufferSDL  [s_keyBufferPutAt] = event->key.keysym.sym;
			s_keyBufferPutAt = (s_keyBufferPutAt + 1) % sizeof(s_keyBufferASCII);
		}

		if(    (event->key.keysym.sym == SDLK_F4)
		    && (event->key.keysym.mod & (KMOD_LALT | KMOD_RALT)) )
		{
			finished = true;
		}

		return 0;
	}

	// Get mouse state
	if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		if( event->button.button == SDL_BUTTON_LEFT )
			s_mouseButton = true;

		s_mousePosition.y = event->button.y;
		s_mousePosition.x = event->button.x;
		return 0;
	}

	if( event->type == SDL_MOUSEBUTTONUP )
	{
		if( event->button.button == SDL_BUTTON_LEFT )
			s_mouseButton = false;

		s_mousePosition.y = event->button.y;
		s_mousePosition.x = event->button.x;
		return 0;
	}

	if( event->type == SDL_MOUSEMOTION )
	{
		s_mousePosition.y = event->motion.y;
		s_mousePosition.x = event->motion.x;
		s_mouseButton = event->motion.state & SDL_BUTTON(1);
		return 0;
	}

	if( event->type == SDL_QUIT )
	{
		finished = true;
		return 0;
	}

	// Handle gaining and losing focus (kind of cheesy)
	if( event->type == SDL_ACTIVEEVENT && (event->active.state & SDL_APPINPUTFOCUS) )
	{
		if( !event->active.gain && s_isForeground )
		{
			FreezeGameTickCount();
			PauseMusic();
            s_isForeground = false;
		}
		else if( event->active.gain && !s_isForeground )
		{
			UnfreezeGameTickCount();
			ResumeMusic();
            s_isForeground = true;

			DoFullRepaint();
		}
	}

	// We never poll for events, we just process them here, so discard everything.
	return 0;
}

void SDLU_StartWatchingTyping( void )
{
	s_interestedInTyping = true;
	s_keyBufferFilled = s_keyBufferPullFrom = s_keyBufferPutAt = 0;
	SDL_EnableUNICODE( 1 );
	SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );
}

void SDLU_StopWatchingTyping( void )
{
	s_interestedInTyping = false;
	SDL_EnableUNICODE( 0 );
	SDL_EnableKeyRepeat( 0, 0 );
}

bool SDLU_CheckTyping( char* ascii, SDLKey* sdl )
{
	if( s_keyBufferFilled > 0 )
	{
		*ascii = s_keyBufferASCII[s_keyBufferPullFrom];
		*sdl   = s_keyBufferSDL  [s_keyBufferPullFrom];
		s_keyBufferPullFrom = (s_keyBufferPullFrom + 1) % sizeof(s_keyBufferASCII);
		s_keyBufferFilled--;
		return true;
	}

	*ascii = '\0';
	*sdl   = SDLK_UNKNOWN;
	return false;
}

void SDLU_GetMouse( SDLU_Point* pt )
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

bool SDLU_PointInRect( SDLU_Point p, SDL_Rect* r )
{
	return (p.x >= r->x)        &&
	       (p.x <  r->x + r->w) &&
	       (p.y >= r->y)        &&
	       (p.y <  r->y + r->h);
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
