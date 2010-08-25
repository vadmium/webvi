/*
 * request.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_REQUEST_H
#define __WEBVIDEO_REQUEST_H

#include <vdr/tools.h>
#include <vdr/thread.h>
#include <libwebvi.h>
#include "buffer.h"

enum eRequestType { REQT_NONE, REQT_MENU, REQT_FILE, REQT_STREAM, REQT_TIMER };

class cFileDownloadRequest;
class cWebviTimer;

// --- cDownloadProgress ---------------------------------------------------

class cDownloadProgress {
private:
  const static int NAME_LEN = 128;

  char name[NAME_LEN];
  long downloaded;
  long total;
  int statusCode;
  cString statusPharse;
  cFileDownloadRequest *req;
  cMutex mutex;
public:
  cDownloadProgress();

  void AssociateWith(cFileDownloadRequest *request);
  void SetContentLength(long bytes);
  void SetTitle(const char *title);
  void Progress(long downloadedbytes);
  void MarkDone(int errorcode, cString pharse);
  bool IsFinished();

  cString GetTitle();
  cString GetPercentage();
  cString GetStatusPharse();
  bool Error();
  cFileDownloadRequest *GetRequest() { return req; }
};

// --- cProgressVector -----------------------------------------------------

class cProgressVector : public cVector<cDownloadProgress *> {
public:
  cDownloadProgress *NewDownload();
};

// --- cMenuRequest ----------------------------------------------------

class cMenuRequest {
private:
  int reqID;
  bool aborted;
  bool finished;
  int status;
  cString statusPharse;

protected:
  WebviCtx webvi;
  WebviHandle handle;
  char *wvtref;
  cMemoryBuffer inBuffer;
  cWebviTimer *timer;

  virtual ssize_t WriteData(const char *ptr, size_t len);
  virtual WebviHandle PrepareHandle();
  static ssize_t WriteCallback(const char *ptr, size_t len, void *request);

  char *ExtractSiteName(const char *ref);
  void AppendQualityParamsToRef();

public:
  cMenuRequest(int ID, const char *wvtreference);
  virtual ~cMenuRequest();

  int GetID() { return reqID; }
  WebviHandle GetHandle() { return handle; }
  const char *GetReference() { return wvtref; }

  bool Start(WebviCtx webvictx);
  virtual void RequestDone(int errorcode, cString pharse);
  bool IsFinished() { return finished; }
  virtual void Abort();
  bool IsAborted() { return aborted; }

  // Return true if the lastest status code indicates success.
  bool Success();
  // Return the status code
  int GetStatusCode() { return status; }
  // Return the response pharse
  cString GetStatusPharse();

  virtual eRequestType GetType() { return REQT_MENU; }

  // Return the content of the reponse message
  virtual cString GetResponse();

  void SetTimer(cWebviTimer *t) { timer = t; }
  cWebviTimer *GetTimer() { return timer; }

  virtual int File() { return -1; }
  virtual bool Read() { return true; }
};

// --- cFileDownloadRequest ------------------------------------------------

class cFileDownloadRequest : public cMenuRequest {
private:
  enum eDownloadState { STATE_WEBVI, STATE_POSTPROCESS, STATE_FINISHED };

  char *title;
  long bytesDownloaded;
  long contentLength;
  cUnbufferedFile *destfile;
  char *destfilename;
  cDownloadProgress *progressUpdater;
  cPipe postProcessPipe;
  eDownloadState state;

protected:
  virtual WebviHandle PrepareHandle();
  virtual ssize_t WriteData(const char *ptr, size_t len);
  bool OpenDestFile();
  char *GetExtension(const char *contentType, const char *url);
  void StartPostProcessing();

public:
  cFileDownloadRequest(int ID, const char *streamref, 
                       cDownloadProgress *progress);
  virtual ~cFileDownloadRequest();

  eRequestType GetType() { return REQT_FILE; }
  void RequestDone(int errorcode, cString pharse);
  void Abort();

  int File();
  bool Read();
};

// --- cStreamUrlRequest ---------------------------------------------------

class cStreamUrlRequest : public cMenuRequest {
protected:
  virtual WebviHandle PrepareHandle();

public:
  cStreamUrlRequest(int ID, const char *ref);

  eRequestType GetType() { return REQT_STREAM; }
};

// --- cTimerRequest -------------------------------------------------------

class cTimerRequest : public cMenuRequest {
public:
  cTimerRequest(int ID, const char *ref);

  eRequestType GetType() { return REQT_TIMER; }
};

// --- cRequestVector ------------------------------------------------------

class cRequestVector : public cVector<cMenuRequest *> {
public:
  cRequestVector(int Allocated = 10) : cVector<cMenuRequest *>(Allocated) {}

  cMenuRequest *FindByHandle(WebviHandle handle);
};

#endif
