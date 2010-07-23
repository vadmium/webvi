/*
 * mimetypes.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vdr/tools.h>
#include "mimetypes.h"
#include "common.h"

// --- cMimeListObject -----------------------------------------------------

cMimeListObject::cMimeListObject(const char *mimetype, const char *extension) {
  type = strdup(mimetype);
  ext = strdup(extension);
}

cMimeListObject::~cMimeListObject() {
  free(type);
  free(ext);
}

// --- cMimeTypes ----------------------------------------------------------

cMimeTypes::cMimeTypes(const char **mimetypefiles) {
  for (const char **filename=mimetypefiles; *filename; filename++) {
    FILE *f = fopen(*filename, "r");
    if (!f) {
      LOG_ERROR_STR((const char *)cString::sprintf("failed to open mime type file %s", *filename));
      continue;
    }

    cReadLine rl;
    char *line = rl.Read(f);
    while (line) {
      // Comment lines starting with '#' and empty lines are skipped
      // Expected format for the lines:
      // mime/type   ext
      if (*line && (*line != '#')) {
        char *ptr = line;
        while ((*ptr != '\0') && (!isspace(*ptr)))
          ptr++;

        if (ptr == line) {
          // empty line, ignore
          line = rl.Read(f);
          continue;
        }

        char *mimetype = (char *)malloc(ptr-line+1);
        strncpy(mimetype, line, ptr-line);
        mimetype[ptr-line] = '\0';

        while (*ptr && isspace(*ptr))
          ptr++;
        char *eptr = ptr;
        while (*ptr && !isspace(*ptr))
          ptr++;

        if (ptr == eptr) {
          // no extension, ignore
          free(mimetype);
          line = rl.Read(f);
          continue;
        }

        char *extension = (char *)malloc(ptr-eptr+1);
        strncpy(extension, eptr, ptr-eptr);
        extension[ptr-eptr] = '\0';

        types.Add(new cMimeListObject(mimetype, extension));
        free(extension);
        free(mimetype);
      }
      line = rl.Read(f);
    }

    fclose(f);
  }
}

char *cMimeTypes::ExtensionFromMimeType(const char *mimetype) {
  if (!mimetype)
    return NULL;

  for (cMimeListObject *m = types.First(); m; m = types.Next(m))
    if (strcmp(m->GetType(), mimetype) == 0) {
      return strdup(m->GetExtension());
    }

  return NULL;
}
