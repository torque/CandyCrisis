// blitter.c

#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include "SDLU.h"

#include "blitter.h"
#include "font.h"
#include "graphics.h"
#include "gworld.h"
#include "level.h"
#include "main.h"

bool update[2][kGridAcross][kGridDown];
bool refresh[2];

void InitBlitter( void )
{
	int player, x, y;

	for( player=0; player<=1; player++ )
	{
		refresh[player] = false;

		for( x=0; x<kGridAcross; x++ )
		{
			for( y=0; y<kGridDown; y++ )
			{
				update[player][x][y] = false;
			}
		}
	}
}

void UpdatePlayerWindow( int player )
{
	int x, y;

	if( control[player] == kNobodyControl ) return;

	if( playerWindowVisible[player] && refresh[player] )
	{
		SDL_Rect updateRect = { .x = 0, .y = 0, .w = 0, .h = 0}, fullRect, offsetRect;
		bool first = true;

		for( x=0; x<kGridAcross; x++ )
		{
			for( y=1; y<kGridDown; y++ )
			{
				if( update[player][x][y] )
				{
					updateRect.y = y * kBlobVertSize;
					updateRect.x = x * kBlobHorizSize;
					updateRect.h = kBlobVertSize;
					updateRect.w = kBlobHorizSize;
					if( first )
					{
						fullRect = updateRect;
						first = false;
					}
					else
					{
						SDLU_UnionRect( &fullRect, &updateRect, &fullRect );
					}

					update[player][x][y] = false;
				}
			}
		}

		if( !first )
		{
			offsetRect = fullRect;
			SDLU_OffsetRect( &offsetRect, playerWindowRect[player].x, playerWindowRect[player].y - kBlobVertSize );

			SDLU_BlitFrontSurface( playerSpriteSurface[player], &fullRect, &offsetRect );
		}
	}
}

void SetUpdateRect( int player, SDL_Rect *where )
{
	int x,y;
	int xMin, xMax, yMin, yMax;

	xMin = where->x / kBlobHorizSize;
	xMax = ( where->x + where->w + kBlobHorizSize - 1 ) / kBlobHorizSize;

	if( xMin < 0 ) xMin = 0;
	if( xMin > (kGridAcross-1) ) xMin = kGridAcross-1;
	if( xMax < 0 ) xMax = 0;
	if( xMax > kGridAcross ) xMax = kGridAcross;

	yMin = where->y / kBlobVertSize;
	yMax = ( where->y + where->h + kBlobVertSize - 1 ) / kBlobVertSize;

	if( yMin < 0 ) yMin = 0;
	if( yMin > (kGridDown-1) ) yMin = kGridDown-1;
	if( yMax < 0 ) yMax = 0;
	if( yMax > kGridDown ) yMax = kGridDown;

	for( x=xMin; x<xMax; x++ )
	{
		for( y=yMin; y<yMax; y++ )
		{
			update[player][x][y] = true;
		}
	}

	refresh[player] = true;
}


void SurfaceBlitMask( SDL_Surface*    object,     SDL_Surface*    mask,     SDL_Surface*    dest,
                      const SDL_Rect* objectRect, const SDL_Rect* maskRect, const SDL_Rect* destRect )
{
	int            startX = 0, startY = 0, endX, endY, x, y, srcRowBytes, mskRowBytes, dstRowBytes;
	unsigned long  bit, startBit, maskBits;
	unsigned char *src, *msk, *dst;
	SDL_Rect       destBounds = dest->clip_rect;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return; // do nothing
	}

	endX = objectRect->w;
	endY = objectRect->h;

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	src         = (unsigned char*) object->pixels;
	msk         = (unsigned char*) mask->pixels;
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = object->pitch;
	mskRowBytes = mask->pitch;
	dstRowBytes = dest->pitch;

	src += (objectRect->y * srcRowBytes) + (objectRect->x * 2);
	msk += (maskRect->y   * mskRowBytes) + (maskRect->x   / 8);
	dst += (destRect->y   * dstRowBytes) + (destRect->x   * 2);

	startBit = 0x80000000 >> (startX & 31);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	src += (srcRowBytes * startY) + (startX * 2);
	dst += (dstRowBytes * startY) + (startX * 2);

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst, *tMsk = msk;

		maskBits = SDL_SwapBE32( *(unsigned long*)msk );
		bit = startBit;

		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				*(short*)dst = *(short*)src;
			}

			if( !(bit >>= 1) ) { msk += 4; maskBits = SDL_SwapBE32( *(unsigned long*)msk ); bit = 0x80000000; }
			src += 2; dst += 2;
		}

		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;
		msk = tMsk + mskRowBytes;
	}
}


