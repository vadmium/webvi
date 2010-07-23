/*
 * buffer.h: Web video plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef __WEBVIDEO_BUFFER_H
#define __WEBVIDEO_BUFFER_H

#include <unistd.h>

// --- cMemoryBuffer -------------------------------------------------------

// FIFO character buffer.

class cMemoryBuffer {
private:
  char *buf;
  size_t offset;
  size_t len;
  size_t capacity;
protected:
  size_t Free() { return capacity-len-offset; }
  virtual void Realloc(size_t newsize);
public:
  cMemoryBuffer(size_t prealloc = 10*1024);
  virtual ~cMemoryBuffer();

  // Put data into the end of the buffer
  virtual ssize_t Put(const char *data, size_t length);
  // Put data from a file descriptor fd to the buffer
  virtual ssize_t PutFromFile(int fd, size_t length);
  // The pointer to the beginning of the buffer. Only valid until the
  // next Put() or PutFromFile().
  virtual char *Get() { return &buf[offset]; }
  // Remove first n bytes from the buffer.
  void Pop(size_t n);
  // Returns the current length of the buffer
  virtual size_t Length() { return len; }
};

#endif // __WEBVIDEO_BUFFER_H
