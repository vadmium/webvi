/*
 * libwebvi.c: C bindings for webvi Python module
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

#include <Python.h>
#include <stdio.h>
#include <dlfcn.h>

#include "libwebvi.h"

static const char *VERSION = "libwebvi/" LIBWEBVI_VERSION;

static const int MAX_ERROR_MESSAGE_LENGTH = 512;
static const int MAX_MSG_STRING_LENGTH = 512;

static PyThreadState *main_state = NULL;

typedef struct per_interpreter_data_t {
  PyThreadState *interp;
  PyObject *webvi_module;
  char *last_error;
  WebviMsg latest_message;
} per_interpreter_data;

#ifdef DEBUG

#define debug(x...) fprintf(stderr, x)
#define handle_pyerr() { if (PyErr_Occurred()) { PyErr_Print(); } }

#else

#define debug(x...) 
#define handle_pyerr() PyErr_Clear()

#endif


/**********************************************************************
 *
 * Internal functions
 */

static PyObject *call_python(PyObject *webvi_module,
                             const char *funcname,
                             PyObject *args) {
  PyObject *func, *val = NULL;

#ifdef DEBUG
  debug("call_python %s ", funcname);
  if (PyObject_Print(args, stderr, 0) == -1)
    debug("<print failed>");
  debug("\n");
#endif

  func = PyObject_GetAttrString(webvi_module, funcname);
  if (func) {
    val = PyObject_CallObject(func, args);

    Py_DECREF(func);
  }

  return val;
}

static long set_callback(PyObject *webvi_module, WebviHandle h,
                         WebviOption callbacktype,
                         webvi_callback callback,
                         PyObject *prototype) {
  long res = WEBVIERR_UNKNOWN_ERROR;

  if (prototype && PyCallable_Check(prototype)) {
    PyObject *args = Py_BuildValue("(l)", (long)callback);
    PyObject *val = PyObject_CallObject(prototype, args);
    Py_DECREF(args);

    if (val) {
      PyObject *webvihandle = PyInt_FromLong(h);
      PyObject *option = PyInt_FromLong(callbacktype);
      PyObject *args2 = PyTuple_Pack(3, webvihandle, option, val);
      PyObject *retval = call_python(webvi_module, "set_opt", args2);
      Py_DECREF(args2);
      Py_DECREF(option);
      Py_DECREF(webvihandle);
      Py_DECREF(val);

      if (retval) {
        if (PyInt_Check(retval))
          res = PyInt_AsLong(retval);
        Py_DECREF(retval);
      }
    }
  }

  if (res == WEBVIERR_UNKNOWN_ERROR)
    handle_pyerr();

  return res;
}

/*
 * Converts PyInt to WebviResult.
 *
 * If intobject is NULL, assumes that a Python exception has occurred.
 */
static WebviResult pyint_as_webviresult(PyObject *intobject) {
  if (intobject && PyInt_Check(intobject))
    return PyInt_AsLong(intobject);

  handle_pyerr();

  return WEBVIERR_UNKNOWN_ERROR;
}

/*
 * Duplicate Python string as C string. If the parameter is a unicode
 * object, it is encoded to UTF-8. The caller must free the returned
 * memory.
 */
static char *PyString_strdupUTF8(PyObject *string) {
  char *buffer = NULL;
  Py_ssize_t len = -1;
  char *ret = NULL;
  PyObject *realstring = string;
  Py_INCREF(realstring);

  if (PyUnicode_Check(realstring)) {
    PyObject *encoded = PyUnicode_AsUTF8String(realstring);
    if (encoded) {
      Py_DECREF(realstring);
      realstring = encoded;
    } else {
      handle_pyerr();
    }
  }

  if (PyString_AsStringAndSize(realstring, &buffer, &len) == -1) {
    handle_pyerr();
    buffer = "";
    len = 0;
  }

  if (buffer) {
    ret = (char *)malloc((len+1)*sizeof(char));
    if (ret)
      memcpy(ret, buffer, len+1);
  }

  Py_DECREF(realstring);

  return ret;
}

/**********************************************************************
 *
 * Public functions
 */

