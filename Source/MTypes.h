/////
///  MTypes.h
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///
#pragma once

typedef signed char MBoolean;

typedef struct MRect
{
	short top;
	short left;
	short bottom;
	short right;
} MRect;


typedef struct MPoint
{
	short v;
	short h;
} MPoint;


void UnionMRect( const MRect* a, const MRect* b, MRect* u );
void OffsetMRect( MRect* r, int x, int y );
unsigned char MPointInMRect( MPoint p, MRect* r );
