/*
 * player.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <vdr/plugin.h>
#include "player.h"
#include "common.h"

bool cXineliboutputPlayer::Launch(const char *url) {
  debug("launching xinelib player, url = %s", url);

  /*
   * xineliboutput plugin insists on percent encoding (certain
   * characters in) the URL. A properly encoded URL will get broken if
   * we let xineliboutput to encode it the second time. For example,
   * current (Feb 2009) Youtube URLs are affected by this. We will
   * decode the URL before passing it to xineliboutput to fix Youtube
   *
   * On the other hand, some URLs will get broken if the encoding is
   * removed here. There simply isn't a way to make all URLs work
   * because of the way xineliboutput handles the encoding.
   */
  char *decoded = URLdecode(url);
  debug("decoded = %s", decoded);
  bool ret = cPluginManager::CallFirstService("MediaPlayer-1.0", (void *)decoded);
  free(decoded);
  return ret;
}

bool cMPlayerPlayer::Launch(const char *url) {
  /*
   * This code for launching mplayer plugin is just for testing, and
   * most likely does not work.
   */

  debug("launching MPlayer");
  warning("Support for MPlayer is experimental. Don't expect this to work!");

  struct MPlayerServiceData
  {
    int result;
    union
    {
      const char *filename;
    } data;
  };

  const char* const tmpPlayListFileName = "/tmp/webvideo.m3u";
  FILE *f = fopen(tmpPlayListFileName, "w");
  fwrite(url, strlen(url), 1, f);
  fclose(f);
  
  MPlayerServiceData mplayerdata;
  mplayerdata.data.filename = tmpPlayListFileName;

  if (!cPluginManager::CallFirstService("MPlayer-Play-v1", &mplayerdata)) {
    debug("Failed to locate Mplayer service");
    return false;
  }

  if (!mplayerdata.result) {
    debug("Mplayer service failed");
    return false;
  }

  return true;
}
