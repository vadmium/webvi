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

#define WVTREFERENCE "wvt:///www.youtube.com/videopage.xsl?srcurl=http%3A//www.youtube.com/watch%3Fv%3Dk5LmKNYTqvk"

#define CHECK_WEBVI_CALL(err, funcname) \
  if (err != WEBVIERR_OK) { \
    fprintf(stderr, "%s FAILED: %s\n", funcname, webvi_strerror(ctx, err));   \
    returncode = 127; \
    goto cleanup; \
  }

struct download_data {
  long bytes_downloaded;
  WebviCtx ctx;
  WebviHandle handle;
};

ssize_t file_callback(const char *buf, size_t len, void *data) {
  struct download_data *dldata = (struct download_data *)data;

  if (dldata->bytes_downloaded == 0) {
    char *url, *title, *contentType;
    long contentLength;

    if (webvi_get_info(dldata->ctx, dldata->handle, WEBVIINFO_URL, &url) != WEBVIERR_OK) {
      fprintf(stderr, "webvi_get_info FAILED\n");
      return -1;
    }

    if (url) {
      printf("File URL: %s\n", url);
      free(url);
    }

    if (webvi_get_info(dldata->ctx, dldata->handle, WEBVIINFO_STREAM_TITLE, &title) != WEBVIERR_OK) {
      fprintf(stderr, "webvi_get_info FAILED\n");
      return -1;
    }

    if (title) {
      printf("Title: %s\n", title);
      free(title);
    }

    if (webvi_get_info(dldata->ctx, dldata->handle, WEBVIINFO_CONTENT_TYPE, &contentType) != WEBVIERR_OK) {
      fprintf(stderr, "webvi_get_info FAILED\n");
      return -1;
    }

    if (contentType) {
      printf("Content type: %s\n", contentType);
      free(contentType);
    }

    if (webvi_get_info(dldata->ctx, dldata->handle, WEBVIINFO_CONTENT_LENGTH, &contentLength) != WEBVIERR_OK) {
      fprintf(stderr, "webvi_get_info FAILED\n");
      return -1;
    }

    printf("Content length: %ld\n", contentLength);
  }

  dldata->bytes_downloaded += len;

  printf("\r%ld", dldata->bytes_downloaded);

  return len;
}

int main(int argc, const char* argv[]) {
  int returncode = 0;
  WebviCtx ctx = 0;
  WebviHandle handle = -1;
  fd_set readfd, writefd, excfd;
  int maxfd, fd, s, msg_remaining;
  struct timeval timeout;
  long running;
  WebviMsg *donemsg;
  int done;
  struct download_data callback_data;

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

  handle = webvi_new_request(ctx, WVTREFERENCE, WEBVIREQ_FILE);
  if (handle == -1) {
    fprintf(stderr, "webvi_new_request FAILED\n");
    returncode = 127;
    goto cleanup;
  }

  callback_data.bytes_downloaded = 0;
  callback_data.ctx = ctx;
  callback_data.handle = handle;
  CHECK_WEBVI_CALL(webvi_set_opt(ctx, handle, WEBVIOPT_WRITEDATA, &callback_data),
                   "webvi_set_opt(WEBVIOPT_WRITEDATA)");
  CHECK_WEBVI_CALL(webvi_set_opt(ctx, handle, WEBVIOPT_WRITEFUNC, file_callback),
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

    timeout.tv_sec = 1;
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

  if (donemsg->status_code != 0) {
    printf("Error: Unexpected status code %d\n", donemsg->status_code);
    returncode = 127;
  } else {
    printf("\nRead %ld bytes.\n"
           "Test successful.\n", callback_data.bytes_downloaded);
  }

cleanup:
  if (ctx != 0) {
    if (handle != -1)
      webvi_delete_handle(ctx, handle);
    webvi_cleanup_context(ctx);
  }
  webvi_cleanup(1);

  return returncode;
}
