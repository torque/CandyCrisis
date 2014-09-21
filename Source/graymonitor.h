// graymonitor.h
#pragma once

void InitGrayMonitors( void );
void ShowGrayMonitor( short player );

extern SDL_Rect grayMonitorZRect, grayMonitorRect[2];
extern bool grayMonitorVisible[2];
