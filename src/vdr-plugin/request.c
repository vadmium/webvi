/*
 * request.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vdr/tools.h>
#include <vdr/i18n.h>
#include "request.h"
#include "common.h"
#include "mimetypes.h"
#include "config.h"
#include "timer.h"

// --- cDownloadProgress ---------------------------------------------------

cDownloadProgress::cDownloadProgress() {
  strcpy(name, "???");
  downloaded = -1;
  total = -1;
  statusCode = -1;
  req = NULL;
}

void cDownloadProgress::AssociateWith(cFileDownloadRequest *request) {
  req = request;
}

void cDownloadProgress::SetContentLength(long bytes) {
  total = bytes;
}

void cDownloadProgress::SetTitle(const char *title) {
  cMutexLock lock(&mutex);

  strncpy(name, title, NAME_LEN-1);
  name[NAME_LEN-1] = '\0';
}

void cDownloadProgress::Progress(long downloadedbytes) {
  // Atomic operation, no mutex needed
  downloaded = downloadedbytes;
}

void cDownloadProgress::MarkDone(int errorcode, cString pharse) {
  cMutexLock lock(&mutex);

  statusCode = errorcode;
  statusPharse = pharse;
}

bool cDownloadProgress::IsFinished() {
  return statusCode != -1;
}

cString cDownloadProgress::GetTitle() {
  cMutexLock lock(&mutex);

  if (req && req->IsAborted())
    return cString::sprintf("[%s] %s", tr("Aborted"), name);
  else
    return cString(name);
}

cString cDownloadProgress::GetPercentage() {
  cMutexLock lock(&mutex);

  if ((const char*)statusPharse != NULL && statusCode != 0)
    // TRANSLATORS: at most 5 characters
    return cString(tr("Error"));
  else if ((downloaded < 0) || (total < 0))
    return cString("???");
  else
    return cString::sprintf("%3d%%", (int) (100*(float)downloaded/total + 0.5));
}

cString cDownloadProgress::GetStatusPharse() {
  cMutexLock lock(&mutex);

  return statusPharse;
}

bool cDownloadProgress::Error() {
  return (const char *)statusPharse != NULL;
}

// --- cProgressVector -----------------------------------------------------

cDownloadProgress *cProgressVector::NewDownload() {
  cDownloadProgress *progress = new cDownloadProgress();
  Append(progress);
  return progress;
}

// --- cMenuRequest --------------------------------------------------------

cMenuRequest::cMenuRequest(int ID, const char *wvtreference)
: reqID(ID), aborted(false), finished(false), status(0), webvi(-1),
  handle(-1), timer(NULL)
{
  wvtref = strdup(wvtreference);
}

cMenuRequest::~cMenuRequest() {
  if (handle != -1) {
    if (!finished)
      Abort();
    webvi_delete_handle(webvi, handle);
  }

  // do not delete timer
}

ssize_t cMenuRequest::WriteCallback(const char *ptr, size_t len, void *request) {
  cMenuRequest *instance = (cMenuRequest *)request;
  if (instance)
    return instance->WriteData(ptr, len);
  else
    return len;
}

ssize_t cMenuRequest::WriteData(const char *ptr, size_t len) {
  return inBuffer.Put(ptr, len);
}

char *cMenuRequest::ExtractSiteName(const char *ref) {
  if (strncmp(ref, "wvt:///", 7) != 0)
    return NULL;

  const char *first = ref+7;
  const char *last = strchr(first, '/');
  if (!last)
    last = first+strlen(first);

  return strndup(first, last-first);
}

void cMenuRequest::AppendQualityParamsToRef() {
  if (!wvtref)
    return;

  char *site = ExtractSiteName(wvtref);
  if (site) {
    const char *min = webvideoConfig->GetMinQuality(site, GetType());
    const char *max = webvideoConfig->GetMaxQuality(site, GetType());
    free(site);

    if (min && !max) {
      cString newref = cString::sprintf("%s&minquality=%s", wvtref, min);
      free(wvtref);
      wvtref = strdup((const char *)newref);

    } else if (!min && max) {
      cString newref = cString::sprintf("%s&maxquality=%s", wvtref, max);
      free(wvtref);
      wvtref = strdup((const char *)newref);

    } else if (min && max) {
      cString newref = cString::sprintf("%s&minquality=%s&maxquality=%s", wvtref, min, max);
      free(wvtref);
      wvtref = strdup((const char *)newref);
    }
  }
}

WebviHandle cMenuRequest::PrepareHandle() {
  if (handle == -1) {
    handle = webvi_new_request(webvi, wvtref, WEBVIREQ_MENU);

    if (handle != -1) {
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEFUNC, WriteCallback);
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEDATA, this);
    }
  }

  return handle;
}

bool cMenuRequest::Start(WebviCtx webvictx) {
  webvi = webvictx;

  if ((PrepareHandle() != -1) && (webvi_start_handle(webvi, handle) == WEBVIERR_OK)) {
    finished = false;
    return true;
  } else 
    return false;
}

void cMenuRequest::RequestDone(int errorcode, cString pharse) {
  finished = true;
  status = errorcode;
  statusPharse = pharse;
}

void cMenuRequest::Abort() {
  if (finished || handle == -1)
    return;

  aborted = true;
  webvi_stop_handle(webvi, handle);
};

bool cMenuRequest::Success() {
  return status == 0;
}

cString cMenuRequest::GetStatusPharse() {
  return statusPharse;
}

cString cMenuRequest::GetResponse() {
  size_t len = inBuffer.Length();
  const char *src = inBuffer.Get();
  char *buf = (char *)malloc((len+1)*sizeof(char));
  strncpy(buf, src, len);
  buf[len] = '\0';
  return cString(buf, true);
}

// --- cFileDownloadRequest ------------------------------------------------

cFileDownloadRequest::cFileDownloadRequest(int ID, const char *streamref, 
                                           const char *destdir,
                                           cDownloadProgress *progress)
:  cMenuRequest(ID, streamref), title(NULL), bytesDownloaded(0),
   contentLength(-1), destfile(NULL), progressUpdater(progress)
{
  this->destdir = strdup(destdir);
  if (progressUpdater)
    progressUpdater->AssociateWith(this);

  AppendQualityParamsToRef();
}

cFileDownloadRequest::~cFileDownloadRequest() {
  if (destfile) {
    destfile->Close();
    delete destfile;
  }
  if (destdir)
    free(destdir);
  if (title)
    free(title);
  // do not delete progressUpdater
}

WebviHandle cFileDownloadRequest::PrepareHandle() {
  if (handle == -1) {
    handle = webvi_new_request(webvi, wvtref, WEBVIREQ_FILE);

    if (handle != -1) {
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEFUNC, WriteCallback);
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEDATA, this);
    }
  }

  return handle;
}

ssize_t cFileDownloadRequest::WriteData(const char *ptr, size_t len) {
  if (!destfile) {
    if (!OpenDestFile())
      return -1;
  }

  bytesDownloaded += len;
  if (progressUpdater)
    progressUpdater->Progress(bytesDownloaded);

  return destfile->Write(ptr, len);
}

bool cFileDownloadRequest::OpenDestFile() {
  char *contentType;
  char *url;
  char *ext;
  cString destfilename;
  int fd, i;

  if (handle == -1) {
    error("handle == -1 while trying to open destination file");
    return false;
  }

  if (destfile)
    delete destfile;

  destfile = new cUnbufferedFile;

  webvi_get_info(webvi, handle, WEBVIINFO_URL, &url);
  webvi_get_info(webvi, handle, WEBVIINFO_STREAM_TITLE, &title);
  webvi_get_info(webvi, handle, WEBVIINFO_CONTENT_TYPE, &contentType);
  webvi_get_info(webvi, handle, WEBVIINFO_CONTENT_LENGTH, &contentLength);

  if (!contentType || !url) {
    if(contentType)
      free(contentType);
    if (url)
      free(url);

    error("no content type or url, can't infer extension");
    return false;
  }

  ext = GetExtension(contentType, url);

  free(url);
  free(contentType);

  char *basename = strdup(title ? title : "???");
  basename = safeFilename(basename);

  i = 1;
  destfilename = cString::sprintf("%s/%s%s", destdir, basename, ext);
  while (true) {
    debug("trying to open %s", (const char *)destfilename);

    fd = destfile->Open(destfilename, O_WRONLY | O_CREAT | O_EXCL, DEFFILEMODE);

    if (fd == -1 && errno == EEXIST)
      destfilename = cString::sprintf("%s/%s-%d%s", destdir, basename, i++, ext);
    else
      break;
  };

  free(basename);
  free(ext);

  if (fd < 0) {
    error("Failed to open file %s: %m", (const char *)destfilename);
    delete destfile;
    destfile = NULL;
    return false;
  }

  info("Saving to %s", (const char *)destfilename);

  if (progressUpdater) {
    progressUpdater->SetTitle(title);
    progressUpdater->SetContentLength(contentLength);
  }

  return true;
}

char *cFileDownloadRequest::GetExtension(const char *contentType, const char *url) {
  // Get extension from Content-Type
  char *ext = NULL;
  char *ext2 = MimeTypes->ExtensionFromMimeType(contentType);

  // Workaround for buggy servers: If the server claims that the mime
  // type is text/plain, ignore the server and fall back to extracting
  // the extension from the URL. This function should be called only
  // for video, audio or ASX files and therefore text/plain is clearly
  // incorrect.
  if (ext2 && contentType && !strcasecmp(contentType, "text/plain")) {
    debug("Ignoring content type text/plain, getting extension from url.");
    free(ext2);
    ext2 = NULL;
  }

  if (ext2) {
    // Append dot in the start of the extension
    ext = (char *)malloc(strlen(ext2)+2);
    ext[0] = '.';
    ext[1] = '\0';
    strcat(ext, ext2);
    free(ext2);
    return ext;
  }

  // Get extension from URL
  ext = extensionFromUrl(url);
  if (ext)
    return ext;

  // No extension!
  return strdup("");
}

void cFileDownloadRequest::RequestDone(int errorcode, cString pharse) {
  cMenuRequest::RequestDone(errorcode, pharse);
  if (progressUpdater)
    progressUpdater->MarkDone(errorcode, pharse);
  if (destfile)
    destfile->Close();
}

// --- cStreamUrlRequest ---------------------------------------------------

cStreamUrlRequest::cStreamUrlRequest(int ID, const char *ref)
: cMenuRequest(ID, ref) {
  AppendQualityParamsToRef();
}

WebviHandle cStreamUrlRequest::PrepareHandle() {
  if (handle == -1) {
    handle = webvi_new_request(webvi, wvtref, WEBVIREQ_STREAMURL);

    if (handle != -1) {
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEFUNC, WriteCallback);
      webvi_set_opt(webvi, handle, WEBVIOPT_WRITEDATA, this);
    }
  }

  return handle;
}

// --- cTimerRequest -------------------------------------------------------

cTimerRequest::cTimerRequest(int ID, const char *ref)
: cMenuRequest(ID, ref)
{
}

// --- cRequestVector ------------------------------------------------------

cMenuRequest *cRequestVector::FindByHandle(WebviHandle handle) {
  for (int i=0; i<Size(); i++)
    if (At(i)->GetHandle() == handle)
      return At(i);

  return NULL;
}
