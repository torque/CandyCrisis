// moving.h
#pragma once

#include <stdbool.h>

bool CanMoveDirection( int player, int dirX, int dirY );
void CalcSecondBlobOffset( int player, int *x, int *y );
bool CanGoLeft( int player );
void GoLeft( int player );
bool CanGoRight( int player );
void GoRight( int player );
bool CanFall( int player );
void DoFall( int player );
bool CanRotate( int player );
void DoRotate( int player );
void DoDrop( int player );
void StopDrop( int player );
