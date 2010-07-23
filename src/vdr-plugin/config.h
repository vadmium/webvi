/*
 * config.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_CONFIG_H
#define __WEBVIDEO_CONFIG_H

#include <vdr/tools.h>
#include "request.h"

class cDownloadQuality : public cListObject {
private:
  char *site;
  char *min;
  char *max;

public:
  cDownloadQuality(const char *site);
  ~cDownloadQuality();

  void SetMin(const char *val);
  void SetMax(const char *val);

  const char *GetSite();
  const char *GetMin();
  const char *GetMax();
};

class cWebvideoConfig {
private:
  char *downloadPath;
  char *templatePath;
  bool preferXine;
  cList<cDownloadQuality> downloadLimits;
  cList<cDownloadQuality> streamLimits;

  const char *GetQuality(const char *site, eRequestType type, int limit);

public:
  cWebvideoConfig();
  ~cWebvideoConfig();

  bool ReadConfigFile(const char *inifile);

  void SetDownloadPath(const char *path);
  const char *GetDownloadPath();

  void SetTemplatePath(const char *path);
  const char *GetTemplatePath();

  void SetPreferXineliboutput(bool pref);
  bool GetPreferXineliboutput();

  const char *GetMinQuality(const char *site, eRequestType type);
  const char *GetMaxQuality(const char *site, eRequestType type);
};

extern cWebvideoConfig *webvideoConfig;

#endif
