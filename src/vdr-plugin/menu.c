/*
 * menu.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdlib.h>
#include <time.h>
#include <vdr/skins.h>
#include <vdr/tools.h>
#include <vdr/i18n.h>
#include <vdr/osdbase.h>
#include <vdr/skins.h>
#include <vdr/font.h>
#include <vdr/osd.h>
#include <vdr/interface.h>
#include "menu.h"
#include "download.h"
#include "config.h"
#include "common.h"
#include "history.h"
#include "timer.h"
#include "menu_timer.h"

cCharSetConv csc = cCharSetConv("UTF-8", cCharSetConv::SystemCharacterTable());
struct MenuPointers menuPointers;

// --- cXMLMenu --------------------------------------------------

cXMLMenu::cXMLMenu(const char *Title, int c0, int c1, int c2,
				       int c3, int c4)
: cOsdMenu(Title, c0, c1, c2, c3, c4)
{
}

bool cXMLMenu::Deserialize(const char *xml) {
  xmlDocPtr doc = xmlParseMemory(xml, strlen(xml));
  if (!doc) {
    xmlErrorPtr xmlerr =  xmlGetLastError();
    if (xmlerr) {
      error("libxml error: %s", xmlerr->message);
    }

    return false;
  }

  xmlNodePtr node = xmlDocGetRootElement(doc);
  if (node)
    node = node->xmlChildrenNode;

  while (node) {
    if (node->type == XML_ELEMENT_NODE) {
      if (!CreateItemFromTag(doc, node)) {
        warning("Failed to parse menu tag: %s", (char *)node->name);
      }
    }
    node = node->next;
  }

  xmlFreeDoc(doc);
  return true;
}

int cXMLMenu::Load(const char *xmlstr) {
  Clear();
  Deserialize(xmlstr);

  return 0;
}


// --- cNavigationMenu -----------------------------------------------------

cNavigationMenu::cNavigationMenu(cHistory *History,
                                 cProgressVector& dlsummaries)
  : cXMLMenu("", 25), summaries(dlsummaries)
{
  title = NULL;
  reference = NULL;
  shortcutMode = 0;
  history = History;
  UpdateHelp();
}

cNavigationMenu::~cNavigationMenu() {
  menuPointers.navigationMenu = NULL;
  Clear();
  if (reference)
    free(reference);
}

bool cNavigationMenu::CreateItemFromTag(xmlDocPtr doc, xmlNodePtr node) {
  if (!xmlStrcmp(node->name, BAD_CAST "link")) {
    NewLinkItem(doc, node);
    return true;
  } else if (!xmlStrcmp(node->name, BAD_CAST "textfield")) {
    NewTextField(doc, node);
    return true;
  } else if (!xmlStrcmp(node->name, BAD_CAST "itemlist")) {
    NewItemList(doc, node);
    return true;
  } else if (!xmlStrcmp(node->name, BAD_CAST "textarea")) {
    NewTextArea(doc, node);
    return true;
  } else if (!xmlStrcmp(node->name, BAD_CAST "button")) {
    NewButton(doc, node);
    return true;
  } else if (!xmlStrcmp(node->name, BAD_CAST "title")) {
    NewTitle(doc, node);
    return true;
  }

  return false;
}

void cNavigationMenu::AddLinkItem(cOsdItem *item,
                                  cLinkBase *ref, 
                                  cLinkBase *streamref) {
  Add(item);

  if (ref)
    links.Append(ref);
  else
    links.Append(NULL);

  if (streamref)
    streams.Append(streamref);
  else
    streams.Append(NULL);
}

void cNavigationMenu::NewLinkItem(xmlDocPtr doc, xmlNodePtr node) {
  // label, ref and object tags
  xmlChar *itemtitle = NULL, *ref = NULL, *streamref = NULL;

  node = node->xmlChildrenNode;
  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "label")) {
      if (itemtitle)
        xmlFree(itemtitle);
      itemtitle = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    } else if (!xmlStrcmp(node->name, BAD_CAST "ref")) {
      if (ref)
        xmlFree(ref);
      ref = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    } else if (!xmlStrcmp(node->name, BAD_CAST "stream")) {
      if (streamref)
        xmlFree(streamref);
      streamref = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    }
    node = node->next;
  }
  if (!itemtitle)
    itemtitle = xmlCharStrdup("???");

  const char *titleconv = csc.Convert((char *)itemtitle);
  cOsdItem *item = new cOsdItem(titleconv);
  cSimpleLink *objlinkdata = NULL;
  cSimpleLink *linkdata = NULL;
  if (ref)
    linkdata = new cSimpleLink((char *)ref);
  if (streamref) {
    // media object
    objlinkdata = new cSimpleLink((char *)streamref);
  } else {
    // navigation link
    char *bracketed = (char *)malloc((strlen(titleconv)+3)*sizeof(char));
    if (bracketed) {
      bracketed[0] = '\0';
      strcat(bracketed, "[");
      strcat(bracketed, titleconv);
      strcat(bracketed, "]");
      item->SetText(bracketed, false);
    }
  }
  AddLinkItem(item, linkdata, objlinkdata);

  xmlFree(itemtitle);
  if (ref)
    xmlFree(ref);
  if (streamref)
    xmlFree(streamref);
}

void cNavigationMenu::NewTextField(xmlDocPtr doc, xmlNodePtr node) {
  // name attribute
  xmlChar *name = xmlGetProp(node, BAD_CAST "name");
  cHistoryObject *curhistpage = history->Current();

  // label tag
  xmlChar *text = NULL;
  node = node->xmlChildrenNode;
  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "label")) {
      if (text)
        xmlFree(text);
      text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    }
    node = node->next;
  }
  if (!text)
    text = xmlCharStrdup("???");

  cTextFieldData *data = curhistpage->GetTextFieldData((char *)name);
  cMenuEditStrItem *item = new cMenuEditStrItem(csc.Convert((char *)text), 
                                                data->GetValue(), 
                                                data->GetLength());
  AddLinkItem(item, NULL, NULL);

  free(text);
  if (name)
    xmlFree(name);
}

void cNavigationMenu::NewItemList(xmlDocPtr doc, xmlNodePtr node) {
  // name attribute
  xmlChar *name = xmlGetProp(node, BAD_CAST "name");
  cHistoryObject *curhistpage = history->Current();

  // label and item tags
  xmlChar *text = NULL;
  cStringList items;
  cStringList itemvalues;
  node = node->xmlChildrenNode;
  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "label")) {
      if (text)
        xmlFree(text);
      text = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    } else if (!xmlStrcmp(node->name, BAD_CAST "item")) {
      xmlChar *str = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
      if (!str)
        str = xmlCharStrdup("???");
      xmlChar *strvalue = xmlGetProp(node, BAD_CAST "value");
      if (!strvalue)
        strvalue = xmlCharStrdup("");

      items.Append(strdup((char *)str));
      itemvalues.Append(strdup((char *)strvalue));

      xmlFree(str);
      xmlFree(strvalue);
    }
    node = node->next;
  }
  if (!text)
    text = xmlCharStrdup("???");

  cItemListData *data = curhistpage->GetItemListData((const char *)name,
                                                     items,
                                                     itemvalues);

  cMenuEditStraItem *item = new cMenuEditStraItem(csc.Convert((char *)text), 
                                                  data->GetValuePtr(), 
                                                  data->GetNumStrings(), 
                                                  data->GetStrings());
  AddLinkItem(item, NULL, NULL);

  xmlFree(text);
  if (name)
    xmlFree(name);
}

void cNavigationMenu::NewTextArea(xmlDocPtr doc, xmlNodePtr node) {
  // label tag
  xmlChar *itemtitle = NULL;
  node = node->xmlChildrenNode;
  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "label")) {
      if (itemtitle)
        xmlFree(itemtitle);
      itemtitle = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    }
    node = node->next;
  }
  if (!itemtitle)
    return;

  const cFont *font = cFont::GetFont(fontOsd);
  cTextWrapper tw(csc.Convert((char *)itemtitle), font, cOsd::OsdWidth());
  for (int i=0; i < tw.Lines(); i++) {
    AddLinkItem(new cOsdItem(tw.GetLine(i), osUnknown, false), NULL, NULL);
  }

  xmlFree(itemtitle);
}

void cNavigationMenu::NewButton(xmlDocPtr doc, xmlNodePtr node) {
  // label and submission tags
  xmlChar *itemtitle = NULL, *submission = NULL;
  cHistoryObject *curhistpage = history->Current();

  node = node->xmlChildrenNode;
  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "label")) {
      if (itemtitle)
        xmlFree(itemtitle);
      itemtitle = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    } else if (!xmlStrcmp(node->name, BAD_CAST "submission")) {
      if (submission)
        xmlFree(submission);
      submission = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    }
    node = node->next;
  }
  if (!itemtitle)
    itemtitle = xmlCharStrdup("???");

  cSubmissionButtonData *data = \
          new cSubmissionButtonData((char *)submission, curhistpage);
  const char *titleconv = csc.Convert((char *)itemtitle); // do not free
  char *newtitle = (char *)malloc((strlen(titleconv)+3)*sizeof(char));
  if (newtitle) {
    newtitle[0] = '\0';
    strcat(newtitle, "[");
    strcat(newtitle, titleconv);
    strcat(newtitle, "]");

    cOsdItem *item = new cOsdItem(newtitle);
    AddLinkItem(item, data, NULL);
    free(newtitle);
  }

  xmlFree(itemtitle);
  if (submission)
    xmlFree(submission);
}

void cNavigationMenu::NewTitle(xmlDocPtr doc, xmlNodePtr node) {
  xmlChar *newtitle = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
  if (newtitle) {
    const char *conv = csc.Convert((char *)newtitle);
    SetTitle(conv);
    if (title)
      free(title);
    title = strdup(conv);
    xmlFree(newtitle);
  }
}

eOSState cNavigationMenu::ProcessKey(eKeys Key)
{
  cWebviTimer *timer;
  bool hasStreams;
  int old = Current();
  eOSState state = cXMLMenu::ProcessKey(Key);
  bool validItem = Current() >= 0 && Current() < links.Size();

  if (HasSubMenu())
    return state;

  if (state == osUnknown) {
    switch (Key) {
    case kInfo:
      // The alternative link is active only when object links are
      // present.
      if (validItem && streams.At(Current()))
        state = Select(links.At(Current()), LT_REGULAR);
      break;

    case kOk:
      // Primary action: download media object or, if not a media
      // link, follow the navigation link.
      if (validItem) {
        if (streams.At(Current()))
          state = Select(streams.At(Current()), LT_MEDIA);
        else
          state = Select(links.At(Current()), LT_REGULAR);
      }
      break;

    case kRed:
      if (shortcutMode == 0) {
        state = HistoryBack();
      } else {
        menuPointers.statusScreen = new cStatusScreen(summaries);
        state = AddSubMenu(menuPointers.statusScreen);
      }
      break;

    case kGreen:
      if (shortcutMode == 0) {
        state = HistoryForward();
      } else {
        return AddSubMenu(new cWebviTimerListMenu(cWebviTimerManager::Instance()));
      }
      break;

    case kYellow:
      if (shortcutMode == 0) {
        hasStreams = false;
        for (int i=0; i < streams.Size(); i++) {
          if (streams[i]) {
            hasStreams = true;
            break;
          }
        }

        if (hasStreams || Interface->Confirm(tr("No streams on this page, create timer anyway?"))) {
          timer = cWebviTimerManager::Instance().Create(title, reference);
          if (timer)
            return AddSubMenu(new cEditWebviTimerMenu(*timer, true, false));
        }

        state = osContinue;
      }
      break;

    case kBlue:
      if (shortcutMode == 0) {
        // Secondary action: start streaming if a media object
        if (validItem && streams.At(Current()))
          state = Select(streams.At(Current()), LT_STREAMINGMEDIA);
      }
      break;

    case k0:
      shortcutMode = shortcutMode == 0 ? 1 : 0;
      UpdateHelp();
      break;

    default:
      break;
    }
  } else {
    // If the key press caused the selected item to change, we need to
    // update the help texts.
    //
    // In cMenuEditStrItem key == kOk with state == osContinue
    // indicates leaving the edit mode. We want to update the help
    // texts in this case also.
    if ((old != Current()) || 
        ((Key == kOk) && (state == osContinue))) {
      UpdateHelp();
    }
  }

  return state;
}

eOSState cNavigationMenu::Select(cLinkBase *link, eLinkType type)
{
  if (!link) {
    return osContinue;
  }
  char *ref = link->GetURL();
  if (!ref) {
    error("link->GetURL() == NULL in cNavigationMenu::Select");
    return osContinue;
  }

  if (type == LT_MEDIA) {
    cDownloadProgress *progress = summaries.NewDownload();
    cFileDownloadRequest *req = \
      new cFileDownloadRequest(history->Current()->GetID(), ref,
			       progress);
    cWebviThread::Instance().AddRequest(req);

    Skins.Message(mtInfo, tr("Downloading in the background"));
  } else if (type == LT_STREAMINGMEDIA) {
    cWebviThread::Instance().AddRequest(new cStreamUrlRequest(history->Current()->GetID(),
						     ref));
    Skins.Message(mtInfo, tr("Starting player..."));
    return osEnd;
  } else {
    cWebviThread::Instance().AddRequest(new cMenuRequest(history->Current()->GetID(),
                                                ref));
    Skins.Message(mtStatus, tr("Retrieving..."));
  }

  return osContinue;
}

void cNavigationMenu::Clear(void) {
  cXMLMenu::Clear();
  SetTitle("");
  if (title)
    free(title);
  title = NULL;
  for (int i=0; i < links.Size(); i++) {
    if (links[i])
      delete links[i];
    if (streams[i])
      delete streams[i];
  }
  links.Clear();
  streams.Clear();
}

void cNavigationMenu::Populate(const cHistoryObject *page, const char *statusmsg) {
  Load(page->GetOSD());
  
  if (reference)
    free(reference);
  reference = strdup(page->GetReference());

  // Make sure that an item is selected (if there is at least
  // one). The help texts are not updated correctly if no item is
  // selected.

  SetCurrent(Get(page->GetSelected()));
  UpdateHelp();
  SetStatus(statusmsg);
}

eOSState cNavigationMenu::HistoryBack() {
  cHistoryObject *cur = history->Current();

  if (cur)
    cur->RememberSelected(Current());

  cHistoryObject *page = history->Back();
  if (page) {
    Populate(page);
    Display();
  }
  return osContinue;
}

eOSState cNavigationMenu::HistoryForward() {
  cHistoryObject *before = history->Current();
  cHistoryObject *after = history->Forward();

  if (before)
    before->RememberSelected(Current());

  // Update only if the menu really changed
  if (before != after) {
    Populate(after);
    Display();
  }
  return osContinue;
}

void cNavigationMenu::UpdateHelp() {
  const char *red = NULL;
  const char *green = NULL;
  const char *yellow = NULL;
  const char *blue = NULL;

  if (shortcutMode == 0) {
    red = (history->Current() != history->First()) ? tr("Back") : NULL;
    green = (history->Current() != history->Last()) ? tr("Forward") : NULL;
    yellow = (Current() >= 0) ? tr("Create timer") : NULL;
    blue = ((Current() >= 0) && (streams.At(Current()))) ? tr("Play") : NULL;
  } else {
    red = tr("Status");
    green = tr("Timers");
  }

  SetHelp(red, green, yellow, blue);
}

// --- cStatusScreen -------------------------------------------------------

cStatusScreen::cStatusScreen(cProgressVector& dlsummaries)
  : cOsdMenu(tr("Unfinished downloads"), 40), summaries(dlsummaries)
{
  int charsperline = cOsd::OsdWidth() / cFont::GetFont(fontOsd)->Width('M');
  SetCols(charsperline-5);

  UpdateHelp();
  Update();
}

cStatusScreen::~cStatusScreen() {
  menuPointers.statusScreen = NULL;
}

void cStatusScreen::Update() {
  int c = Current();

  Clear();

  if (summaries.Size() == 0) {
    SetTitle(tr("No active downloads"));
  } else {

    for (int i=0; i<summaries.Size(); i++) {
      cString dltitle;
      cDownloadProgress *s = summaries[i];
      dltitle = cString::sprintf("%s\t%s",
				 (const char *)s->GetTitle(),
				 (const char *)s->GetPercentage());

      Add(new cOsdItem(dltitle));
    }

    if (c >= 0)
      SetCurrent(Get(c));
  }

  lastupdate = time(NULL);

  UpdateHelp();
  Display();
}

bool cStatusScreen::NeedsUpdate() {
  return (Count() > 0) && (time(NULL) - lastupdate >= updateInterval);
}

eOSState cStatusScreen::ProcessKey(eKeys Key) {
  cFileDownloadRequest *req;
  int old = Current();
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (HasSubMenu())
    return state;

  if (state == osUnknown) {
    switch (Key) {
    case kYellow:
      if ((Current() >= 0) && (Current() < summaries.Size())) {
	if (summaries[Current()]->IsFinished()) {
	  delete summaries[Current()];
	  summaries.Remove(Current());
	  Update();
	} else if ((req = summaries[Current()]->GetRequest()) && 
		   !req->IsFinished()) {
	  req->Abort();
          Update();
	}
      }
      return osContinue;

    case kOk:
    case kInfo:
      if (summaries[Current()]->Error()) {
        cString msg = cString::sprintf("%s\n%s: %s",
                      (const char *)summaries[Current()]->GetTitle(),
                      tr("Error"),
                      (const char *)summaries[Current()]->GetStatusPharse());
        return AddSubMenu(new cMenuText(tr("Error details"), msg));
      } else {
        cString msg = cString::sprintf("%s (%s)",
                      (const char *)summaries[Current()]->GetTitle(),
                      (const char *)summaries[Current()]->GetPercentage());
        return AddSubMenu(new cMenuText(tr("Download details"), msg));
      }

      return osContinue;

    default:
      break;
    }
  } else {
    // Update help if the key press caused the menu item to change.
    if (old != Current())
      UpdateHelp();
  }

  return state;
}

void cStatusScreen::UpdateHelp() {
  bool remove = false;
  if ((Current() >= 0) && (Current() < summaries.Size())) {
    if (summaries[Current()]->IsFinished()) {
      remove = true;
    }
  }

  const char *yellow = remove ? tr("Remove") : tr("Abort");

  SetHelp(NULL, NULL, yellow, NULL);
}
