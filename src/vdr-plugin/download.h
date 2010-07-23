/*
 * download.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_DOWNLOAD_H
#define __WEBVIDEO_DOWNLOAD_H

#include <vdr/thread.h>
#include <libwebvi.h>
#include "request.h"

// --- cWebviThread --------------------------------------------------------

class cWebviThread : public cThread {
private:
  WebviCtx webvi;
  cMutex requestMutex;
  cRequestVector activeRequestList;
  cRequestVector newRequestList;
  cRequestVector finishedRequestList;
  int newreqread, newreqwrite;

  void MoveToFinishedList(cMenuRequest *req);
  void ActivateNewRequest();
  void StopFinishedRequests();

protected:
  void Action(void);

public:
  cWebviThread();
  ~cWebviThread();

  static cWebviThread &Instance();

  // Stop the thread
  void Stop();
  // Set path to the site templates. Should be set before
  // Start()ing the thread.
  void SetTemplatePath(const char *path);
  // Start executing req. The control of req is handed over to the
  // downloader thread. The main thread should not access req until
  // the request is handed back to the main thread by
  // GetFinishedRequest().
  void AddRequest(cMenuRequest *req);
  // Return a request that has finished or NULL if no requests are
  // finished. The ownership of the returned cMenuRequest object
  // is again assigned to the main thread. The main thread should poll
  // this function periodically.
  cMenuRequest *GetFinishedRequest();
  // Returns the number download requests currectly active
  int GetUnfinishedCount();
};

#endif