void SurfaceBlitBlob( const SDL_Rect* blobRect, const SDL_Rect* destRect )
{
	SurfaceBlitMask( blobSurface, maskSurface, SDLU_GetCurrentSurface(),
	                 blobRect,    blobRect,    destRect                  );
}

void SurfaceBlitColor( SDL_Surface*    mask,     SDL_Surface*    dest,
                       const SDL_Rect* maskRect, const SDL_Rect* destRect,
                       int r, int g, int b, int weight )
{
	int            startX = 0, startY = 0, endX, endY, x, y, mskRowBytes, dstRowBytes;
	unsigned long  bit, startBit, maskBits;
	unsigned char *msk, *dst;
	SDL_Rect       destBounds = dest->clip_rect;

	endX = maskRect->w;
	endY = maskRect->h;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return;
	}

	msk         = (unsigned char*) mask->pixels;
	dst         = (unsigned char*) dest->pixels;
	mskRowBytes = mask->pitch;
	dstRowBytes = dest->pitch;

	msk += (maskRect->y * mskRowBytes) + (maskRect->x / 8);
	dst += (destRect->y * dstRowBytes) + (destRect->x * 2);

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	startBit = 0x80000000 >> (startX & 31);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	dst += (dstRowBytes * startY) + (startX * 2);

	r *= weight;
	g *= weight;
	b *= weight;
	weight = 32 - weight;

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tMsk = msk, *tDst = dst;
		int work, workB, workG, workR;

		maskBits = SDL_SwapBE32( *(unsigned long*)msk );
		bit = startBit;

		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				work = *(short*)dst;
				workB = ((((work    ) & 0x001F)*weight) + b) >> 5;
				workG = ((((work>> 5) & 0x001F)*weight) + g);
				workR = ((((work>>10) & 0x001F)*weight) + r) << 5;

				*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
			}

			if( !(bit >>= 1) ) {
				msk += 4;
				maskBits = SDL_SwapBE32( *(unsigned long*)msk );
				bit = 0x80000000;
			}
			dst += 2;
		}

		msk = tMsk + mskRowBytes;
		dst = tDst + dstRowBytes;
	}
}


void SurfaceBlitAlpha( SDL_Surface* back,        SDL_Surface* source,        SDL_Surface* alpha,        SDL_Surface* dest,
                       const SDL_Rect* backRect, const SDL_Rect* sourceRect, const SDL_Rect* alphaRect, const SDL_Rect* destRect )
{
	int startX = 0, startY = 0, endX, endY, x, y, srcRowBytes, alfRowBytes, dstRowBytes, bckRowBytes;
	unsigned char *bck, *src, *alf, *dst;
	SDL_Rect destBounds = dest->clip_rect;

	endX = sourceRect->w;
	endY = sourceRect->h;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return; // do nothing
	}


	bck         = (unsigned char*) back->pixels;
	src         = (unsigned char*) source->pixels;
	alf         = (unsigned char*) alpha->pixels;
	dst         = (unsigned char*) dest->pixels;
	bckRowBytes = back->pitch;
	srcRowBytes = source->pitch;
	alfRowBytes = alpha->pitch;
	dstRowBytes = dest->pitch;

	bck += (backRect->y   * bckRowBytes) + (backRect->x   * 2);
	src += (sourceRect->y * srcRowBytes) + (sourceRect->x * 2);
	alf += (alphaRect->y  * alfRowBytes) + (alphaRect->x  * 2);
	dst += (destRect->y   * dstRowBytes) + (destRect->x   * 2);

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	bck += (bckRowBytes * startY) + (startX * 2);
	src += (srcRowBytes * startY) + (startX * 2);
	alf += (alfRowBytes * startY) + (startX * 2);
	dst += (dstRowBytes * startY) + (startX * 2);

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tAlf = alf, *tDst = dst, *tBck = bck;
		int pixS, pixB, weightS, weightB, workB, workG, workR;

		for( x=startX; x<endX; x++ )
		{
			weightB = *(short*)alf;

			if( weightB == 0x7FFF )
			{
				*(short*)dst = *(short*)bck;
			}
			else if( weightB == 0 )
			{
				*(short*)dst = *(short*)src;
			}
			else
			{
				weightS = ~weightB;

				pixS = *(short*)src;
				pixB = *(short*)bck;

				workB = ((((pixS      ) & 0x001F) * ((weightS      ) & 0x001F)) + (((pixB      ) & 0x001F) * ((weightB      ) & 0x001F))) >> 5;
				workG = ((((pixS >>  5) & 0x001F) * ((weightS >>  5) & 0x001F)) + (((pixB >>  5) & 0x001F) * ((weightB >>  5) & 0x001F)));
				workR = ((((pixS >> 10) & 0x001F) * ((weightS >> 10) & 0x001F)) + (((pixB >> 10) & 0x001F) * ((weightB >> 10) & 0x001F))) << 5;

				*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
			}

			src += 2;
			alf += 2;
			bck += 2;
			dst += 2;
		}

		bck = tBck + bckRowBytes;
		src = tSrc + srcRowBytes;
		alf = tAlf + alfRowBytes;
		dst = tDst + dstRowBytes;
	}
}


