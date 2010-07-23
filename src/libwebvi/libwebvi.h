/*
 * libwebvi.h: C bindings for webvi Python module
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

#ifndef __LIBWEBVI_H
#define __LIBWEBVI_H

#include <sys/select.h>
#include <stdlib.h>

typedef int WebviHandle;

typedef ssize_t (*webvi_callback)(const char *, size_t, void *);

typedef enum {
  WEBVIMSG_DONE
} WebviMsgType;

typedef struct {
  WebviMsgType msg;
  WebviHandle handle;
  int status_code;
  char *data;
} WebviMsg;

typedef enum {
  WEBVIREQ_MENU,
  WEBVIREQ_FILE,
  WEBVIREQ_STREAMURL
} WebviRequestType;

typedef enum {
  WEBVIERR_UNKNOWN_ERROR = -1,
  WEBVIERR_OK = 0,
  WEBVIERR_INVALID_HANDLE,
  WEBVIERR_INVALID_PARAMETER
} WebviResult;

typedef enum {
  WEBVIOPT_WRITEFUNC,
  WEBVIOPT_READFUNC,
  WEBVIOPT_WRITEDATA,
  WEBVIOPT_READDATA,
} WebviOption;

typedef enum {
  WEBVIINFO_URL,
  WEBVIINFO_CONTENT_LENGTH,
  WEBVIINFO_CONTENT_TYPE,
  WEBVIINFO_STREAM_TITLE
} WebviInfo;

typedef enum {
  WEBVI_SELECT_TIMEOUT = 0,
  WEBVI_SELECT_READ = 1,
  WEBVI_SELECT_WRITE = 2,
  WEBVI_SELECT_EXCEPTION = 4
} WebviSelectBitmask;

typedef enum {
  WEBVI_CONFIG_TEMPLATE_PATH
} WebviConfig;

typedef long WebviCtx;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the library. Must be called before any other functions
 * (the only exception is webvi_version() which can be called before
 * the library is initialized).
 *
 * Returns 0, if initialization succeeds.
 */
int webvi_global_init(void);

/*
 * Frees all resources currently used by libwebvi and terminates all
 * active connections. Do not call any libwebvi function after this.
 * If the cleanup_python equals 0, the Python library is deinitialized
 * by calling Py_Finalize(), otherwise the Python library is left
 * loaded to be used by other modules of the program.
 */
void webvi_cleanup(int cleanup_python);

/*
 * Create a new context. A valid context is required for calling other
 * functions in the library. The created contextes are independent of
 * each other. The context must be destroyed by a call to
 * webvi_cleanup_context when no longer needed.
 *
 * Return value 0 indicates an error.
 */
WebviCtx webvi_initialize_context(void);

/*
 * Free resources allocated by context ctx. The context can not be
 * used anymore after a call to this function.
 */
void webvi_cleanup_context(WebviCtx ctx);

/*
 * Return the version of libwebvi as a string. The returned value
 * points to a status buffer, and the caller should modify or not free() it.
 */
const char* webvi_version(void);

/*
 * Return a string describing an error code. The returned value points
 * to a status buffer, and the caller should not modify or free() it.
 */
const char* webvi_strerror(WebviCtx ctx, WebviResult err);

/*
 * Set a new value for a global configuration option conf.
 *
 * Currently the only legal value for conf is TEMPLATE_PATH, which
 * sets the base directory for the XSLT templates.
 *
 * The string pointed by value is copied to the library.
 */
WebviResult webvi_set_config(WebviCtx ctx, WebviConfig conf, const char *value);

/*
 * Creates a new download request.
 *
 * webvireference is a wvt:// URI of the resource that should be
 * downloaded. type should be WEBVIREQ_MENU, if the resource should be
 * transformed into a XML menu (that is if webvireferece comes from
 * <ref> tag), WEBVIREQ_FILE, if the resource points to a media stream
 * (from <stream> tag) whose contents should be downloaded, or
 * WEBVIREQ_STREAMURL, if the resource is points to a media stream
 * whose real URL should be resolved.
 *
 * Typically, the reference has been acquired from a previously
 * downloaded menu. A special constant "wvt:///?srcurl=mainmenu" with
 * type WEBVIREQ_MENU can be used to download mainmenu.
 *
 * The return value is a handle to the newly created request. Value -1
 * indicates an error.
 *
 * The request is initialized but the actual network transfer is not
 * started. You can set up additional configuration options on the
 * handle using webvi_set_opt() before starting the handle with
 * webvi_start_handle().
 */
WebviHandle webvi_new_request(WebviCtx ctx, const char *wvtreference, WebviRequestType type);

/*
 * Starts the transfer on handle h. The transfer one or more sockets
 * whose file descriptors are returned by webvi_fdset(). The actual
 * transfer is done during webvi_perform() calls.
 */
WebviResult webvi_start_handle(WebviCtx ctx, WebviHandle h);

/*
 * Requests that the transfer on handle h shoud be aborted. After the
 * library has actually finished aborting the transfer, the handle h
 * is returned by webvi_get_message() with non-zero status code.
 */
WebviResult webvi_stop_handle(WebviCtx ctx, WebviHandle h);

/*
 * Frees resources associated with handle h. The handle can not be
 * used after this call. If the handle is still in the middle of a
 * transfer, the transfer is forcefully aborted.
 */
WebviResult webvi_delete_handle(WebviCtx ctx, WebviHandle h);

