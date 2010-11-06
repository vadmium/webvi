/*
 * timer.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_TIMER_H
#define __WEBVIDEO_TIMER_H

#include <time.h>
#include <stdio.h>
#include <vdr/tools.h>
#include "request.h"

#define REQ_ID_TIMER -2
#define DEFAULT_TIMER_INTERVAL 7*24*60*60
#define RETRY_TIMER_INTERVAL 60*60
#define MIN_TIMER_INTERVAL 10*60
#define MAX_TIMER_HISTORY_SIZE 2000

class cWebviTimerManager;

// --- cWebviTimer -----------------------------------------------

class cWebviTimer : public cListObject {
private:
  int id;
  char *title;
  char *reference;

  time_t lastUpdate;
  int interval;

  bool running;
  cStringList activeStreams;
  bool lastSucceeded;
  char *lastError;

  cWebviTimerManager *parent;

public:
  cWebviTimer(int ID, const char *title, const char *ref,
              cWebviTimerManager *manager,
              time_t last=0, int interval=DEFAULT_TIMER_INTERVAL,
              bool success=true, const char *errmsg=NULL);
  ~cWebviTimer();

  int GetID() const { return id; }
  void SetTitle(const char *newTitle);
  const char *GetTitle() const { return title; }
  void SetInterval(int interval);
  int GetInterval() const;
  const char *GetReference() const { return reference; }

  time_t LastUpdate() const { return lastUpdate; }
  time_t NextUpdate() const;

  void SetError(const char *errmsg);
  bool Success() const { return lastSucceeded; }
  const char *LastError() const;

  void Execute();
  bool Running() { return running; }
  void DownloadStreams(const char *menuxml, cProgressVector& summaries);
  void CheckFailed(const char *errmsg);
  void RequestFinished(const char *ref, const char *errmsg);
};

// --- cWebviTimerManager ----------------------------------------

class cWebviTimerManager {
private:
  cList<cWebviTimer> timers;
  int nextID;
  cStringList refHistory;
  bool modified;
  bool disableSaving;
  bool convertTemplatePaths;

  cWebviTimerManager();
  ~cWebviTimerManager() {};
  cWebviTimerManager(const cWebviTimerManager &);             // intentionally undefined
  cWebviTimerManager &operator=(const cWebviTimerManager &);  // intentionally undefined

  void LoadTimers(FILE *f);
  void LoadHistory(FILE *f);
  void SaveTimers(FILE *f, const char *version);
  void SaveHistory(FILE *f);
 
  char *UpgradedTemplatePath(char *ref);
  void ConvertTimerHistoryTemplates();

public:
  static cWebviTimerManager &Instance();

  bool Load(const char *path);
  bool Save(const char *path, const char *version);

  cWebviTimer *Create(const char *title, const char *reference,
                      bool getExisting=true);
  void Remove(cWebviTimer *timer);
  cWebviTimer *First() const { return timers.First(); }
  cWebviTimer *Next(const cWebviTimer *cur) const { return timers.Next(cur); }
  cWebviTimer *GetLinear(int idx) const { return timers.Get(idx); }
  cWebviTimer *GetByID(int id) const;
  void SetModified() { modified = true; }

  void Update();
  void MarkDownloaded(const char *ref);
  bool AlreadyDownloaded(const char *ref);
};

#endif
