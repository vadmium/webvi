# api.py - webvi API
#
# Copyright (c) 2009-2011 Antti Ajanki <antti.ajanki@iki.fi>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""webvi API

Example workflow:

1) Create a new request. ref is a wvt:// URI.

handle = new_request(ref, WebviRequestType.MENU)

2) Setup a timeout callback:

def timeout_func(t, ignored):
    global alarm
    now = current_time_in_milliseconds()
    alarm = now + t

set_config(WebviConfig.TIMEOUT_CALLBACK, timeout_func)

3) Setup a data callback:

set_opt(handle, WebviOpt.WRITEFUNC, write_func)

4) Start the network transfer:

start_handle(handle)

5) Get active file descriptors, wait for activity using the timeout
value supplied by timeout callback, and let webvi process the file
descriptor.

import select

...

readfd, writefd, excfd = fdset()[1:4]
timeout = alarm - current_time_in_milliseconds()
if timeout < 0:
    timeout = 10.0
readfd, writefd, excfd = select.select(readfd, writefd, excfd, timeout)
if readfd == writefd == excfd == []: # timeout
    perform(WebviSelect.TIMEOUT, WebviSelectBitmask.CHECK)
else:
    for fd in readfd:
        perform(fd, WebviSelectBitmask.READ)
    for fd in writefd:
        perform(fd, WebviSelectBitmask.WRITE)

6) Iterate 5) until pop_message returns handle, which indicates that
the request has been completed.

finished, status, errmsg, remaining = pop_message()
if finished == handle:
    print 'done'