int webvi_global_init() {
  if (main_state)
    return 0;

  // Python modules in lib-dynload/*.so do not correctly depend on
  // libpython*.so. We need to dlopen the library here, otherwise
  // importing webvi dies with "undefined symbol:
  // PyExc_ValueError". See http://bugs.python.org/issue4434
  dlopen(PYTHONSHAREDLIB, RTLD_LAZY | RTLD_GLOBAL);

  Py_InitializeEx(0);
  PyEval_InitThreads();
  main_state = PyThreadState_Get();
  PyEval_ReleaseLock(); /* release GIL acquired by PyEval_InitThreads */

  return 0;
}

void webvi_cleanup(int cleanup_python) {
  /* Should we kill active interpreters first? */

  if (cleanup_python != 0) {
    PyEval_AcquireLock();
    PyThreadState_Swap(main_state);
    Py_Finalize();
  }
}

WebviCtx webvi_initialize_context(void) {
  per_interpreter_data *ctx = (per_interpreter_data *)malloc(sizeof(per_interpreter_data));
  if (!ctx)
    goto err;

  PyEval_AcquireLock();

  ctx->interp = NULL;
  ctx->last_error = NULL;
  ctx->latest_message.msg = 0;
  ctx->latest_message.handle = -1;
  ctx->latest_message.status_code = -1;
  ctx->latest_message.data = (char *)malloc(MAX_MSG_STRING_LENGTH*sizeof(char));
  if (!ctx->latest_message.data)
    goto err;

  ctx->interp = Py_NewInterpreter();
  if (!ctx->interp) {
    debug("Py_NewInterpreter failed\n");
    goto err;
  }

  PyThreadState_Swap(ctx->interp);

  ctx->webvi_module = PyImport_ImportModule("webvi.api");
  if (!ctx->webvi_module) {
    debug("import webvi.api failed\n");
    handle_pyerr();
    goto err;
  }

  /* These are used to wrap C-callbacks into Python callables. 
     Keep in sync with libwebvi.h. */
  if (PyRun_SimpleString("from ctypes import CFUNCTYPE, c_int, c_size_t, c_char_p, c_void_p\n"
                         "WriteCallback = CFUNCTYPE(c_size_t, c_char_p, c_size_t, c_void_p)\n"
                         "ReadCallback = CFUNCTYPE(c_size_t, c_char_p, c_size_t, c_void_p)\n") != 0) {
    debug("callback definitions failed\n");
    goto err;
  }

  PyEval_ReleaseThread(ctx->interp);

  return (WebviCtx)ctx;

err:
  if (ctx) {
    if (ctx->interp) {
      Py_EndInterpreter(ctx->interp);
      PyThreadState_Swap(NULL);
    }

    PyEval_ReleaseLock();

    if (ctx->latest_message.data)
      free(ctx->latest_message.data);
    free(ctx);
  }

  return 0;
}

void webvi_cleanup_context(WebviCtx ctx) {
  if (ctx == 0)
    return;

  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyThreadState_Swap(c->interp);

  /* FIXME: explicitly terminate all active handles? */

  Py_DECREF(c->webvi_module);
  c->webvi_module = NULL;

  Py_EndInterpreter(c->interp);
  c->interp = NULL;

  PyThreadState_Swap(NULL);

  free(c);
}

const char* webvi_version(void) {
  return VERSION;
}

const char* webvi_strerror(WebviCtx ctx, WebviResult res) {
  char *errmsg;

  per_interpreter_data *c = (per_interpreter_data *)ctx;

  if (!c->last_error) {
    /* We are going to leak c->last_error */
    c->last_error = (char *)malloc(MAX_ERROR_MESSAGE_LENGTH*sizeof(char));
    if (!c->last_error)
      return NULL;
  }

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(i)", res);
  PyObject *msg = call_python(c->webvi_module, "strerror", args);
  Py_DECREF(args);

  if (msg) {
    errmsg = PyString_AsString(msg);
    if (!errmsg) {
      handle_pyerr();
      errmsg = "Internal error";
    }

    strncpy(c->last_error, errmsg, MAX_ERROR_MESSAGE_LENGTH-1);
    c->last_error[MAX_ERROR_MESSAGE_LENGTH] = '\0';

    Py_DECREF(msg);
  } else {
    handle_pyerr();
  }

  PyEval_ReleaseThread(c->interp);

  return c->last_error;
}

