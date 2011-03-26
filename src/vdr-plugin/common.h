/*
 * common.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_COMMON_H
#define __WEBVIDEO_COMMON_H

#ifdef DEBUG
#define debug(x...) dsyslog("Webvideo: " x);
#define info(x...) isyslog("Webvideo: " x);
#define warning(x...) esyslog("Webvideo: Warning: " x);
#define error(x...) esyslog("Webvideo: " x);
#else
#define debug(x...) ;
#define info(x...) isyslog("Webvideo: " x);
#define warning(x...) esyslog("Webvideo: Warning: " x);
#define error(x...) esyslog("Webvideo: " x);
#endif

// Return the extension of the url or NULL, if the url has no
// extension. The caller must free the returned string.
char *extensionFromUrl(const char *url);
// Return the domain part from url.
cString parseDomain(const char *url);
// Returns a "safe" version of filename. Currently just removes / from
// the name. The caller must free the returned string.
char *validateFileName(const char *filename);
int moveFile(const char *oldpath, const char *newpath);
// Return the URL encoded version of s. The called must free the
// returned memory.
char *URLencode(const char *s);
// Remove URL encoding from s. The called must free the returned
// memory.
char *URLdecode(const char *s);
// Return a "safe" version of filename. Replace '/' with '_' and
// remove dots from the beginning. If vfatnames is true, replace also
// other characters which are not allowed on VFAT. The string is
// modified in-place, i.e. returns the pointer filename that was
// passed as argument.
char *safeFilename(char *filename, bool vfatnames);
// Escape s so that it can be passed as parameter to a shell command.
cString shellEscape(const char *s);
// Convert string s to lower case
char *strlower(char *s);

#endif // __WEBVIDEO_COMMON_H
