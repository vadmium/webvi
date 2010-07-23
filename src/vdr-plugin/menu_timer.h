/*
 * menu_timer.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_MENU_TIMER_H
#define __WEBVIDEO_MENU_TIMER_H

#include <vdr/osdbase.h>
#include "timer.h"

// --- cEditWebviTimerMenu -------------------------------------------------

class cEditWebviTimerMenu : public cOsdMenu {
private:
  static const int maxTitleLen = 128;

  cWebviTimer &timer;
  char title[maxTitleLen];
  int interval;
  bool refresh;

public:
  cEditWebviTimerMenu(cWebviTimer &timer, bool refreshWhenDone=false,
		      bool execButton=true);
  ~cEditWebviTimerMenu();

  virtual eOSState ProcessKey(eKeys Key);
};

// --- cWebviTimerListMenu -------------------------------------------------

class cWebviTimerListMenu : public cOsdMenu {
private:
  cWebviTimerManager& timers;

public:
  cWebviTimerListMenu(cWebviTimerManager &timers);

  virtual eOSState ProcessKey(eKeys Key);
};

#endif