"""

import request
import asyncore
import asyncurl
from constants import WebviErr, WebviOpt, WebviInfo, WebviSelectBitmask, WebviConfig
from ConfigParser import RawConfigParser
import os.path

# Human readable messages for WebviErr items
error_messages = {
    WebviErr.OK: 'Succeeded',
    WebviErr.INVALID_HANDLE: 'Invalid handle',
    WebviErr.INVALID_PARAMETER: "Invalid parameter",
    WebviErr.INTERNAL_ERROR: "Internal error"
    }

# Module-level variables
finished_queue = []
request_list = request.RequestList()
socket_map = asyncore.socket_map

# Internal functions

class MyRequest(request.Request):
    def request_done(self, err, errmsg):
        """Calls the inherited function and puts the handle of the
        finished request to the finished_queue."""
        finished_queue.append(self)
        request.Request.request_done(self, err, errmsg)
    
# Public functions

def strerror(err):
    """Return human readable error message for conststants.WebviErr"""
    try:
        return error_messages[err]
    except KeyError:
        return error_messages[WebviErr.INTERNAL_ERROR]

def set_config(conf, value):
    """Set a new value for a global configuration option conf.

    Currently the only legal value for conf is
    constants.WebviConfig.TEMPLATE_PATH, which sets the base directory
    for the XSLT templates.
    """
    if conf == WebviConfig.TEMPLATE_PATH:
        request.set_template_path(value)
        return WebviErr.OK
    elif conf == WebviConfig.DEBUG:
        if value == '0':
            request.DEBUG = False
        else:
            request.DEBUG = True
        return WebviErr.OK
    elif conf == WebviConfig.TIMEOUT_CALLBACK:
        request.set_timeout_callback(value)
        return WebviErr.OK
    elif conf == WebviConfig.TIMEOUT_DATA:
        request.timeout_data = value
        return WebviErr.OK
    else:
        return WebviErr.INVALID_PARAMETER

def load_config(bool=()):
    """Load options from config files."""
    cfgprs = RawConfigParser()
    cfgprs.read(['/etc/webvi.conf', os.path.expanduser('~/.webvi')])
    options = dict()
    if cfgprs.has_section('webvi'):
        for opt, val in cfgprs.items('webvi'):
            if opt in ('verbose',) + bool:
                try:
                    options[opt] = cfgprs.getboolean('webvi', opt)
                except ValueError:
                    print 'Invalid config: %s = %s' % (opt, val)

                # convert verbose to integer
                if opt == 'verbose':
                    if options['verbose']:
                        options['verbose'] = 1
                    else:
                        options['verbose'] = 0

            else:
                options[opt] = val

    return (options, cfgprs)

def apply_config(options):
    if options.has_key('verbose'):
        set_config(WebviConfig.DEBUG, options['verbose'])
    if options.has_key('templatepath'):
        set_config(WebviConfig.TEMPLATE_PATH, options['templatepath'])

def new_request(reference, reqtype):
    """Create a new request.

    reference is a wvt:// URI which typically comes from previously
    opened menu. reqtype is one of conststants.WebviRequestType and
    indicates wheter the reference is a navigation menu, stream that
    should be downloaded, or a stream whose URL should be returned.
    
    Returns a handle (an integer) will be given to following
    functions. Return value -1 indicates an error.
    """
    req = MyRequest(reference, reqtype)

    if req.srcurl is None:
        return -1
    
    return request_list.put(req)

def set_opt(handle, option, value):
    """Set configuration options on a handle.

    option specifies option's name (one of constants.WebviOpt values)
    and value is the new value for the option.
    """

    try:
        req = request_list[handle]
    except KeyError:
        return WebviErr.INVALID_HANDLE

    if option == WebviOpt.WRITEFUNC:
        req.writefunc = value
    elif option == WebviOpt.WRITEDATA:
        req.writedata = value
    elif option == WebviOpt.READFUNC:
        req.readfunc = value
    elif option == WebviOpt.READDATA:
        req.readdata = value
    else:
        return WebviErr.INVALID_PARAMETER

    return WebviErr.OK

def get_info(handle, info):
    """Get information about a handle.

    info is the type of data that is to be returned (one of
    constants.WebviInfo values).
    """
    try:
        req = request_list[handle]
    except KeyError:
        return (WebviErr.INVALID_HANDLE, None)

    val = None
    if info == WebviInfo.URL:
        if req.dl is not None:
            val = req.dl.get_url()
        else:
            val = req.srcurl
    elif info == WebviInfo.CONTENT_LENGTH:
        val = req.contentlength
    elif info == WebviInfo.CONTENT_TYPE:
        val = req.contenttype
    elif info == WebviInfo.STREAM_TITLE:
        val = req.streamtitle
    else:
        return (WebviErr.INVALID_PARAMETER, None)

    return (WebviErr.OK, val)

def start_handle(handle):
    """Start the network transfer on a handle."""
    try:
        req = request_list[handle]
    except KeyError:
        return WebviErr.INVALID_HANDLE

    req.start()
    return WebviErr.OK

def stop_handle(handle):
    """Aborts network transfer on a handle.

    The abort is confirmed by pop_message() returning the handle with
    an non-zero error code.
    """
    try:
        req = request_list[handle]
    except KeyError:
        return WebviErr.INVALID_HANDLE

    if not req.is_finished():
        req.stop()

    return WebviErr.OK

def delete_handle(handle):
    """Frees resources related to handle.

    This should be called when the transfer has been completed and the
    user is done with the handle. If the transfer is still in progress
    when delete_handle() is called, the transfer is aborted. After
    calling delete_handle() the handle value will be invalid, and
    should not be feed to other functions anymore.
    """
    try:
        del request_list[handle]
    except KeyError:
        return WebviErr.INVALID_HANDLE
    
    return WebviErr.OK

def pop_message():
    """Retrieve messages about finished requests.
    
    If a request has been finished since the last call to this
    function, returns a tuple (handle, status, msg, num_messages),
    where handle identifies the finished request, status is a numeric
    status code (non-zero for an error), msg is a description of an
    error as string, and num_messages is the number of messages that
    can be retrieved by calling pop_messages() again immediately. If
    the finished requests queue is empty, returns (-1, -1, "", 0).
    """
    if finished_queue:
        req = finished_queue.pop()
        return (req.handle, req.status, req.errmsg, len(finished_queue))
    else:
        return (-1, -1, "", 0)

def fdset():
    """Get the list of file descriptors that are currently in use by
    the library.

    Returns a tuple, where the first item is a constants.WebviErr
    value indicating the success of the call, the next three values
    are lists of descriptors that should be monitored for reading,
    writing, and exceptional conditions, respectively. The last item
    is the maximum of the file descriptors in the three lists.
    """
    readfd = []
    writefd = []
    excfd = []
    maxfd = -1

    for fd, disp in socket_map.iteritems():
        if disp.readable():
            readfd.append(fd)
            if fd > maxfd:
                maxfd = fd
        if disp.writable():
            writefd.append(fd)
            if fd > maxfd:
                maxfd = fd
    
    return (WebviErr.OK, readfd, writefd, excfd, maxfd)

def perform(fd, ev_bitmask):
    """Perform transfer on file descriptor fd.

    fd is a file descriptor that has been signalled to be ready by
    select() or similar system call. ev_bitmask specifies what kind of
    activity has been detected using values of
    constants.WebviSelectBitmask. If ev_bitmask is
    constants.WebviSelectBitmask.CHECK the type of activity is
    checked by the function.

    If a timeout occurs before any file descriptor becomes ready, this
    function should be called with fd set to
    constants.WebviSelect.TIMEOUT and ev_bitmask set to
    constants.WebviSelectBitmask.CHECK.
    """
    if fd < 0:
        asyncurl.poll_timeout()
    else:
        disp = socket_map.get(fd)
        if disp is not None:
            if ev_bitmask & WebviSelectBitmask.READ != 0 or \
               (ev_bitmask == WebviSelectBitmask.CHECK and disp.readable()):
                disp.handle_read_event()
            if ev_bitmask & WebviSelectBitmask.WRITE != 0 or \
               (ev_bitmask == WebviSelectBitmask.CHECK and disp.writable()):
                disp.handle_write_event()

    return (WebviErr.OK, len(socket_map))
