/*
 * testlibwebvi.c: unittest for webvi C bindings
 *
 * Copyright (c) 2010 Antti Ajanki <antti.ajanki@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/select.h>
#include <errno.h>

#include "libwebvi.h"

#define WVTREFERENCE "wvt:///?srcurl=mainmenu"
//#define WVTREFERENCE "wvt:///youtube/categories.xsl?srcurl=http://gdata.youtube.com/schemas/2007/categories.cat"
//#define WVTREFERENCE "wvt:///youtube/search.xsl"

#define CHECK_WEBVI_CALL(err, funcname) \
  if (err != WEBVIERR_OK) { \
    fprintf(stderr, "%s FAILED: %s\n", funcname, webvi_strerror(ctx, err));   \
    returncode = 127; \
    goto cleanup; \
  }

ssize_t count_bytes_callback(const char *buf, size_t len, void *data) {
  long *bytes = (long *)data;
  *bytes += len;
  return len;
}

int main(int argc, const char* argv[]) {
  int returncode = 0;
  WebviCtx ctx = 0;
  WebviHandle handle = -1;
  long bytes = 0;
  fd_set readfd, writefd, excfd;
  int maxfd, fd, s, msg_remaining;
  struct timeval timeout;
  long running;
  WebviMsg *donemsg;
  int done;
  char *contenttype;

  printf("Testing %s\n", webvi_version());

  if (webvi_global_init() != 0) {
    fprintf(stderr, "webvi_global_init FAILED\n");
    return 127;
  }

  ctx = webvi_initialize_context();
  if (ctx == 0) {
    fprintf(stderr, "webvi_initialize_context FAILED\n");
    returncode = 127;
    goto cleanup;
  }

  CHECK_WEBVI_CALL(webvi_set_config(ctx, WEBVI_CONFIG_TEMPLATE_PATH, "../../templates"),
                   "webvi_set_config(WEBVI_CONFIG_TEMPLATE_PATH)");

  handle = webvi_new_request(ctx, WVTREFERENCE, WEBVIREQ_MENU);
  if (handle == -1) {
    fprintf(stderr, "webvi_new_request FAILED\n");
    returncode = 127;
    goto cleanup;
  }

  CHECK_WEBVI_CALL(webvi_set_opt(ctx, handle, WEBVIOPT_WRITEDATA, &bytes),
                   "webvi_set_opt(WEBVIOPT_WRITEDATA)");
  CHECK_WEBVI_CALL(webvi_set_opt(ctx, handle, WEBVIOPT_WRITEFUNC, count_bytes_callback),
                   "webvi_set_opt(WEBVIOPT_WRITEFUNC)");
  CHECK_WEBVI_CALL(webvi_start_handle(ctx, handle),
                   "webvi_start_handle");

  done = 0;
  do {
    FD_ZERO(&readfd);
    FD_ZERO(&writefd);
    FD_ZERO(&excfd);
    CHECK_WEBVI_CALL(webvi_fdset(ctx, &readfd, &writefd, &excfd, &maxfd),
                     "webvi_fdset");

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    s = select(maxfd+1, &readfd, &writefd, NULL, &timeout);

    if (s < 0) {
      if (errno == EINTR)
        continue;

      perror("select FAILED");
      returncode = 127;
      goto cleanup;

    } if (s == 0) {
      CHECK_WEBVI_CALL(webvi_perform(ctx, 0, WEBVI_SELECT_TIMEOUT, &running),
                       "webvi_perform");
    } else {
      for (fd=0; fd<=maxfd; fd++) {
        if (FD_ISSET(fd, &readfd)) {
          CHECK_WEBVI_CALL(webvi_perform(ctx, fd, WEBVI_SELECT_READ, &running),
                           "webvi_perform");
        }
        if (FD_ISSET(fd, &writefd)) {
          CHECK_WEBVI_CALL(webvi_perform(ctx, fd, WEBVI_SELECT_WRITE, &running),
                           "webvi_perform");
        }
      }
    }

    do {
      donemsg = webvi_get_message(ctx, &msg_remaining);
      if (donemsg && donemsg->msg == WEBVIMSG_DONE && donemsg->handle == handle) {
        done = 1;
      }
    } while (msg_remaining > 0);
  } while (!done);

  CHECK_WEBVI_CALL(webvi_get_info(ctx, handle, WEBVIINFO_CONTENT_TYPE, &contenttype),
                   "webvi_get_info");
  printf("Read %ld bytes. Content type: %s\n", bytes, contenttype);
  free(contenttype);

  printf("Test successful.\n");

cleanup:
  if (ctx != 0) {
    if (handle != -1)
      webvi_delete_handle(ctx, handle);
    webvi_cleanup_context(ctx);
  }
  webvi_cleanup(1);

  return returncode;
}
