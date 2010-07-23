/*
 * download.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <vdr/tools.h>
#include "download.h"
#include "common.h"

// --- cWebviThread --------------------------------------------------------

cWebviThread::cWebviThread() {
  int pipefd[2];

  if (pipe(pipefd) == -1)
    LOG_ERROR_STR("new request pipe");
  newreqread = pipefd[0];
  newreqwrite = pipefd[1];
  //fcntl(newreqread, F_SETFL, O_NONBLOCK);
  //fcntl(newreqwrite, F_SETFL, O_NONBLOCK);

  webvi = webvi_initialize_context();
}

cWebviThread::~cWebviThread() {
  int numactive = activeRequestList.Size();
  for (int i=0; i<activeRequestList.Size(); i++)
    delete activeRequestList[i];
  activeRequestList.Clear();

  for (int i=0; i<finishedRequestList.Size(); i++) {
    delete finishedRequestList[i];
  }
  finishedRequestList.Clear();

  webvi_cleanup_context(webvi);

  if (numactive > 0) {
    esyslog("%d requests failed to complete", numactive);
  }
}

cWebviThread &cWebviThread::Instance() {
  static cWebviThread instance;
  
  return instance;
}

void cWebviThread::SetTemplatePath(const char *path) {
  if (webvi != 0 && path)
    webvi_set_config(webvi, WEBVI_CONFIG_TEMPLATE_PATH, path);
}

void cWebviThread::MoveToFinishedList(cMenuRequest *req) {
  // Move the request from the activeList to finishedList.
  requestMutex.Lock();
  for (int i=0; i<activeRequestList.Size(); i++) {
    if (activeRequestList[i] == req) {
      activeRequestList.Remove(i);
      break;
    }
  }
  finishedRequestList.Append(req);

  requestMutex.Unlock();
}

void cWebviThread::ActivateNewRequest() {
  // Move requests from newRequestList to activeRequestList and start
  // them.
  requestMutex.Lock();
  for (int i=0; i<newRequestList.Size(); i++) {
    cMenuRequest *req = newRequestList[i];
    if (req->IsAborted()) {
      // The request has been aborted even before we got a chance to
      // start it.
      MoveToFinishedList(req);
    } else {
      debug("starting request %d", req->GetID());

      if (!req->Start(webvi)) {
        error("Request failed to start");
        req->RequestDone(-1, "Request failed to start");
        MoveToFinishedList(req);
      } else {
        activeRequestList.Append(req);
      }
    }
  }

  newRequestList.Clear();
  requestMutex.Unlock();
}

void cWebviThread::StopFinishedRequests() {
  // Check if some requests have finished, and move them to
  // finishedRequestList.
  int msg_remaining;
  WebviMsg *donemsg;
  cMenuRequest *req;

  do {
    donemsg = webvi_get_message(webvi, &msg_remaining);

    if (donemsg && donemsg->msg == WEBVIMSG_DONE) {
      requestMutex.Lock();
      req = activeRequestList.FindByHandle(donemsg->handle);
      if (req) {
        debug("Finished request %d", req->GetID());
        req->RequestDone(donemsg->status_code, donemsg->data);
        MoveToFinishedList(req);
      }
      requestMutex.Unlock();
    }
  } while (msg_remaining > 0);
}

void cWebviThread::Stop() {
  // The thread may be sleeping, wake it up first.
  TEMP_FAILURE_RETRY(write(newreqwrite, "S", 1));
  Cancel(5);
}

void cWebviThread::Action(void) {
  fd_set readfds, writefds, excfds;
  int maxfd;
  struct timeval timeout;
  long running_handles;
  bool check_done = false;

  if (webvi == 0) {
    error("Failed to get libwebvi context");
    return;
  }

  while (Running()) {
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&excfds);
    webvi_fdset(webvi, &readfds, &writefds, &excfds, &maxfd);
    FD_SET(newreqread, &readfds);
    if (newreqread > maxfd)
      maxfd = newreqread;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int s = TEMP_FAILURE_RETRY(select(maxfd+1, &readfds, &writefds, NULL, 
                                      &timeout));
    if (s == -1) {
      // select error
      LOG_ERROR_STR("select() error in webvideo downloader thread:");
      Cancel(-1);

    } else if (s == 0) {
      // timeout
      webvi_perform(webvi, 0, WEBVI_SELECT_TIMEOUT, &running_handles);
      check_done = true;

    } else {
      for (int fd=0; fd<=maxfd; fd++) {
        if (FD_ISSET(fd, &readfds)) {
          if (fd == newreqread) {
            char tmpbuf[8];
            int n = read(fd, tmpbuf, 8);
            if (n > 0 && memchr(tmpbuf, 'S', n))
              Cancel(-1);
            ActivateNewRequest();
          } else {
            webvi_perform(webvi, fd, WEBVI_SELECT_READ, &running_handles);
            check_done = true;
          }
        }
        if (FD_ISSET(fd, &writefds))
          webvi_perform(webvi, fd, WEBVI_SELECT_WRITE, &running_handles);
        if (FD_ISSET(fd, &excfds))
          webvi_perform(webvi, fd, WEBVI_SELECT_EXCEPTION, &running_handles);
      }
    }

    if (check_done) {
      StopFinishedRequests();
      check_done = false;
    }
  }
}

void cWebviThread::AddRequest(cMenuRequest *req) {
  requestMutex.Lock();
  newRequestList.Append(req);
  requestMutex.Unlock();

  int s = TEMP_FAILURE_RETRY(write(newreqwrite, "*", 1));
  if (s == -1)
    LOG_ERROR_STR("Failed to signal new webvideo request");
}

cMenuRequest *cWebviThread::GetFinishedRequest() {
  cMenuRequest *res = NULL;
  requestMutex.Lock();
  if (finishedRequestList.Size() > 0) {
    res = finishedRequestList[finishedRequestList.Size()-1];
    finishedRequestList.Remove(finishedRequestList.Size()-1);
  }
  requestMutex.Unlock();

  return res;
}

int cWebviThread::GetUnfinishedCount() {
  if (!Running())
    return 0;
  else
    return activeRequestList.Size();
}
