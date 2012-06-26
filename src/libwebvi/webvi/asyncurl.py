# asyncurl.py - Wrapper class for using pycurl objects in asyncore
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

"""This is a wrapper for using pycurl objects in asyncore.

Start a transfer by creating an async_curl_dispatch, and call
asyncurl.loop() instead of asyncore.loop().
"""

import asyncore
import pycurl
import traceback
import select
import time
from errno import EINTR

SOCKET_TIMEOUT = pycurl.SOCKET_TIMEOUT
CSELECT_IN = pycurl.CSELECT_IN
CSELECT_OUT = pycurl.CSELECT_OUT
CSELECT_ERR = pycurl.CSELECT_ERR

def poll(timeout=0.0, map=None, mdisp=None):
    if map is None:
        map = asyncore.socket_map
    if mdisp is None:
        mdisp = multi_dispatcher
    if map:
        if mdisp.timeout != -1:
            timeout = min(timeout, mdisp.timeout/1000.0)

        r = []; w = []; e = []
        for fd, obj in map.items():
            is_r = obj.readable()
            is_w = obj.writable()
            if is_r:
                r.append(fd)
            if is_w:
                w.append(fd)
            if is_r or is_w:
                e.append(fd)
        if [] == r == w == e:
            if timeout > 0:
                time.sleep(timeout)
        else:
            try:
                r, w, e = select.select(r, w, e, timeout)
            except select.error, err:
                if err[0] != EINTR:
                    raise
                else:
                    return

        if [] == r == w == e:
            mdisp.socket_action(SOCKET_TIMEOUT, 0)
            return

        for fd in r:
            obj = map.get(fd)
            if obj is None:
                continue
            asyncore.read(obj)

        for fd in w:
            obj = map.get(fd)
            if obj is None:
                continue
            asyncore.write(obj)

        for fd in e:
            obj = map.get(fd)
            if obj is None:
                continue
            asyncore._exception(obj)

def poll_timeout(mdisp=None):
    if mdisp is None:
        mdisp = multi_dispatcher
    mdisp.socket_action(SOCKET_TIMEOUT, 0)

def loop(timeout=30.0, use_poll=False, map=None, count=None, mdisp=None):
    if map is None:
        map = asyncore.socket_map
    if mdisp is None:
        mdisp = multi_dispatcher

    if use_poll and hasattr(select, 'poll'):
        print 'poll2 not implemented'
    poll_fun = poll

    if count is None:
        while map:
            poll_fun(timeout, map, mdisp)

    else:
        while map and count > 0:
            poll_fun(timeout, map, mdisp)
            count = count - 1

def noop_callback(s):
    pass


class curl_multi_dispatcher:
    """A dispatcher for pycurl.CurlMulti() objects. An instance of
    this class is created automatically. There is usually no need to
    construct one manually."""
    def __init__(self, socket_map=None):
        if socket_map is None:
            self._map = asyncore.socket_map
        else:
            self._map = socket_map
        self.dispatchers = {}
        self.timeout = -1
        # The lambda is to avoid "not callable" error from pylint
        self.timeout_callback = lambda x, y: None
        self._curlm = pycurl.CurlMulti()
        self._curlm.setopt(pycurl.M_SOCKETFUNCTION, self._socket_callback)
        self._curlm.setopt(pycurl.M_TIMERFUNCTION, self._update_timeout)
        
    def _socket_callback(self, action, socket, user_data, socket_data):
#         print 'socket callback: %d, %s' % \
#               (socket, {pycurl.POLL_NONE: "NONE",
#                         pycurl.POLL_IN: "IN",
#                         pycurl.POLL_OUT: "OUT",
#                         pycurl.POLL_INOUT: "INOUT",
#                         pycurl.POLL_REMOVE: "REMOVE"}[action])

        if action == pycurl.POLL_NONE:
            return
        elif action == pycurl.POLL_REMOVE:
            if socket in self._map:
                del self._map[socket]
            return

        obj = self._map.get(socket)
        if obj is None:
            obj = dispatcher_wrapper(socket, self)
            self._map[socket] = obj
        
        if action == pycurl.POLL_IN:
            obj.set_readable(True)
            obj.set_writable(False)
        elif action == pycurl.POLL_OUT:
            obj.set_readable(False)
            obj.set_writable(True)
        elif action == pycurl.POLL_INOUT:
            obj.set_readable(True)
            obj.set_writable(True)

    def _update_timeout(self, msec):
        self.timeout = msec
        if self.timeout_callback:
            self.timeout_callback(self, msec)

    def attach(self, curldisp):
        """Starts a transfer on curl handle by attaching it to this
        multihandle."""
        self.dispatchers[curldisp.curl] = curldisp
        try:
            self._curlm.add_handle(curldisp.curl)
        except pycurl.error:
           # the curl object is already on this multi-stack
            pass

        # _curlm.timeout() seems to be needed, although curl
        # documentation doesn't mention it.
        self._update_timeout(self._curlm.timeout())
        self.check_completed()

    def detach(self, curldisp):
        """Removes curl handle from this multihandle, and fire its
        completion callback function."""
        self.del_curl(curldisp.curl)

        # libcurl does not send POLL_REMOVE when a handle is aborted
        for socket, curlobj in self._map.items():
            if curlobj == curldisp:
                del self._map[socket]
                break

    def del_curl(self, curl):
        try:
            self._curlm.remove_handle(curl)
        except pycurl.error:
            # the curl object is not on this multi-stack
            pass
        if curl in self.dispatchers:
            del self.dispatchers[curl]
        curl.close()

    def socket_action(self, fd, evbitmask):
        res = -1
        OK = False
        while not OK:
            try:
                res = self._curlm.socket_action(fd, evbitmask)
                OK = True
            except pycurl.error:
                # Older libcurls may return CURLM_CALL_MULTI_PERFORM,
                # which pycurl (at least 7.19.0) converts to an
                # exception. If that happens, call socket_action
                # again.
                pass
        self.check_completed()
        return res
    
    def check_completed(self):
        nmsg, success, failed = self._curlm.info_read()
        for handle in success:
            disp = self.dispatchers.get(handle)
            if disp is not None:
                try:
                    disp.handle_completed(0, None)
                except:
                    self.handle_error()
            self.del_curl(handle)
        for handle, err, errmsg in failed:
            disp = self.dispatchers.get(handle)
            if disp is not None:
                try:
                    disp.handle_completed(err, errmsg)
                except:
                    self.handle_error()
            self.del_curl(handle)

    def handle_error(self):
        print 'Exception occurred in multicurl processing'
        print traceback.format_exc()


