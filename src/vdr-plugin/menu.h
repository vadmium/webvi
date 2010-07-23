/*
 * menu.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_MENU_H
#define __WEBVIDEO_MENU_H

#include <time.h>
#include <vdr/osdbase.h>
#include <vdr/menuitems.h>
#include <vdr/menu.h>
#include <libxml/parser.h>
#include "download.h"
#include "menudata.h"

extern cCharSetConv csc;

// --- cXMLMenu --------------------------------------------------

class cXMLMenu : public cOsdMenu {
protected:
  virtual bool Deserialize(const char *xml);
  virtual bool CreateItemFromTag(xmlDocPtr doc, xmlNodePtr node) = 0;
public:
  cXMLMenu(const char *Title, int c0 = 0, int c1 = 0, 
           int c2 = 0, int c3 = 0, int c4 = 0);

  int Load(const char *xmlstr);
};

// --- cNavigationMenu -----------------------------------------------------

enum eLinkType { LT_REGULAR, LT_MEDIA, LT_STREAMINGMEDIA };

class cHistory;
class cHistoryObject;
class cStatusScreen;

class cNavigationMenu : public cXMLMenu {
private:
  // links[i] is the navigation link of the i:th item
  cVector<cLinkBase *> links;
  // streams[i] is the media stream link of the i:th item
  cVector<cLinkBase *> streams;
  cProgressVector& summaries;
  char *title;
  char *reference;
  int shortcutMode;

protected:
  cHistory *history;

  virtual bool CreateItemFromTag(xmlDocPtr doc, xmlNodePtr node);
  void AddLinkItem(cOsdItem *item, cLinkBase *ref, cLinkBase *streamref);
  void NewLinkItem(xmlDocPtr doc, xmlNodePtr node);
  void NewTextField(xmlDocPtr doc, xmlNodePtr node);
  void NewItemList(xmlDocPtr doc, xmlNodePtr node);
  void NewTextArea(xmlDocPtr doc, xmlNodePtr node);
  void NewButton(xmlDocPtr doc, xmlNodePtr node);
  void NewTitle(xmlDocPtr doc, xmlNodePtr node);
  void UpdateHelp();

public:
  cNavigationMenu(cHistory *History, cProgressVector& dlsummaries);
  virtual ~cNavigationMenu();

  virtual eOSState ProcessKey(eKeys Key);
  virtual eOSState Select(cLinkBase *link, eLinkType type);
  virtual void Clear(void);
  eOSState HistoryBack();
  eOSState HistoryForward();

  const char *Reference() const { return reference; }
  void Populate(const cHistoryObject *page, const char *statusmsg=NULL);
};

// --- cStatusScreen -------------------------------------------------------

class cStatusScreen : public cOsdMenu {
public:
  const static time_t updateInterval = 5; // seconds
private:
  cProgressVector& summaries;
  time_t lastupdate;

protected:
  void UpdateHelp();

public:
  cStatusScreen(cProgressVector& dlsummaries);
  ~cStatusScreen();

  void Update();
  bool NeedsUpdate();

  virtual eOSState ProcessKey(eKeys Key);
};

// --- MenuPointers --------------------------------------------------------

struct MenuPointers {
  cNavigationMenu *navigationMenu;
  cStatusScreen *statusScreen;

  MenuPointers() : navigationMenu(NULL), statusScreen(NULL) {};
};

extern struct MenuPointers menuPointers;

#endif // __WEBVIDEO_MENU_H
