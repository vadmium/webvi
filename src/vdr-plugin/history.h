/*
 * history.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_HISTORY_H
#define __WEBVIDEO_HISTORY_H

#include <vdr/tools.h>
#include "menudata.h"

// --- cHistoryObject -----------------------------------------------------

class cHistoryObject : public cListObject {
private:
  char *osdxml;
  int id;
  int selected;
  cVector<cQueryData *> editData;
  char *reference;

  cQueryData *GetEditItem(const char *controlName);

public:
  cHistoryObject(const char *xml, const char *reference, int ID);
  ~cHistoryObject();

  int GetID() const { return id; }
  const char *GetOSD() const { return osdxml; }
  const char *GetReference() const { return reference; }
  void RememberSelected(int sel) { selected = sel; }
  int GetSelected() const { return selected; }
  
  int QuerySize() const;
  char *GetQueryFragment(int i, const char *encoding) const;
  cTextFieldData *GetTextFieldData(const char *controlName);
  cItemListData *GetItemListData(const char *controlName,
                                 cStringList &items,
                                 cStringList &itemvalues);
};

// --- cHistory ------------------------------------------------------------

class cHistory : public cList<cHistoryObject> {
private:
  cHistoryObject *current;
public:
  cHistory();

  void Clear();
  void TruncateAndAdd(cHistoryObject *page);
  void Reset();
  cHistoryObject *Current();
  cHistoryObject *Home();
  cHistoryObject *Back();
  cHistoryObject *Forward();
};

#endif // __WEBVIDEO_HISTORY_H