class dispatcher_wrapper:
    """An internal helper class that connects a file descriptor in the
    asyncore.socket_map to a curl_multi_dispatcher."""
    def __init__(self, fd, multicurl):
        self.fd = fd
        self.multicurl = multicurl
        self.read_flag = False
        self.write_flag = False

    def readable(self):
        return self.read_flag

    def writable(self):
        return self.write_flag

    def set_readable(self, x):
        self.read_flag = x

    def set_writable(self, x):
        self.write_flag = x

    def handle_read_event(self):
        self.multicurl.socket_action(self.fd, CSELECT_IN)

    def handle_write_event(self):
        self.multicurl.socket_action(self.fd, CSELECT_OUT)

    def handle_expt_event(self):
        self.multicurl.socket_action(self.fd, CSELECT_ERR)

    def handle_error(self):
        print 'Exception occurred during processing of a curl request'
        print traceback.format_exc()


class async_curl_dispatcher:
    """A dispatcher class for pycurl transfers."""
    def __init__(self, url, auto_start=True):
        """Initializes a pycurl object self.curl. Chunks of downloaded data
        are passed to self.curl_write(), which should be provided by the
        subclass. If auto_start is False, the transfer is not
        started before a call to add_channel().
        """
        self.url = url
        self.socket = None
        self.curl = pycurl.Curl()
        self.curl.setopt(pycurl.URL, self.url)
        self.curl.setopt(pycurl.FOLLOWLOCATION, 1)
        self.curl.setopt(pycurl.AUTOREFERER, 1)
        self.curl.setopt(pycurl.MAXREDIRS, 10)
        self.curl.setopt(pycurl.FAILONERROR, 1)
        self.curl.setopt(pycurl.WRITEFUNCTION, self.curl_write)
        if auto_start:
            self.add_channel()

    def send(self, data):
        raise NotImplementedError

    def add_channel(self, multidisp=None):
        if multidisp is None:
            multidisp = multi_dispatcher
        multidisp.attach(self)

    def del_channel(self, multidisp=None):
        if multidisp is None:
            multidisp = multi_dispatcher
        multidisp.detach(self)

    def close(self):
        self.del_channel()

    def log_info(self, message, type='info'):
        if type != 'info':
            print '%s: %s' % (type, message)

    def handle_error(self):
        print 'Exception occurred during processing of a curl request'
        print traceback.format_exc()
        self.close()

    def handle_write(self):
        self.log_info('unhandled write event', 'warning')

    def handle_completed(self, err, errmsg):
        """Called when the download has finished. err is a numeric
        error code (or 0 if the download was successfull) and errmsg
        is a curl error message as a string."""
        # It seems that a reference to self.write_to_buf forbids
        # garbage collection from deleting this object. unsetopt() or
        # setting the callback to None are not allowed. Is there a
        # better way?
        self.curl.setopt(pycurl.WRITEFUNCTION, noop_callback)
        self.close()


def test():

    class curl_request(async_curl_dispatcher):
        def __init__(self, url, outfile, i):
            async_curl_dispatcher.__init__(self, url, False)
            self.id = i
            self.outfile = outfile
            self.add_channel()

        def curl_write(self, buf):
            print '%s: writing %d bytes' % (self.id, len(buf))
            self.outfile.write(buf)

        def handle_completed(self, err, errmsg):
            if err != 0:
                print '%s: error: %d %s' % (self.id, err, errmsg)
            else:
                print '%s: completed' % self.id

    curl_request('http://www.python.org', open('python.out', 'w'), 1)
    curl_request('http://en.wikipedia.org/wiki/Main_Page', open('wikipedia.out', 'w'), 2)
    loop(timeout=5.0)
    

pycurl.global_init(pycurl.GLOBAL_DEFAULT)
try:
    multi_dispatcher
except NameError:
    multi_dispatcher = curl_multi_dispatcher()

if __name__ == '__main__':
    test()