void SurfaceBlitWeightedDualAlpha( SDL_Surface* back,        SDL_Surface* source,        SDL_Surface* mask,        SDL_Surface* alpha,        SDL_Surface* dest,
                                   const SDL_Rect* backRect, const SDL_Rect* sourceRect, const SDL_Rect* maskRect, const SDL_Rect* alphaRect, const SDL_Rect* destRect,
                                   int inWeight )
{
	int startX = 0, startY = 0, endX, endY, x, y,
	    srcRowBytes, alfRowBytes, mskRowBytes, dstRowBytes, bckRowBytes;
	unsigned long  bit, startBit, maskBits;
	unsigned char *bck, *src, *alf, *msk, *dst;
	SDL_Rect destBounds = dest->clip_rect;

	endX = sourceRect->w;
	endY = sourceRect->h;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return; // do nothing
	}

	bck = (unsigned char*) back->pixels;
	src = (unsigned char*) source->pixels;
	msk = (unsigned char*) mask->pixels;
	alf = (unsigned char*) alpha->pixels;
	dst = (unsigned char*) dest->pixels;

	bckRowBytes = back->pitch;
	srcRowBytes = source->pitch;
	mskRowBytes = mask->pitch;
	alfRowBytes = alpha->pitch;
	dstRowBytes = dest->pitch;

	bck += (backRect->y   * bckRowBytes) + (backRect->x   * 2);
	src += (sourceRect->y * srcRowBytes) + (sourceRect->x * 2);
	alf += (alphaRect->y  * alfRowBytes) + (alphaRect->x  * 2);
	dst += (destRect->y   * dstRowBytes) + (destRect->x   * 2);
	msk += (maskRect->y   * mskRowBytes) + (maskRect->x   / 8);

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	bck += (bckRowBytes * startY) + (startX * 2);
	src += (srcRowBytes * startY) + (startX * 2);
	alf += (alfRowBytes * startY) + (startX * 2);
	dst += (dstRowBytes * startY) + (startX * 2);
	msk += (mskRowBytes * startY) + ((startX & ~31) / 8);
	startBit = 0x80000000 >> (startX & 31);

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tAlf = alf, *tDst = dst, *tBck = bck, *tMsk = msk;
		int pixS, pixB, weightS, weightB, work, workB, workG, workR;

		maskBits = SDL_SwapBE32( *(unsigned long*)msk );
		bit = startBit;

		for( x=startX; x<endX; x++ )
		{
			if( maskBits & bit )
			{
				work = *(short*)alf;
				workB = ((((work    ) & 0x001F)*inWeight) ) >> 5;
				workG = ((((work>> 5) & 0x001F)*inWeight) );
				workR = ((((work>>10) & 0x001F)*inWeight) ) << 5;

				weightB = ~((workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F));

				if( weightB == 0x7FFF )
				{
					*(short*)dst = *(short*)bck;
				}
				else if( weightB == 0 )
				{
					*(short*)dst = *(short*)src;
				}
				else
				{
					weightS = ~weightB;

					pixS = *(short*)src;
					pixB = *(short*)bck;

					workB = ((((pixS      ) & 0x001F) * ((weightS      ) & 0x001F)) + (((pixB      ) & 0x001F) * ((weightB      ) & 0x001F))) >> 5;
					workG = ((((pixS >>  5) & 0x001F) * ((weightS >>  5) & 0x001F)) + (((pixB >>  5) & 0x001F) * ((weightB >>  5) & 0x001F)));
					workR = ((((pixS >> 10) & 0x001F) * ((weightS >> 10) & 0x001F)) + (((pixB >> 10) & 0x001F) * ((weightB >> 10) & 0x001F))) << 5;

					*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
				}
			}

			if( !(bit >>= 1) ) { msk += 4; maskBits = SDL_SwapBE32( *(unsigned long*)msk ); bit = 0x80000000; }
			src += 2;
			alf += 2;
			bck += 2;
			dst += 2;
		}

		bck = tBck + bckRowBytes;
		src = tSrc + srcRowBytes;
		alf = tAlf + alfRowBytes;
		dst = tDst + dstRowBytes;
		msk = tMsk + mskRowBytes;
	}
}

