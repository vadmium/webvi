/*
 * buffer.c: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vdr/tools.h>
#include "buffer.h"

// --- cMemoryBuffer -------------------------------------------------------

cMemoryBuffer::cMemoryBuffer(size_t prealloc) {
  capacity = prealloc;
  buf = (char *)malloc(capacity*sizeof(char));
  offset = 0;
  len = 0;
}

cMemoryBuffer::~cMemoryBuffer() {
  if (buf)
    free(buf);
}

void cMemoryBuffer::Realloc(size_t newsize) {
  if (newsize > capacity-offset) {
    if (newsize <= capacity) {
      // The new buffer fits in the memory if we just move the current
      // content offset bytes backwards.
      buf = (char *)memmove(buf, &buf[offset], len);
      offset = 0;
    } else {
      // We need to realloc. Move the content to the beginning of the
      // buffer while we are at it.
      capacity += min(capacity, (size_t)10*1024);
      capacity = max(capacity, newsize);
      char *newbuf = (char *)malloc(capacity*sizeof(char));
      if (newbuf) {
        memcpy(newbuf, &buf[offset], len);
        offset = 0;
        free(buf);
        buf = newbuf;
      }
    }
  }
}

ssize_t cMemoryBuffer::Put(const char *data, size_t bytes) {
  if (len+bytes > Free()) {
    Realloc(len+bytes);
  }

  if (buf) {
    memcpy(&buf[offset+len], data, bytes);
    len += bytes;
    return bytes;
  }
  return -1;
}

ssize_t cMemoryBuffer::PutFromFile(int fd, size_t bytes) {
  if (len+bytes > Free()) {
    Realloc(len+bytes);
  }

  if (buf) {
    ssize_t r = safe_read(fd, &buf[offset+len], bytes);
    if (r > 0)
      len += r;
    return r;
  } else
    return -1;
}

void cMemoryBuffer::Pop(size_t bytes) {
  if (bytes <= len) {
    offset += bytes;
    len -= bytes;
  }
}
