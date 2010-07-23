/*
 * history.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <string.h>
#include "history.h"
#include "menu.h"

// --- cHistoryObject -----------------------------------------------------

cHistoryObject::cHistoryObject(const char *xml, const char *ref, int ID) {
  osdxml = strdup(xml);
  reference = strdup(ref);
  id = ID;
  selected = 0;
}

cHistoryObject::~cHistoryObject() {
  if (osdxml)
    free(osdxml);
  if (reference)
    free(reference);

  for (int i=0; i < editData.Size(); i++)
    delete editData[i];
}

cQueryData *cHistoryObject::GetEditItem(const char *controlName) {
  for (int i=0; i < editData.Size(); i++) {
    if (strcmp(editData[i]->GetName(), controlName) == 0) {
      return editData[i];
    }
  }

  return NULL;
}

int cHistoryObject::QuerySize() const {
  return editData.Size();
}

char *cHistoryObject::GetQueryFragment(int i) const {
  if (i < 0 && i >= editData.Size())
    return NULL;
  else
    return editData[i]->GetQueryFragment();
}

cTextFieldData *cHistoryObject::GetTextFieldData(const char *controlName) {
  cQueryData *edititem = GetEditItem(controlName);
  cTextFieldData *tfdata = dynamic_cast<cTextFieldData *>(edititem);

  if (!tfdata) {
    tfdata = new cTextFieldData(controlName, 256);
    editData.Append(tfdata);
  }

  return tfdata;
}

cItemListData *cHistoryObject::GetItemListData(const char *controlName,
                                               cStringList &items,
                                               cStringList &values) {
  int n;
  char **itemtable, **itemvaluetable;
  cQueryData *edititem = GetEditItem(controlName);
  cItemListData *ildata = dynamic_cast<cItemListData *>(edititem);

  if (!ildata) {
    n = min(items.Size(), values.Size());
    itemtable = (char **)malloc(n*sizeof(char *));
    itemvaluetable = (char **)malloc(n*sizeof(char *));

    for (int i=0; i<n; i++) {
      itemtable[i] = strdup(csc.Convert(items[i]));
      itemvaluetable[i] = strdup(values[i]);
    }
    
    ildata = new cItemListData(controlName,
                               itemtable,
                               itemvaluetable,
                               n);

    editData.Append(ildata);
  }

  return ildata;
}

// --- cHistory ------------------------------------------------------------

cHistory::cHistory() {
  current = NULL;
}

void cHistory::Clear() {
  current = NULL;
  cList<cHistoryObject>::Clear();
}

void cHistory::TruncateAndAdd(cHistoryObject *page) {
  cHistoryObject *last = Last();
  while ((last) && (last != current)) {
    Del(last);
    last = Last();
  }

  Add(page);
  current = Last();
}

void cHistory::Reset() {
  current = NULL;
}

cHistoryObject *cHistory::Current() {
  return current;
}

cHistoryObject *cHistory::Home() {
  current = First();
  return current;
}

cHistoryObject *cHistory::Back() {
  if (current)
    current = Prev(current);
  return current;
}

cHistoryObject *cHistory::Forward() {
  cHistoryObject *next;
  if (current) {
    next = Next(current);
    if (next)
      current = next;
  } else {
    current = First();
  }
  return current;
}
