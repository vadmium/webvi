/*
 * config.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "dictionary.h"
#include "iniparser.h"
#include "common.h"

// --- cDownloadQuality ---------------------------------------------------

cDownloadQuality::cDownloadQuality(const char *sitename)
: min(NULL), max(NULL) {
  site = sitename ? strdup(sitename) : NULL;
}

cDownloadQuality::~cDownloadQuality() {
  if (site)
    free(site);
  if (min)
    free(min);
  if (max)
    free(max);
}

void cDownloadQuality::SetMin(const char *val) {
  if (min)
    free(min);

  min = val ? strdup(val) : NULL;
}

void cDownloadQuality::SetMax(const char *val) {
  if (max)
    free(max);

  max = val ? strdup(val) : NULL;
}

const char *cDownloadQuality::GetSite() {
  return site;
}

const char *cDownloadQuality::GetMin() {
  return min;
}

const char *cDownloadQuality::GetMax() {
  return max;
}

// --- cWebvideoConfig -----------------------------------------------------

cWebvideoConfig *webvideoConfig = new cWebvideoConfig();

cWebvideoConfig::cWebvideoConfig() {
  downloadPath = NULL;
  templatePath = NULL;
  preferXine = true;
  postProcessCmd = NULL;
}

cWebvideoConfig::~cWebvideoConfig() {
  if (downloadPath)
    free(downloadPath);
  if (templatePath)
    free(templatePath);
  if (postProcessCmd)
    free(postProcessCmd);
}

void cWebvideoConfig::SetDownloadPath(const char *path) {
  if (downloadPath)
    free(downloadPath);
  downloadPath = path ? strdup(path) : NULL;
}

const char *cWebvideoConfig::GetDownloadPath() {
  return downloadPath;
}

void cWebvideoConfig::SetTemplatePath(const char *path) {
  if (templatePath)
    free(templatePath);
  templatePath = path ? strdup(path) : NULL;
}

const char *cWebvideoConfig::GetTemplatePath() {
  return templatePath;
}

void cWebvideoConfig::SetPreferXineliboutput(bool pref) {
  preferXine = pref;
}

bool cWebvideoConfig::GetPreferXineliboutput() {
  return preferXine;
}

bool cWebvideoConfig::ReadConfigFile(const char *inifile) {
  dictionary *conf = iniparser_load(inifile);

  if (!conf)
    return false;

  info("loading config file %s", inifile);

  const char *templatepath = iniparser_getstring(conf, "webvi:templatepath", NULL);
  if (templatepath) {
    debug("templatepath = %s (from %s)", templatepath, inifile);
    SetTemplatePath(templatepath);
  }

  for (int i=0; i<iniparser_getnsec(conf); i++) {
    const char *section = iniparser_getsecname(conf, i);

    if (strcmp(section, "webvi") != 0) {
      const int maxsectionlen = 100;
      char key[128];
      char *keyname;
      const char *sitename;

      cString domain = parseDomain(section);
      if (domain == "")
        sitename = section;
      else
        sitename = domain;

      strncpy(key, section, maxsectionlen);
      key[maxsectionlen] = '\0';
      strcat(key, ":");
      keyname = key+strlen(key);

      strcpy(keyname, "download-min-quality");
      const char *download_min = iniparser_getstring(conf, key, NULL);

      strcpy(keyname, "download-max-quality");
      const char *download_max = iniparser_getstring(conf, key, NULL);

      strcpy(keyname, "stream-min-quality");
      const char *stream_min = iniparser_getstring(conf, key, NULL);

      strcpy(keyname, "stream-max-quality");
      const char *stream_max = iniparser_getstring(conf, key, NULL);

      if (download_min || download_max) {
        cDownloadQuality *limits = new cDownloadQuality(sitename);
        limits->SetMin(download_min);
        limits->SetMax(download_max);
        downloadLimits.Add(limits);

	debug("download priorities for %s (from %s): min = %s, max = %s",
	      sitename, inifile, download_min, download_max);
      }

      if (stream_min || stream_max) {
        cDownloadQuality *limits = new cDownloadQuality(sitename);
        limits->SetMin(stream_min);
        limits->SetMax(stream_max);
        streamLimits.Add(limits);

	debug("streaming priorities for %s (from %s): min = %s, max = %s",
	      sitename, inifile, stream_min, stream_max);
      }
    }
  }

  iniparser_freedict(conf);

  return true;
}

const char *cWebvideoConfig::GetQuality(const char *site, eRequestType type, int limit) {
  if (type != REQT_FILE && type != REQT_STREAM)
    return NULL;

  cList<cDownloadQuality>& priorlist = downloadLimits;
  if (type == REQT_STREAM)
    priorlist = streamLimits;

  cDownloadQuality *node = priorlist.First();

  while (node && (strcmp(site, node->GetSite()) != 0)) {
    node = priorlist.Next(node);
  }

  if (!node)
    return NULL;

  if (limit == 0)
    return node->GetMin();
  else
    return node->GetMax();
}

const char *cWebvideoConfig::GetMinQuality(const char *site, eRequestType type) {
  return GetQuality(site, type, 0);
}

const char *cWebvideoConfig::GetMaxQuality(const char *site, eRequestType type) {
  return GetQuality(site, type, 1);
}

void cWebvideoConfig::SetPostProcessCmd(const char *cmd) {
  if (postProcessCmd)
    free(postProcessCmd);
  postProcessCmd = cmd ? strdup(cmd) : NULL;
}

const char *cWebvideoConfig::GetPostProcessCmd() {
  return postProcessCmd;
}
