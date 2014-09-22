// pause.h
#pragma once

void HandleDialog( int type );
void SurfaceGetEdges( SDL_Surface* edgeSurface, const SDL_Rect *rect );
void SurfaceCurveEdges( SDL_Surface* edgeSurface, const SDL_Rect *rect );

enum
{
	kPauseDialog = 0,
	kHiScoreDialog,
	kContinueDialog,
	kControlsDialog,
	kNumDialogs
};