void SurfaceBlitWeightedCharacter( SkittlesFontPtr font, unsigned char text, SDL_Point *dPoint, int r, int g, int b, int alpha )
{
	if( alpha == 31 )
	{
		SurfaceBlitCharacter( font, text, dPoint, r, g, b, 0 );
	}
	else if( alpha == 0 )
	{
		return;
	}
	else
	{
		SDL_Surface*   destSurface = SDLU_GetCurrentSurface();
		unsigned char* src;
		unsigned char* dst;
		int            srcRowBytes;
		int            dstRowBytes;
		int            index;
		SDL_Rect       destBounds = destSurface->clip_rect;

		int height = font->surface->h;
		int width  = font->width[text];
		int across = font->across[text];

		if( (dPoint->x + width)  > (destBounds.x + destBounds.w) ||
		    (dPoint->y + height) > (destBounds.y + destBounds.h) ||
		     dPoint->x           < destBounds.x ||
		     dPoint->y           < destBounds.y  )
		{
			dPoint->x += width;
			return; // do nothing
		}

		srcRowBytes = font->surface->pitch;
		dstRowBytes = destSurface->pitch;

		src = (unsigned char*) font->surface->pixels + across;
		dst = (unsigned char*) destSurface->pixels + (dPoint->x * 2) + (dPoint->y * dstRowBytes);

		while( height-- )
		{
			unsigned char *tSrc = src, *tDst = dst;

			for( index=0; index<width; index++ )
			{
				int workR, workG, workB, work, weightS, weightD;
				weightS = (*src >> 3) & 0x1F;

				weightS = (weightS * alpha) >> 5;

				if( weightS )
				{
					weightD = 32 - weightS;

					work = *(short*)dst;
					workB = ((((work    ) & 0x001F) * weightD) + (b * weightS)) >> 5;
					workG = ((((work>> 5) & 0x001F) * weightD) + (g * weightS));
					workR = ((((work>>10) & 0x001F) * weightD) + (r * weightS)) << 5;

					*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
				}

				src++;
				dst+=2;
			}

			src = tSrc + srcRowBytes;
			dst = tDst + dstRowBytes;
		}

		dPoint->x += width;
	}
}

