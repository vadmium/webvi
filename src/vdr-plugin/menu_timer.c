/*
 * menu.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <time.h>
#include <vdr/i18n.h>
#include <vdr/tools.h>
#include <vdr/menuitems.h>
#include <vdr/interface.h>
#include "menu_timer.h"

#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])

const char *intervalNames[] = {NULL, NULL, NULL};
const int intervalValues[] = {24*60*60, 7*24*60*60, 30*24*60*60};

// --- cEditWebviTimerMenu -------------------------------------------------

cEditWebviTimerMenu::cEditWebviTimerMenu(cWebviTimer &timer,
					 bool refreshWhenDone,
					 bool execButton)
  : cOsdMenu(tr("Edit timer"), 20), timer(timer), interval(1),
    refresh(refreshWhenDone)
{
  // title
  strn0cpy(title, timer.GetTitle(), maxTitleLen);
  Add(new cMenuEditStrItem(tr("Title"), title, maxTitleLen));

  // interval
  for (unsigned i=0; i<ARRAYSIZE(intervalValues); i++) {
    if (timer.GetInterval() == intervalValues[i]) {
      interval = i;
      break;
    }
  }

  if (!intervalNames[0]) {
    // Initialize manually to make the translations work
    intervalNames[0] = tr("Once per day");
    intervalNames[1] = tr("Once per week");
    intervalNames[2] = tr("Once per month");
  }
  
  Add(new cMenuEditStraItem(tr("Update interval"), &interval,
                            ARRAYSIZE(intervalNames), intervalNames));
  
  // "execute now" button
  if (execButton)
    Add(new cOsdItem(tr("Execute now"), osUser1, true));

  // last update time
  char lastTime[25];
  if (timer.LastUpdate() == 0) {
    // TRANSLATORS: at most 24 chars
    strcpy(lastTime, tr("Never"));
  } else {
    time_t updateTime = timer.LastUpdate();
    strftime(lastTime, 25, "%x %X", localtime(&updateTime));
  }

  cString lastUpdated = cString::sprintf("%s\t%s", tr("Last fetched:"), lastTime);
  Add(new cOsdItem(lastUpdated, osUnknown, false));

  // still running?
  if (timer.Running()) {
    Add(new cOsdItem("Timer active", osUnknown, false));
  }

  // error
  if (!timer.Success()) {
    Add(new cOsdItem(tr("Error on last refresh!"), osUnknown, false));
    Add(new cOsdItem(timer.LastError(), osUnknown, false));
  }
}

cEditWebviTimerMenu::~cEditWebviTimerMenu() {
  if (refresh)
    timer.Execute();
}

eOSState cEditWebviTimerMenu::ProcessKey(eKeys Key) {
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (state == osContinue) {
    timer.SetTitle(title);
    timer.SetInterval(intervalValues[interval]);
  } else if (state == osUser1) {
    timer.Execute();
    Skins.Message(mtInfo, tr("Downloading in the background"));
  }

  return state;
}

// --- cWebviTimerListMenu -------------------------------------------------

cWebviTimerListMenu::cWebviTimerListMenu(cWebviTimerManager &timers) 
  : cOsdMenu(tr("Timers")), timers(timers)
{
  cWebviTimer *t = timers.First();
  while (t) {
    Add(new cOsdItem(t->GetTitle(), osUnknown, true));
    t = timers.Next(t);
  }

  SetHelp(NULL, NULL, tr("Remove"), NULL);
}

eOSState cWebviTimerListMenu::ProcessKey(eKeys Key) {
  cWebviTimer *t;
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (HasSubMenu())
    return state;

  if (state == osUnknown) {
    switch (Key) {
    case kOk:
      t = timers.GetLinear(Current());
      if (t)
        return AddSubMenu(new cEditWebviTimerMenu(*t));
      break;
      
    case kYellow:
      t = timers.GetLinear(Current());
      if (t) {
	if (t->Running()) {
	  // FIXME: ask if the user wants to cancel the downloads
	  Skins.Message(mtInfo, tr("Timer running, can't remove"));
	} else if (Interface->Confirm(tr("Remove timer?"))) {
	  timers.Remove(t);
	  Del(Current());
	  Display();
	}

	return osContinue;
      }
      break;

    default:
      break;
    }
  }

  return state;
}
