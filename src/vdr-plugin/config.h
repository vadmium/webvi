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
  char *postProcessCmd;
  bool preferXine;
  bool vfatNames;
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

  void SetUseVFATNames(bool vfat);
  bool GetUseVFATNames();

  const char *GetMinQuality(const char *site, eRequestType type);
  const char *GetMaxQuality(const char *site, eRequestType type);

  void SetPostProcessCmd(const char *cmd);
  const char *GetPostProcessCmd();
};

extern cWebvideoConfig *webvideoConfig;

#endif
