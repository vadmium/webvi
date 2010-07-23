/*
 * menudata.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_MENUDATA_H
#define __WEBVIDEO_MENUDATA_H

// --- cLinkBase -----------------------------------------------------------

class cLinkBase {
public:
  virtual ~cLinkBase() {}; // avoids "virtual functions but
			   // non-virtual destructor" warning

  virtual char *GetURL() = 0;
};

// --- cQueryData ----------------------------------------------------------

class cQueryData {
private:
  char *name;

public:
  cQueryData(const char *Name);
  virtual ~cQueryData();

  const char *GetName() { return name; }
  virtual char *GetQueryFragment() = 0;
};

// --- cSimpleLink ---------------------------------------------------------

class cSimpleLink : public cLinkBase {
private:
  char *ref;
public:
  cSimpleLink(const char *ref);
  virtual ~cSimpleLink();

  virtual char *GetURL();
};

// --- cTextFieldData ------------------------------------------------------

class cTextFieldData : public cQueryData {
private:
  char *name;
  char *valuebuffer;
  int valuebufferlength;
public:
  cTextFieldData(const char *Name, int Length);
  virtual ~cTextFieldData();

  virtual char *GetQueryFragment();
  char *GetValue();
  int GetLength();
};

// --- cItemListData -------------------------------------------------------

class cItemListData : public cQueryData {
private:
  char *name;
  int value;
  int numstrings;
  char **strings;
  char **stringvalues;
public:
  cItemListData(const char *Name, char **Strings, char **StringValues, int NumStrings);
  virtual ~cItemListData();

  virtual char *GetQueryFragment();
  char **GetStrings();
  char **GetStringValues();
  int GetNumStrings();
  int *GetValuePtr();
};

// --- cSubmissionButtonData -----------------------------------------------

class cHistoryObject;

class cSubmissionButtonData : public cLinkBase {
private:
  char *querybase;
  const cHistoryObject *page;
public:
  cSubmissionButtonData(const char *queryUrl,
                        const cHistoryObject *currentPage);
  virtual ~cSubmissionButtonData();

  virtual char *GetURL();
};

#endif