/*
 * Sets configuration options that changes behaviour of the handle.
 * opt is one of the values of WebviOption enum as indicated below.
 * The fourth parameter sets the value of the specified option. Its
 * type depends on opt as discussed below.
 *
 * Possible values for opt:
 *
 * WEBVIOPT_WRITEFUNC
 *
 * Set the callback function that shall be called when data is read
 * from the network. The fourth parameter is a pointer to the callback
 * funtion
 *
 * ssize_t (*webvi_callback)(const char *, size_t, void *).
 *
 * When the function is called, the first parameter is a pointer to
 * the incoming data, the second parameters is the size of the
 * incoming data block in bytes, and the third parameter is a pointer
 * to user's data structure can be set by WEBVIOPT_WRITEDATA option.
 *
 * The callback funtion should return the number of bytes is
 * processed. If this differs from the size of the incoming data
 * block, it indicates that an error occurred and the transfer will be
 * aborted.
 *
 * If write callback has not been set (or if it is set to NULL) the
 * incoming data is printed to stdout.
 *
 * WEBVIOPT_WRITEDATA
 *
 * Sets the value that will be passed to the write callback. The
 * fourth parameter is of type void *.
 *
 * WEBVIOPT_READFUNC
 *
 * Set the callback function that shall be called when data is to be
 * send to network. The fourth parameter is a pointer to the callback
 * funtion
 *
 * ssize_t (*webvi_callback)(const char *, size_t, void *)
 *
 * The first parameter is a pointer to a buffer where the data that is
 * to be sent should be written. The second parameter is the maximum
 * size of the buffer. The thirs parameter is a pointer to user data
 * set with WEBVIOPT_READDATA.
 *
 * The return value should be the number of bytes actually written to
 * the buffer. If the return value is -1, the transfer is aborted.
 *
 * WEBVIOPT_READDATA
 *
 * Sets the value that will be passed to the read callback. The
 * fourth parameter is of type void *.
 *
 */
WebviResult webvi_set_opt(WebviCtx ctx, WebviHandle h, WebviOption opt, ...);

/*
 * Get information specific to a WebviHandle. The value will be
 * written to the memory location pointed by the third argument. The
 * type of the pointer depends in the second parameter as discused
 * below. 
 *
 * Available information:
 * 
 * WEBVIINFO_URL
 *
 * Receive URL. The third parameter must be a pointer to char *. The
 * caller must free() the memory.
 *
 * WEBVIINFO_CONTENT_LENGTH
 *
 * Receive the value of Content-length field, or -1 if the size is
 * unknown. The third parameter must be a pointer to long.
 *
 * WEBVIINFO_CONTENT_TYPE
 *
 * Receive the Content-type string. The returned value is NULL, if the
 * Content-type is unknown. The third parameter must be a pointer to
 * char *. The caller must free() the memory.
 * 
 * WEBVIINFO_STREAM_TITLE
 *
 * Receive stream title. The returned value is NULL, if title is
 * unknown. The third parameter must be a pointer to char *. The
 * caller must free() the memory.
 *
 */
WebviResult webvi_get_info(WebviCtx ctx, WebviHandle h, WebviInfo info, ...);

/*
 * Get active file descriptors in use by the library. The file
 * descriptors that should be waited for reading, writing or
 * exceptions are returned in read_fd_set, write_fd_set and
 * exc_fd_set, respectively. The fd_sets are not cleared, but the new
 * file descriptors are added to them. max_fd will contain the highest
 * numbered file descriptor that was returned in one of the fd_sets.
 *
 * One should wait for action in one of the file descriptors returned
 * by this function using select(), poll() or similar system call,
 * and, after seeing action on a file descriptor, call webvi_perform
 * on that descriptor.
 */
WebviResult webvi_fdset(WebviCtx ctx, fd_set *readfd, fd_set *writefd, fd_set *excfd, int *max_fd);

/*
 * Perform input or output action on a file descriptor.
 * 
 * activefd is a file descriptor that was returned by an earlier call
 * to webvi_fdset and has been signalled to be ready by select() or
 * similar funtion. ev_bitmask should be OR'ed combination of
 * WEBVI_SELECT_READ, WEBVI_SELECT_WRITE, WEBVI_SELECT_EXCEPTION to
 * indicate that activefd has been signalled to be ready for reading,
 * writing or being in exception state, respectively. ev_bitmask can
 * also set to WEBVI_SELECT_TIMEOUT which means that the state is
 * checked internally. On return, running_handles will contain the
 * number of still active file descriptors.
 *
 * This function should be called with activefd set to 0 and
 * ev_bitmask to WEBVI_SELECT_TIMEOUT periodically (every few seconds)
 * even if no file descriptors have become ready to allow for timeout
 * handling and other internal tasks.
 */
WebviResult webvi_perform(WebviCtx ctx, int sockfd, int ev_bitmask, long *running_handles);

/*
 * Return the next message from the message queue. Currently the only
 * message, WEBVIMSG_DONE, indicates that a transfer on a handle has
 * finished. The number of messages remaining in the queue after this
 * call is written to remaining_messages. The pointers in the returned
 * WebviMsg point to handle's internal buffers and is valid until the
 * next call to webvi_get_message(). The caller should free the
 * returned WebviMsg. The return value is NULL if there is no messages
 * in the queue.
 */
WebviMsg *webvi_get_message(WebviCtx ctx, int *remaining_messages);

#ifdef __cplusplus
}
#endif


#endif
