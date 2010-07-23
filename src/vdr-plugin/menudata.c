/*
 * menudata.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <string.h>
#include <stdlib.h>
#include <vdr/tools.h>
#include "menudata.h"
#include "common.h"
#include "history.h"

// --- cQueryData ----------------------------------------------------------

cQueryData::cQueryData(const char *Name) {
  name = Name ? strdup(Name) : NULL;
}

cQueryData::~cQueryData() {
  if (name)
    free(name);
}

// --- cSimpleLink ---------------------------------------------------------

cSimpleLink::cSimpleLink(const char *reference) {
  ref = reference ? strdup(reference) : NULL;
}

cSimpleLink::~cSimpleLink() {
  if (ref) {
    free(ref);
  }
}

char *cSimpleLink::GetURL() {
  return ref;
}

// --- cTextFieldData ------------------------------------------------------

cTextFieldData::cTextFieldData(const char *Name, int Length) 
: cQueryData(Name)
{
  valuebufferlength = Length;
  valuebuffer = (char *)malloc(Length*sizeof(char));
  *valuebuffer = '\0';
}

cTextFieldData::~cTextFieldData() {
  if(valuebuffer)
    free(valuebuffer);
}

char *cTextFieldData::GetQueryFragment() {
  const char *name = GetName();

  if (name && *name && valuebuffer) {
    char *encoded = URLencode(valuebuffer);
    cString tmp = cString::sprintf("%s,%s", name, encoded);
    free(encoded);
    return strdup(tmp);
  }

  return NULL;
}

char *cTextFieldData::GetValue() {
  return valuebuffer;
}

int cTextFieldData::GetLength() {
  return valuebufferlength;
}

// --- cItemListData -------------------------------------------------------

cItemListData::cItemListData(const char *Name, char **Strings, char **StringValues, int NumStrings) 
: cQueryData(Name)
{
  strings = Strings;
  stringvalues = StringValues;
  numstrings = NumStrings;
  value = 0;
}

cItemListData::~cItemListData() {
  for (int i=0; i < numstrings; i++) {
    free(strings[i]);
    free(stringvalues[i]);
  }
  if (strings)
    free(strings);
  if (stringvalues)
    free(stringvalues);
}

char *cItemListData::GetQueryFragment() {
  const char *name = GetName();

  if (name && *name) {
    cString tmp = cString::sprintf("%s,%s", name, stringvalues[value]);
    return strdup(tmp);
  }

  return NULL;
}

char **cItemListData::GetStrings() {
  return strings;
}

char **cItemListData::GetStringValues() {
  return stringvalues;
}

int cItemListData::GetNumStrings() {
  return numstrings;
}

int *cItemListData::GetValuePtr() {
  return &value;
}

// --- cSubmissionButtonData -----------------------------------------------

cSubmissionButtonData::cSubmissionButtonData(
        const char *queryUrl, const cHistoryObject *currentPage)
{
  querybase = queryUrl ? strdup(queryUrl) : NULL;
  page = currentPage;
}

cSubmissionButtonData::~cSubmissionButtonData() {
  if (querybase)
    free(querybase);
  // do not free page
}

char *cSubmissionButtonData::GetURL() {
  if (!querybase)
    return NULL;

  char *querystr = (char *)malloc(sizeof(char)*(strlen(querybase)+2));
  strcpy(querystr, querybase);

  if (!page)
    return querystr;

  if (strchr(querystr, '?'))
    strcat(querystr, "&");
  else
    strcat(querystr, "?");

  int numparameters = 0;
  for (int i=0; i<page->QuerySize(); i++) {
    char *parameter = page->GetQueryFragment(i);
    if (parameter) {
      querystr = (char *)realloc(querystr, (strlen(querystr)+strlen(parameter)+8)*sizeof(char));
      if (i > 0)
	strcat(querystr, "&");
      strcat(querystr, "subst=");
      strcat(querystr, parameter);
      numparameters++;

      free(parameter);
    }
  }

  if (numparameters == 0) {
    // remove the '?' or '&' because no parameters were added to the url
    querystr[strlen(querystr)-1] = '\0';
  }

  return querystr;
}
