/////
///  MTypes.h
///
///  Generic replacements for very basic Mac types.
///
///  John Stiles, 2002/10/14
///
#pragma once

typedef struct MRect
{
	short top;
	short left;
	short bottom;
	short right;
} MRect;


void UnionMRect( const MRect* a, const MRect* b, MRect* u );
void OffsetMRect( MRect* r, int x, int y );