void SurfaceBlitCharacter( SkittlesFontPtr font, unsigned char text, SDL_Point *dPoint, int r, int g, int b, int dropShadow )
{
	SDL_Surface*    destSurface = SDLU_GetCurrentSurface();
	unsigned char*  src;
	unsigned char*  dst;
	int             srcRowBytes;
	int             dstRowBytes;
	int             index;
	int             rgb555;
	SDL_Rect        destBounds = destSurface->clip_rect;

	int height = font->surface->h;
	int width  = font->width[text];
	int across = font->across[text];

	if( (dPoint->x + width)  > (destBounds.x + destBounds.w) ||
	    (dPoint->y + height) > (destBounds.y + destBounds.h) ||
	     dPoint->x           < destBounds.x ||
	     dPoint->y           < destBounds.y  )
	{
		dPoint->x += width;
		return; // do nothing
	}

	srcRowBytes = font->surface->pitch;
	dstRowBytes = destSurface->pitch;

	src = (unsigned char*) font->surface->pixels + across;
	dst = (unsigned char*) destSurface->pixels + (dPoint->x * 2) + (dPoint->y * dstRowBytes);
	rgb555 = (r << 10) | (g << 5) | b;

	switch( dropShadow )
	{
		case 0:
			while( height-- )
			{
				unsigned char *tSrc = src, *tDst = dst;

				for( index=0; index<width; index++ )
				{
					int workR, workG, workB, work, weightS, weightD;
					weightS = (*src >> 3) & 0x1F;

					if( weightS == 31 )
					{
						*(short*)dst = rgb555;
					}
					else if( weightS > 0 )
					{
						weightD = 32 - weightS;

						work = *(short*)dst;
						workB = ((((work    ) & 0x001F) * weightD) + (b * weightS)) >> 5;
						workG = ((((work>> 5) & 0x001F) * weightD) + (g * weightS));
						workR = ((((work>>10) & 0x001F) * weightD) + (r * weightS)) << 5;

						*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
					}

					src++;
					dst+=2;
				}

				src = tSrc + srcRowBytes;
				dst = tDst + dstRowBytes;
			}
			break;

		default:
			{
				unsigned char *drp = dst + ((dstRowBytes + 2) * dropShadow);

				while( height-- )
				{
					unsigned char *tSrc = src, *tDst = dst, *tDrp = drp;

					for( index=0; index<width; index++ )
					{
						int workR, workG, workB, work, weightS, weightD;
						weightS = (*src >> 3) & 0x1F;

						if( weightS == 31 )
						{
							*(short*)drp = 0;
							*(short*)dst = rgb555;
						}
						else if( weightS > 0 )
						{
							weightD = 32 - weightS;

							work = *(short*)drp;
							workB = ((((work    ) & 0x001F) * weightD)) >> 5;
							workG = ((((work>> 5) & 0x001F) * weightD));
							workR = ((((work>>10) & 0x001F) * weightD)) << 5;
							*(short*)drp = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);

							work = *(short*)dst;
							workB = ((((work    ) & 0x001F) * weightD) + (b * weightS)) >> 5;
							workG = ((((work>> 5) & 0x001F) * weightD) + (g * weightS));
							workR = ((((work>>10) & 0x001F) * weightD) + (r * weightS)) << 5;
							*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
						}

						src++;
						dst+=2;
						drp+=2;
					}

					src = tSrc + srcRowBytes;
					dst = tDst + dstRowBytes;
					drp = tDrp + dstRowBytes;
				}
				break;
			}
	}
	dPoint->x += width;
}

void SurfaceBlitColorOver( SDL_Surface* source,        SDL_Surface* dest,
                           const SDL_Rect* sourceRect, const SDL_Rect* destRect,
                           int r, int g, int b, int weight )
{
	int            startX = 0, startY = 0, endX, endY, x, y, dstRowBytes, srcRowBytes;
	unsigned char* src;
	unsigned char* dst;
	SDL_Rect       destBounds = dest->clip_rect;

	endX = destRect->w;
	endY = destRect->h;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return; // do nothing
	}

	src         = (unsigned char*) source->pixels;
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = source->pitch;
	dstRowBytes = dest->pitch;

	src += (sourceRect->y * srcRowBytes) + (sourceRect->x * 2);
	dst += (destRect->y * dstRowBytes) + (destRect->x * 2);

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	src += (srcRowBytes * startY) + (startX * 2);
	dst += (dstRowBytes * startY) + (startX * 2);

	r *= weight;
	g *= weight;
	b *= weight;
	weight = 32 - weight;

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst;
		int work, workB, workG, workR;

		for( x=startX; x<endX; x++ )
		{
			work = *(short*)src;
			workB = ((((work    ) & 0x001F)*weight) + b) >> 5;
			workG = ((((work>> 5) & 0x001F)*weight) + g);
			workR = ((((work>>10) & 0x001F)*weight) + r) << 5;

			*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);

			src += 2;
			dst += 2;
		}

		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;
	}
}

// NOTE: due to rounding error, there should never be a case of transitioning all the way from 0 to 31.
// You'll overflow and get weird glitches on the edges.

