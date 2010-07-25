/*
 * timer.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <string.h>
#include <errno.h>
#include <libxml/parser.h>
#include "timer.h"
#include "request.h"
#include "common.h"
#include "download.h"
#include "config.h"

// --- cWebviTimer -----------------------------------------------

cWebviTimer::cWebviTimer(int ID, const char *title,
                         const char *ref, cWebviTimerManager *manager,
                         time_t last, int interval, bool success,
                         const char *errmsg)
  : id(ID), title(title ? strdup(title) : strdup("???")),
    reference(ref ? strdup(ref) : NULL), lastUpdate(last),
    interval(interval), running(false), lastSucceeded(success),
    lastError(errmsg ? strdup(errmsg) : NULL),
    parent(manager)
{
}

cWebviTimer::~cWebviTimer() {
  if(title)
    free(title);
  if (reference)
    free(reference);
  if (lastError)
    free(lastError);
}

void cWebviTimer::SetTitle(const char *newTitle) {
  if (title)
    free(title);
  title = newTitle ? strdup(newTitle) : strdup("???");

  parent->SetModified();
}

void cWebviTimer::SetInterval(int interval) {
  if (interval < MIN_TIMER_INTERVAL)
    this->interval = MIN_TIMER_INTERVAL;
  else
    this->interval = interval;

  parent->SetModified();
}

int cWebviTimer::GetInterval() const {
  return interval;
}

time_t cWebviTimer::NextUpdate() const {
  int delta = interval;

  // Retry again soon if the last try failed
  if (!lastSucceeded && delta > RETRY_TIMER_INTERVAL)
    delta = RETRY_TIMER_INTERVAL;

  return lastUpdate + delta;
}

void cWebviTimer::Execute() {
  if (running) {
    debug("previous instance of this timer is still running");
    return;
  }

  info("Executing timer \"%s\"", title);

  running = true;
  cTimerRequest *req = new cTimerRequest(id, reference);
  req->SetTimer(this);
  cWebviThread::Instance().AddRequest(req);

  lastUpdate = time(NULL);
  SetError(NULL);
  parent->SetModified();

  activeStreams.Clear();
}

void cWebviTimer::SetError(const char *errmsg) {
  bool oldSuccess = lastSucceeded;

  if (lastError)
    free(lastError);
  lastError = NULL;

  if (errmsg) {
    lastSucceeded = false;
    lastError = strdup(errmsg);
  } else {
    lastSucceeded = true;
  }

  if (oldSuccess != lastSucceeded)
    parent->SetModified();
}

const char *cWebviTimer::LastError() const {
  return lastError ? lastError : "";
}

void cWebviTimer::DownloadStreams(const char *menuxml, cProgressVector& summaries) {
  if (!menuxml) {
    SetError("xml == NULL");
    return;
  }

  xmlDocPtr doc = xmlParseMemory(menuxml, strlen(menuxml));
  if (!doc) {
    xmlErrorPtr xmlerr = xmlGetLastError();
    if (xmlerr)
      error("libxml error: %s", xmlerr->message);
    SetError(xmlerr->message);
    return;
  }

  xmlNodePtr node = xmlDocGetRootElement(doc);
  if (node)
    node = node->xmlChildrenNode;

  while (node) {
    if (!xmlStrcmp(node->name, BAD_CAST "link")) {
      xmlNodePtr node2 = node->children;

      while(node2) {
        if (!xmlStrcmp(node2->name, BAD_CAST "stream")) {
          xmlChar *streamref = xmlNodeListGetString(doc, node2->xmlChildrenNode, 1);
          const char *ref = (const char *)streamref;

          if (parent->AlreadyDownloaded(ref)) {
            debug("timer: %s has already been downloaded", ref);
          } else if (*ref) {
            info("timer: downloading %s", ref);
            
            activeStreams.Append(strdup(ref));
            cFileDownloadRequest *req = \
              new cFileDownloadRequest(REQ_ID_TIMER, ref,
                                       webvideoConfig->GetDownloadPath(),
                                       summaries.NewDownload());
            req->SetTimer(this);
            cWebviThread::Instance().AddRequest(req);
          }

          xmlFree(streamref);
        }

        node2 = node2->next;
      }
    }

    node = node->next;
  }

  xmlFreeDoc(doc);

  if (activeStreams.Size() == 0) {
    running = false;
  }
}

void cWebviTimer::CheckFailed(const char *errmsg) {
  SetError(errmsg);
  running = false;
}

void cWebviTimer::RequestFinished(const char *ref, const char *errmsg) {
  if (errmsg && !lastError)
    SetError(errmsg);

  if (ref) {
    if (!errmsg && parent)
      parent->MarkDownloaded(ref);

    int i = activeStreams.Find(ref);
    if (i != -1) {
      free(activeStreams[i]);
      activeStreams.Remove(i);
    }
  }

  if (activeStreams.Size() == 0) {
    info("timer \"%s\" done", title);
    running = false;
  } else {
    debug("timer %s is still downloading %d streams", reference, activeStreams.Size());
  }
}

// --- cWebviTimerManager ----------------------------------------

cWebviTimerManager::cWebviTimerManager()
  : nextID(1), modified(false), disableSaving(false)
{
}

cWebviTimerManager &cWebviTimerManager::Instance() {
  static cWebviTimerManager instance;

  return instance;
}

void cWebviTimerManager::LoadTimers(FILE *f) {
  cReadLine rl;
  long lastRefresh;
  int interval;
  int success;
  char *ref;
  const char *ver;
  const char *title;
  const char *errmsg;
  int n, i;

  ver = rl.Read(f);
  if (strcmp(ver, "# WVTIMER1") != 0) {
    error("Can't load timers. Unknown format: %s", ver);
    disableSaving = true;
    return;
  }

  i = 1;
  while (true) {
    n = fscanf(f, "%ld %d %d %ms", &lastRefresh, &interval, &success, &ref);
    if (n != 4) {
      if (n != EOF) {
        error("Error while reading webvi timers file");
      } else if (ferror(f)) {
        LOG_ERROR_STR("webvi timers file");
      }

      break;
    }

    title = rl.Read(f);
    title = title ? skipspace(title) : "???";
    errmsg = success ? NULL : "";

    info("timer %d:  title %s", i++, title);
    debug("  ref %s, lastRefresh %ld, interval %d", ref, lastRefresh, interval);

    timers.Add(new cWebviTimer(nextID++, title, ref, this,
			       (time_t)lastRefresh, interval,
			       success, errmsg));

    free(ref);
  }
}

void cWebviTimerManager::LoadHistory(FILE *f) {
  cReadLine rl;
  char *line;

  while ((line = rl.Read(f)))
    refHistory.Append(strdup(line));

  debug("loaded history: len = %d", refHistory.Size());
}

void cWebviTimerManager::SaveTimers(FILE *f) {
  // Format: space separated field in this order:
  // lastUpdate interval lastSucceeded reference title

  fprintf(f, "# WVTIMER1\n");

  cWebviTimer *t = timers.First();
  while (t) {
    if (fprintf(f, "%ld %d %d %s %s\n",
                t->LastUpdate(), t->GetInterval(), t->Success(),
                t->GetReference(), t->GetTitle()) < 0) {
      error("Failed to save timer data!");
    }
    
    t = timers.Next(t);
  }
}

void cWebviTimerManager::SaveHistory(FILE *f) {
  int size = refHistory.Size();
  int first;

  if (size <= MAX_TIMER_HISTORY_SIZE)
    first = 0;
  else
    first = size - MAX_TIMER_HISTORY_SIZE;

  for (int i=first; i<size; i++) {
    const char *ref = refHistory[i];
    if (fwrite(ref, strlen(ref), 1, f) != 1 ||
        fwrite("\n", 1, 1, f) != 1) {
      error("Error while writing timer history");
      break;
    }
  }
}

bool cWebviTimerManager::Load(const char *path) {
  FILE *f;
  bool ok = true;

  cString timersname = AddDirectory(path, "timers.dat");
  f = fopen(timersname, "r");
  if (f) {
    debug("loading webvi timers from %s", (const char *)timersname);
    LoadTimers(f);
    fclose(f);
  } else {
    if (errno != ENOENT)
      LOG_ERROR_STR("Can't load webvi timers");
    ok = false;
  }

  cString historyname = AddDirectory(path, "timers.hst");
  f = fopen(historyname, "r");
  if (f) {
    debug("loading webvi history from %s", (const char *)historyname);
    LoadHistory(f);
    fclose(f);
  } else {
    if (errno != ENOENT)
      LOG_ERROR_STR("Can't load webvi timer history");
    ok = false;
  }
  
  return ok;
}

bool cWebviTimerManager::Save(const char *path) {
  FILE *f;
  bool ok = true;

  if (!modified)
    return true;
  if (disableSaving) {
    error("Not saving timers because the file format is unknown.");
    return false;
  }

  cString timersname = AddDirectory(path, "timers.dat");
  f = fopen(timersname, "w");
  if (f) {
    debug("saving webvi timers to %s", (const char *)timersname);
    SaveTimers(f);
    fclose(f);
  } else {
    LOG_ERROR_STR("Can't save webvi timers");
    ok = false;
  }

  cString historyname = AddDirectory(path, "timers.hst");
  f = fopen(historyname, "w");
  if (f) {
    debug("saving webvi timer history to %s", (const char *)historyname);
    SaveHistory(f);
    fclose(f);
  } else {
    LOG_ERROR_STR("Can't save webvi timer history");
    ok = false;
  }

  modified = !ok;
  
  return ok;
}

void cWebviTimerManager::Update() {
  char timestr[25];
  cWebviTimer *timer = timers.First();
  if (!timer)
    return;

  time_t now = time(NULL);

#ifdef DEBUG
  strftime(timestr, 25, "%x %X", localtime(&now));
  debug("Running webvi timers update at %s", timestr);
#endif

  while (timer) {
    if (timer->NextUpdate() < now) {
      debug("%d. %s: launching now",
            timer->GetID(), timer->GetTitle());
      timer->Execute();
    } else {
#ifdef DEBUG
      time_t next = timer->NextUpdate();
      strftime(timestr, 25, "%x %X", localtime(&next));
      debug("%d. %s: next update at %s",
            timer->GetID(), timer->GetTitle(), timestr);
#endif
    }

    timer = timers.Next(timer);
  }
}

cWebviTimer *cWebviTimerManager::GetByID(int id) const {
  cWebviTimer *timer = timers.First();

  while (timer) {
    if (timer->GetID() == id)
      return timer;

    timer = timers.Next(timer);
  }

  return NULL;
}

cWebviTimer *cWebviTimerManager::Create(const char *title,
                                        const char *ref,
                                        bool getExisting) {
  cWebviTimer *t;

  if (!ref)
    return NULL;

  if (getExisting) {
    t = timers.First();
    while (t) {
      if (strcmp(t->GetReference(), ref) == 0) {
        return t;
      }

      t = timers.Next(t);
    }
  }

  t = new cWebviTimer(nextID++, title, ref, this);
  timers.Add(t);

  modified = true;

  return t;
}

void cWebviTimerManager::Remove(cWebviTimer *timer) {
  timers.Del(timer);
  modified = true;
}

void cWebviTimerManager::MarkDownloaded(const char *ref) {
  if (!ref)
    return;

  if (refHistory.Find(ref) == -1) {
    refHistory.Append(strdup(ref));
    modified = true;
  }
}

bool cWebviTimerManager::AlreadyDownloaded(const char *ref) {
  return refHistory.Find(ref) != -1;
}
