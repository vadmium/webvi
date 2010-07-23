/*
 * menu.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_PLAYER_H
#define __WEBVIDEO_PLAYER_H

class cMediaPlayer {
public:
  virtual ~cMediaPlayer() {};
  virtual bool Launch(const char *url) = 0;
};

class cXineliboutputPlayer : public cMediaPlayer {
public:
  bool Launch(const char *url);
};

class cMPlayerPlayer : public cMediaPlayer {
public:
  bool Launch(const char *url);
};


#endif
