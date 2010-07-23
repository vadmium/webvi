/*
 * mimetypes.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_MIMETYPES_H
#define __WEBVIDEO_MIMETYPES_H

class cMimeListObject : public cListObject {
private:
  char *type;
  char *ext;
public:
  cMimeListObject(const char *mimetype, const char *extension);
  ~cMimeListObject();

  char *GetType() { return type; };
  char *GetExtension() { return ext; };
};

class cMimeTypes {
private:
  cList<cMimeListObject> types;
public:
  cMimeTypes(const char **filenames);

  char *ExtensionFromMimeType(const char *mimetype);
};

extern cMimeTypes *MimeTypes;

#endif