void SurfaceBlitBlendOver( SDL_Surface* source,        SDL_Surface* dest,
                           const SDL_Rect* sourceRect, const SDL_Rect* destRect,
                           int r1, int g1, int b1,
                           int r2, int g2, int b2,
                           int r3, int g3, int b3,
                           int r4, int g4, int b4,
                           int weight )
{
	int startX = 0, startY = 0, endX, endY, x, y;
	int rA, gA, bA;
	int rB, gB, bB;
	int rI, gI, bI;
	int vrLI, vgLI, vbLI;
	int vrRI, vgRI, vbRI;
	int weight16, ditherDiff;
	int width, height, dstRowBytes, srcRowBytes;
	unsigned char *src, *dst;
	SDL_Rect destBounds = dest->clip_rect;
	bool oddX = false;

	if( SDLU_SeparateRects( destRect, &dest->clip_rect ) )
	{
		return; // do nothing
	}

	endX = destRect->w;
	endY = destRect->h;

	src         = (unsigned char*) source->pixels;
	dst         = (unsigned char*) dest->pixels;
	srcRowBytes = source->pitch;
	dstRowBytes = dest->pitch;

	src += (sourceRect->y * srcRowBytes) + (sourceRect->x * 2);
	dst += (destRect->y * dstRowBytes)   + (destRect->x * 2);

	if( destRect->x < destBounds.x ) {
		startX -= destRect->x - destBounds.x;
	}
	if( (destRect->x + destRect->w) > (destBounds.x + destBounds.w) ){
		endX   -= (destRect->x + destRect->w) - (destBounds.x + destBounds.w);
	}
	if( destRect->y < destBounds.y ) {
		startY -= destRect->y - destBounds.y;
	}
	if( (destRect->y + destRect->h) > (destBounds.y + destBounds.h) ) {
		endY   -= (destRect->y + destRect->h) - (destBounds.y + destBounds.h);
	}

	src += (srcRowBytes * startY) + (startX * 2);
	dst += (dstRowBytes * startY) + (startX * 2);

	height = endY - startY;
	width  = endX - startX;

	weight16 = weight << 16;

	r1 *= weight16;		g1 *= weight16;		b1 *= weight16;
	r2 *= weight16;		g2 *= weight16;		b2 *= weight16;
	r3 *= weight16;		g3 *= weight16;		b3 *= weight16;
	r4 *= weight16;		g4 *= weight16;		b4 *= weight16;
	ditherDiff = weight16 >> 1;

	vrLI = (r3 - r1) / height;
	vgLI = (g3 - g1) / height;
	vbLI = (b3 - b1) / height;

	vrRI = (r4 - r2) / height;
	vgRI = (g4 - g2) / height;
	vbRI = (b4 - b2) / height;

	weight = 32 - weight;

	if( (endX - startX) & 1 )
	{
		endX--;
		oddX = true;
	}

	for( y=startY; y<endY; y++ )
	{
		unsigned char *tSrc = src, *tDst = dst;
		int work, workB, workG, workR;

		if( y & 1 )
		{
			rA =  r1;
			gA =  g1;
			bA =  b1;
			rB =  r1 - ditherDiff; if( rB < 0 ) rB = 0;
			gB =  g1 - ditherDiff; if( gB < 0 ) gB = 0;
			bB =  b1 - ditherDiff; if( bB < 0 ) bB = 0;
		}
		else
		{
			rA =  r1 - ditherDiff; if( rA < 0 ) rA = 0;
			gA =  g1 - ditherDiff; if( gA < 0 ) gA = 0;
			bA =  b1 - ditherDiff; if( bA < 0 ) bA = 0;
			rB =  r1;
			gB =  g1;
			bB =  b1;
		}

		rI = (2 * (r2 - r1)) / width;
		gI = (2 * (g2 - g1)) / width;
		bI = (2 * (b2 - b1)) / width;

		for( x=startX; x<endX; x+=2 )
		{
			work = *(short*)src;
			workB = ((((work<<16) & 0x001F0000)*weight) + bA) >> 21;
			workG = ((((work<<11) & 0x001F0000)*weight) + gA) >> 16;
			workR = ((((work<< 6) & 0x001F0000)*weight) + rA) >> 11;

			*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);

			src+= 2;
			dst+= 2;
			rA += rI;
			gA += gI;
			bA += bI;

			work = *(short*)src;
			workB = ((((work<<16) & 0x001F0000)*weight) + bB) >> 21;
			workG = ((((work<<11) & 0x001F0000)*weight) + gB) >> 16;
			workR = ((((work<< 6) & 0x001F0000)*weight) + rB) >> 11;

			*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);

			src+= 2;
			dst+= 2;
			rB += rI;
			gB += gI;
			bB += bI;
		}

		if( oddX )
		{
			work = *(short*)src;
			workB = ((((work<<16) & 0x001F0000)*weight) + bA) >> 21;
			workG = ((((work<<11) & 0x001F0000)*weight) + gA) >> 16;
			workR = ((((work<< 6) & 0x001F0000)*weight) + rA) >> 11;

			*(short*)dst = (workR & 0x7C00) | (workG & 0x03E0) | (workB & 0x001F);
		}

		src = tSrc + srcRowBytes;
		dst = tDst + dstRowBytes;

		r1 += vrLI; r2 += vrRI;
		g1 += vgLI; g2 += vgRI;
		b1 += vbLI; b2 += vbRI;
	}
}

