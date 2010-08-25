/*
 * common.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <vdr/tools.h>
#include "common.h"

char *extensionFromUrl(const char *url) {
  if (!url)
    return NULL;

  // Find the possible query ("?query=foo") or fragment ("#bar"). The
  // extension is located right before them.
  size_t extendpos = strcspn(url, "?#");

  size_t extstartpos = extendpos-1;
  while ((extstartpos > 0) && (url[extstartpos] != '.') && (url[extstartpos] != '/'))
    extstartpos--;

  if ((extstartpos > 0) && (url[extstartpos] == '.')) {
    // We found the extension. Copy it to a buffer, and return it.
    char *ext = (char *)malloc(sizeof(char)*(extendpos-extstartpos+1));
    memcpy(ext, &url[extstartpos], extendpos-extstartpos);
    ext[extendpos-extstartpos] = '\0';

    return ext;
  }

  return NULL;
}

char *validateFileName(const char *filename) {
  if (!filename)
    return NULL;

  char *validated = (char *)malloc(strlen(filename)+1);
  int j=0;
  for (unsigned int i=0; i<strlen(filename); i++) {
    if (filename[i] != '/') {
      validated[j++] = filename[i];
    }
  }
  validated[j] = '\0';
  return validated;
}

int moveFile(const char *oldpath, const char *newpath) {
  if (rename(oldpath, newpath) == 0) {
    return 0;
  } else if (errno == EXDEV) {
    // rename can't move a file between file systems. We have to copy
    // the file manually.
    int fdout = open(newpath, O_WRONLY | O_CREAT | O_EXCL, DEFFILEMODE);
    if (fdout < 0) {
      return -1;
    }

    int fdin = open(oldpath, O_RDONLY);
    if (fdin < 0) {
      close(fdout);
      return -1;
    }

    const int bufsize = 4096;
    char buffer[bufsize];
    bool ok = true;
    while (true) {
      ssize_t len = safe_read(fdin, &buffer, bufsize);
      if (len == 0) {
        break;
      } else if (len < 0) {
        ok = false;
        break;
      }

      if (safe_write(fdout, &buffer, len) != len) {
        ok = false;
        break;
      }
    }

    close(fdin);
    close(fdout);

    if (ok && (unlink(oldpath) <0)) {
      return -1;
    }

    return 0;
  } else {
    return -1;
  }
}

char *URLencode(const char *s) {
  char reserved_and_unsafe[] =
    { // reserved characters
      '$', '&', '+', ',', '/', ':', ';', '=', '?', '@',
      // unsafe characters
      ' ', '"', '<', '>', '#', '%', '{', '}',
      '|', '\\', '^', '~', '[', ']', '`',
      '\0'
    };

  char *buf = (char *)malloc((3*strlen(s)+1)*sizeof(char));
  if (!buf)
    return NULL;

  unsigned char *out;
  const unsigned char *in;
  for (out=(unsigned char *)buf, in=(const unsigned char *)s; *in != '\0'; in++) {
    if ((*in < 32)                            // control chracters
        || (strchr(reserved_and_unsafe, *in)) // reserved and unsafe
        || (*in > 127))                       // non-ASCII
    {
      snprintf((char *)out, 4, "%%%02hhX", *in);
      out += 3;
    } else {
      *out = *in;
      out++;
    }
  }
  *out = '\0';

  return buf;
}

char *URLdecode(const char *s) {
  char *res = (char *)malloc(strlen(s)+1);
  const char *in = s;
  char *out = res;
  const char *hex = "0123456789ABCDEF";
  const char *h1, *h2;

  while (*in) {
    if ((*in == '%') && (in[1] != '\0') && (in[2] != '\0')) {
      h1 = strchr(hex, toupper(in[1]));
      h2 = strchr(hex, toupper(in[2]));
      if (h1 && h2) {
        *out = ((h1-hex) << 4) + (h2-hex);
        in += 3;
      } else {
        *out = *in;
        in++;
      }
    } else {
      *out = *in;
      in++;
    }
    out++;
  }
  *out = '\0';

  return res;
}

char *safeFilename(char *filename) {
  if (filename) {
    strreplace(filename, '/', '!');

    char *p = filename;
    while ((*p == '.') || isspace(*p)) {
      p++;
    }

    if (p != filename) {
      memmove(filename, p, strlen(p)+1);
    }
  }

  return filename;
}

cString shellEscape(const char *s) {
  char *buffer = (char *)malloc((4*strlen(s)+3)*sizeof(char));
  const char *src = s;
  char *dst = buffer;

  *dst++ = '\'';
  while (*src) {
    if (*src == '\'') {
      *dst++ = '\'';
      *dst++ = '\\';
      *dst++ = '\'';
      *dst++ = '\'';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst++ = '\'';
  *dst = '\0';

  return cString(buffer, true);
}
