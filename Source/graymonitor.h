// graymonitor.h
#pragma once

void InitGrayMonitors( void );
void ShowGrayMonitor( short player );

extern MRect grayMonitorZRect, grayMonitorRect[2];
extern bool grayMonitorVisible[2];