WebviResult webvi_set_config(WebviCtx ctx, WebviConfig conf, const char *value) {
  WebviResult res;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(is)", conf, value);
  PyObject *v = call_python(c->webvi_module, "set_config", args);
  Py_DECREF(args);

  res = pyint_as_webviresult(v);
  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviHandle webvi_new_request(WebviCtx ctx, const char *webvireference, WebviRequestType type) {
  WebviHandle res = -1;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(si)", webvireference, type);
  PyObject *v = call_python(c->webvi_module, "new_request", args);
  Py_DECREF(args);

  res = pyint_as_webviresult(v);
  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_start_handle(WebviCtx ctx, WebviHandle h) {
  WebviResult res;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(i)", h);
  PyObject *v = call_python(c->webvi_module, "start_handle", args);
  Py_DECREF(args);

  res = pyint_as_webviresult(v);
  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_stop_handle(WebviCtx ctx, WebviHandle h) {
  WebviResult res = WEBVIERR_UNKNOWN_ERROR;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(i)", h);
  PyObject *v = call_python(c->webvi_module, "stop_handle", args);
  Py_DECREF(args);

  res = pyint_as_webviresult(v);
  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_delete_handle(WebviCtx ctx, WebviHandle h) {
  WebviResult res;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(i)", h);
  PyObject *v = call_python(c->webvi_module, "delete_handle", args);
  Py_DECREF(args);

  res = pyint_as_webviresult(v);
  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_set_opt(WebviCtx ctx, WebviHandle h, WebviOption opt, ...) {
  va_list argptr;
  WebviResult res = WEBVIERR_UNKNOWN_ERROR;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);
  
  PyObject *m = PyImport_AddModule("__main__");
  if (!m) {
    handle_pyerr();
    PyEval_ReleaseThread(c->interp);
    return res;
  }

  PyObject *maindict = PyModule_GetDict(m);

  va_start(argptr, opt);

  switch (opt) {
  case WEBVIOPT_WRITEFUNC:
  {
    webvi_callback writerptr = va_arg(argptr, webvi_callback);
    PyObject *write_prototype = PyDict_GetItemString(maindict, "WriteCallback");
    if (write_prototype)
      res = set_callback(c->webvi_module, h, WEBVIOPT_WRITEFUNC,
                         writerptr, write_prototype);
    break;
  }

  case WEBVIOPT_WRITEDATA:
  {
    void *data = va_arg(argptr, void *);
    PyObject *args = Py_BuildValue("(iil)", h, WEBVIOPT_WRITEDATA, (long)data);
    PyObject *v = call_python(c->webvi_module, "set_opt", args);
    Py_DECREF(args);

    res = pyint_as_webviresult(v);
    Py_XDECREF(v);

    break;
  }

  case WEBVIOPT_READFUNC:
  {
    webvi_callback readerptr = va_arg(argptr, webvi_callback);
    PyObject *read_prototype = PyDict_GetItemString(maindict, "ReadCallback");
    if (read_prototype)
      res = set_callback(c->webvi_module, h, WEBVIOPT_READFUNC,
                         readerptr, read_prototype);
    break;
  }

  case WEBVIOPT_READDATA:
  {
    void *data = va_arg(argptr, void *);
    PyObject *args = Py_BuildValue("(iil)", h, WEBVIOPT_READDATA, (long)data);
    PyObject *v = call_python(c->webvi_module, "set_opt", args);
    Py_DECREF(args);

    res = pyint_as_webviresult(v);
    Py_XDECREF(v);

    break;
  }

  default:
    res = WEBVIERR_INVALID_PARAMETER;
    break;
  }

  va_end(argptr);
  
  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_get_info(WebviCtx ctx, WebviHandle h, WebviInfo info, ...) {
  va_list argptr;
  WebviResult res = WEBVIERR_UNKNOWN_ERROR;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  va_start(argptr, info);

  switch (info) {
  case WEBVIINFO_URL:
  {
    char **dest = va_arg(argptr, char **);
    PyObject *args = Py_BuildValue("(ii)", h, info);
    PyObject *v = call_python(c->webvi_module, "get_info", args);
    Py_DECREF(args);

    *dest = NULL;

    if (v) {
      if (PySequence_Check(v) && (PySequence_Length(v) >= 2)) {
        PyObject *retval = PySequence_GetItem(v, 0);
        PyObject *val = PySequence_GetItem(v, 1);

        if (PyInt_Check(retval) &&
            (PyString_Check(val) || PyUnicode_Check(val))) {
          *dest = PyString_strdupUTF8(val);
          res = PyInt_AsLong(retval);
        }

        Py_DECREF(val);
        Py_DECREF(retval);
      }

      Py_DECREF(v);
    } else {
      handle_pyerr();
    }
    
    break;
  }

  case WEBVIINFO_CONTENT_LENGTH:
  {
    long *dest = va_arg(argptr, long *);
    PyObject *args = Py_BuildValue("(ii)", h, info);
    PyObject *v = call_python(c->webvi_module, "get_info", args);
    Py_DECREF(args);

    *dest = -1;

    if (v) {
      if (PySequence_Check(v) && (PySequence_Length(v) >= 2)) {
        PyObject *retval = PySequence_GetItem(v, 0);
        PyObject *val = PySequence_GetItem(v, 1);

        if (PyInt_Check(retval) && PyInt_Check(val)) {
          *dest = PyInt_AsLong(val);
          res = PyInt_AsLong(retval);
        }

        Py_DECREF(val);
        Py_DECREF(retval);
      }
      
      Py_DECREF(v);
    } else {
      handle_pyerr();
    }

    break;
  }

  case WEBVIINFO_CONTENT_TYPE:
  {
    char **dest = va_arg(argptr, char **);
    PyObject *args = Py_BuildValue("(ii)", h, info);
    PyObject *v = call_python(c->webvi_module, "get_info", args);
    Py_DECREF(args);

    *dest = NULL;

    if (v) {
      if (PySequence_Check(v) && (PySequence_Length(v) >= 2)) {
        PyObject *retval = PySequence_GetItem(v, 0);
        PyObject *val = PySequence_GetItem(v, 1);

        if (PyInt_Check(retval) &&
            (PyString_Check(val) || PyUnicode_Check(val))) {
          *dest = PyString_strdupUTF8(val);
          res = PyInt_AsLong(retval);
        }

        Py_DECREF(val);
        Py_DECREF(retval);
      }

      Py_DECREF(v);
    } else {
      handle_pyerr();
    }
    
    break;
  }

  case WEBVIINFO_STREAM_TITLE:
  {
    char **dest = va_arg(argptr, char **);
    PyObject *args = Py_BuildValue("(ii)", h, info);
    PyObject *v = call_python(c->webvi_module, "get_info", args);
    Py_DECREF(args);

    *dest = NULL;

    if (v) {
      if (PySequence_Check(v) && (PySequence_Length(v) >= 2)) {
        PyObject *retval = PySequence_GetItem(v, 0);
        PyObject *val = PySequence_GetItem(v, 1);

        if (PyInt_Check(retval) &&
            (PyString_Check(val) || PyUnicode_Check(val))) {
          *dest = PyString_strdupUTF8(val);
          res = PyInt_AsLong(retval);
        }

        Py_DECREF(val);
        Py_DECREF(retval);
      }

      Py_DECREF(v);
    } else {
      handle_pyerr();
    }
    
    break;
  }

  default:
    res = WEBVIERR_INVALID_PARAMETER;
    break;
  }

  va_end(argptr);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_fdset(WebviCtx ctx,
                        fd_set *read_fd_set,
                        fd_set *write_fd_set,
                        fd_set *exc_fd_set,
                        int *max_fd)
{
  WebviResult res = WEBVIERR_UNKNOWN_ERROR;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *v = call_python(c->webvi_module, "fdset", NULL);

  if (v && PySequence_Check(v) && (PySequence_Length(v) == 5)) {
    PyObject *retval = PySequence_GetItem(v, 0);
    PyObject *readfd = PySequence_GetItem(v, 1);
    PyObject *writefd = PySequence_GetItem(v, 2);
    PyObject *excfd = PySequence_GetItem(v, 3);
    PyObject *maxfd = PySequence_GetItem(v, 4);
    PyObject *fd;
    int i;

    if (readfd && PySequence_Check(readfd)) {
      for (i=0; i<PySequence_Length(readfd); i++) {
        fd = PySequence_GetItem(readfd, i);
        if (fd && PyInt_Check(fd))
          FD_SET(PyInt_AsLong(fd), read_fd_set);
        else
          handle_pyerr();

        Py_XDECREF(fd);
      }
    }

    if (writefd && PySequence_Check(writefd)) {
      for (i=0; i<PySequence_Length(writefd); i++) {
        fd = PySequence_GetItem(writefd, i);
        if (fd && PyInt_Check(fd))
          FD_SET(PyInt_AsLong(fd), write_fd_set);
        else
          handle_pyerr();

        Py_XDECREF(fd);
      }
    }

    if (excfd && PySequence_Check(excfd)) {
      for (i=0; i<PySequence_Length(excfd); i++) {
        fd = PySequence_GetItem(excfd, i);
        if (fd && PyInt_Check(fd))
          FD_SET(PyInt_AsLong(fd), exc_fd_set);
        else
          handle_pyerr();

        Py_XDECREF(fd);
      }
    }

    if (maxfd && PyInt_Check(maxfd))
      *max_fd = PyInt_AsLong(maxfd);
    else
      handle_pyerr();

    if (retval && PyInt_Check(retval))
      res = PyInt_AsLong(retval);
    else
      handle_pyerr();

    Py_XDECREF(maxfd);
    Py_XDECREF(excfd);
    Py_XDECREF(writefd);
    Py_XDECREF(readfd);
    Py_XDECREF(retval);
  } else {
    handle_pyerr();
  }

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviResult webvi_perform(WebviCtx ctx, int fd, int ev_bitmask, long *running_handles) {
  WebviResult res = WEBVIERR_UNKNOWN_ERROR;
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  PyEval_AcquireThread(c->interp);

  PyObject *args = Py_BuildValue("(ii)", fd, ev_bitmask);
  PyObject *v = call_python(c->webvi_module, "perform", args);
  Py_DECREF(args);

  if (v && (PySequence_Check(v) == 1) && (PySequence_Size(v) == 2)) {
    PyObject *retval = PySequence_GetItem(v, 0);
    PyObject *numhandles = PySequence_GetItem(v, 1);

    if (PyInt_Check(numhandles))
      *running_handles = PyInt_AsLong(numhandles);
    if (PyInt_Check(retval))
      res = PyInt_AsLong(retval);

    Py_DECREF(numhandles);
    Py_DECREF(retval);
  } else {
    handle_pyerr();
  }

  Py_XDECREF(v);

  PyEval_ReleaseThread(c->interp);

  return res;
}

WebviMsg *webvi_get_message(WebviCtx ctx, int *remaining_messages) {
  per_interpreter_data *c = (per_interpreter_data *)ctx;

  WebviMsg *msg = NULL;

  PyEval_AcquireThread(c->interp);

  PyObject *v = call_python(c->webvi_module, "pop_message", NULL);

  if (v) {
    if ((PySequence_Check(v) == 1) && (PySequence_Length(v) == 4)) {
      msg = &(c->latest_message);
      msg->msg = WEBVIMSG_DONE;
      msg->handle = -1;
      msg->status_code = -1;
      msg->data[0] = '\0';

      PyObject *handle = PySequence_GetItem(v, 0);
      if (handle && PyInt_Check(handle))
        msg->handle = (WebviHandle)PyInt_AsLong(handle);
      Py_XDECREF(handle);
    
      PyObject *status = PySequence_GetItem(v, 1);
      if (status && PyInt_Check(status))
        msg->status_code = (int)PyInt_AsLong(status);
      Py_XDECREF(status);
        
      PyObject *errmsg = PySequence_GetItem(v, 2);
      if (errmsg &&
          (PyString_Check(errmsg) || PyUnicode_Check(errmsg))) {
        char *cstr = PyString_strdupUTF8(errmsg);
        if (cstr) {
          strncpy(msg->data, cstr, MAX_MSG_STRING_LENGTH);
          msg->data[MAX_MSG_STRING_LENGTH-1] = '\0';

          free(cstr);
        }
      }
      Py_XDECREF(errmsg);

      PyObject *remaining = PySequence_GetItem(v, 3);
      if (remaining && PyInt_Check(remaining))
        *remaining_messages = (int)PyInt_AsLong(remaining);
      else
        *remaining_messages = 0;
      Py_XDECREF(remaining);
    }

    if (msg->handle == -1)
      msg = NULL;

    Py_DECREF(v);
  } else {
    handle_pyerr();
  }

  PyEval_ReleaseThread(c->interp);

  return msg;
}
