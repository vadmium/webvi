# download.py - webvi downloader backend
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

import os
import asyncore
import asynchat
import cStringIO
import urllib
import subprocess
import socket
import signal
import pycurl
import asyncurl
import utils
import version

WEBVID_USER_AGENT = 'libwebvi/%s %s' % (version.VERSION, pycurl.version)
MOZILLA_USER_AGENT = 'Mozilla/5.0 (X11; U; Linux i686 (x86_64); en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5'

try:
    from libmimms import libmms
except ImportError, e:
    pass

# Mapping from curl error codes to webvi errors. The error constants
# are defined only in pycurl 7.16.1 and newer.
if pycurl.version_info()[2] >= 0x071001:
    CURL_ERROR_CODE_MAPPING = \
             {pycurl.E_OK:                     0,
              pycurl.E_OPERATION_TIMEOUTED:    408,
              pycurl.E_OUT_OF_MEMORY:          500,
              pycurl.E_PARTIAL_FILE:           504,
              pycurl.E_READ_ERROR:             504,
              pycurl.E_RECV_ERROR:             504,
              pycurl.E_REMOTE_FILE_NOT_FOUND:  404,
              pycurl.E_TOO_MANY_REDIRECTS:     404,
              pycurl.E_UNSUPPORTED_PROTOCOL:   500,
              pycurl.E_URL_MALFORMAT:          400,
              pycurl.E_COULDNT_CONNECT:        506,
              pycurl.E_COULDNT_RESOLVE_HOST:   506,
              pycurl.E_COULDNT_RESOLVE_PROXY:  506,
              pycurl.E_FILE_COULDNT_READ_FILE: 404,
              pycurl.E_GOT_NOTHING:            504,
              pycurl.E_HTTP_RETURNED_ERROR:    404,
              pycurl.E_INTERFACE_FAILED:       506,
              pycurl.E_LOGIN_DENIED:           403}
else:
    CURL_ERROR_CODE_MAPPING = {pycurl.E_OK:    0}

class DownloaderException(Exception):
    def __init__(self, errcode, errmsg):
        self.code = errcode
        self.msg = errmsg
    
    def __str__(self):
        return '%s %s' % (self.code, self.msg)

def create_downloader(url, templatedir, writefunc=None, headerfunc=None,
                      donefunc=None, HTTPheaders=None, headers_only=False):
    """Downloader factory.

    Returns a suitable downloader object according to url type. Raises
    DownloaderException if creating the downloader fails.
    """
    if url == '':
        return DummyDownloader('', writefunc, headerfunc, donefunc,
                               headers_only)

    elif url.startswith('mms://') or url.startswith('mmsh://'):
        try:
            libmms
        except (NameError, OSError):
            raise DownloaderException(501, 'MMS scheme not supported. Install mimms.')
        return MMSDownload(url, writefunc, headerfunc, donefunc,
                           headers_only)

    elif url.startswith('wvt://'):
        executable, parameters = parse_external_downloader_wvt_uri(url, templatedir)
        if executable is None:
            raise DownloaderException(400, 'Invalid wvt:// URL')
        try:
            return ExternalDownloader(executable, parameters, writefunc,
                                      headerfunc, donefunc, headers_only)
        except OSError, (errno, strerror):
            raise DownloaderException(500, 'Failed to execute %s: %s' %
                                      (executable, strerror))

    else:
        return CurlDownload(url, writefunc, headerfunc, donefunc,
                            HTTPheaders, headers_only)

def convert_curl_error(err, errmsg, aborted):
    """Convert a curl error code err to webvi error code."""
    if err == pycurl.E_WRITE_ERROR:
        return (402, 'Aborted')
    elif err not in CURL_ERROR_CODE_MAPPING:
        return (500, errmsg)
    else:
        return (CURL_ERROR_CODE_MAPPING[err], errmsg)

def parse_external_downloader_wvt_uri(url, templatedir):
    exe = None
    params = []
    if not url.startswith('wvt:///bin/'):
        return (exe, params)

    split = url[len('wvt:///bin/'):].split('?', 1)
    exe = templatedir + '/bin/' + split[0]

    if len(split) > 1:
        params = [urllib.unquote(x) for x in split[1].split('&')]

    return (exe, params)

def _new_process_group():
    os.setpgid(0, 0)

class DownloaderBase:
    """Base class for downloaders."""
    def __init__(self, url):
        self.url = url

    def start(self):
        """Should start the download process."""
        pass

    def abort(self):
        """Signals that the download should be aborted."""
        pass

    def get_url(self):
        """Return the URL where the data was downloaded."""
        return self.url

    def get_body(self):
        return ''

    def get_encoding(self):
        """Return the encoding of the downloaded object, or None if
        encoding is not known."""
        return None
    

class DummyDownloader(DownloaderBase, asyncore.file_dispatcher):
    """This class doesn't actually download anything, but returns msg
    string as if it had been result of a download operation.
    """
    def __init__(self, msg, writefunc=None, headerfunc=None,
                 donefunc=None, headers_only=False):
        DownloaderBase.__init__(self, '')
        self.donefunc = donefunc
        self.writefunc = writefunc
        self.headers_only = headers_only

        readfd, writefd = os.pipe()
        asyncore.file_dispatcher.__init__(self, readfd)
        os.write(writefd, msg)
        os.close(writefd)

    def set_file(self, fd):
        # Like asyncore.file_dispatcher.set_file() but doesn't call
        # add_channel(). We'll call add_channel() in start() when the
        # download shall begin.
        self.socket = asyncore.file_wrapper(fd)
        self._fileno = self.socket.fileno()

    def start(self):
        if self.headers_only:
            self.donefunc(0, None)
        else:
            self.add_channel()

    def readable(self):
        return True

    def writable(self):
        return False

    def handle_read(self):
        try:
            data = self.recv(4096)
            if data and self.writefunc is not None:
                self.writefunc(data)
        except socket.error:
            self.handle_error()

    def handle_close(self):
        self.close()

        if self.donefunc is not None:
            self.donefunc(0, '')


class CurlDownload(DownloaderBase, asyncurl.async_curl_dispatcher):
    """Downloads a large number of different URL schemes using
    libcurl."""
    def __init__(self, url, writefunc=None, headerfunc=None,
                 donefunc=None, HTTPheaders=None, headers_only=False):
        DownloaderBase.__init__(self, url)
        asyncurl.async_curl_dispatcher.__init__(self, url, False)
        self.donefunc = donefunc
        self.writefunc = writefunc
        self.contenttype = None
        self.running = True
        self.aborted = False

        self.curl.setopt(pycurl.USERAGENT, WEBVID_USER_AGENT)
        if headers_only:
            self.curl.setopt(pycurl.NOBODY, 1)
        if headerfunc is not None:
            self.curl.setopt(pycurl.HEADERFUNCTION, headerfunc)
        self.curl.setopt(pycurl.WRITEFUNCTION, self.writewrapper)

        headers = []
        if HTTPheaders is not None:
            for headername, headerdata in HTTPheaders.iteritems():
                if headername == 'cookie':
                    self.curl.setopt(pycurl.COOKIE, headerdata)
                else:
                    headers.append(headername + ': ' + headerdata)

        self.curl.setopt(pycurl.HTTPHEADER, headers)

    def start(self):
        self.add_channel()

    def close(self):
        self.contenttype = self.curl.getinfo(pycurl.CONTENT_TYPE)
        asyncurl.async_curl_dispatcher.close(self)
        self.running = False

    def abort(self):
        self.aborted = True

    def writewrapper(self, data):
        if self.aborted:
            return 0

        if self.writefunc is None:
            return self.write_to_buf(data)
        else:
            return self.writefunc(data)

    def get_body(self):
        return self.buffer.getvalue()

    def get_encoding(self):
        if self.running:
            self.contenttype = self.curl.getinfo(pycurl.CONTENT_TYPE)
        
        if self.contenttype is None:
            return None

        values = self.contenttype.split(';', 1)
        if len(values) > 1:
            for par in values[1].split(' '):
                if par.startswith('charset='):
                    return par[len('charset='):].strip('"')

        return None

    def handle_read(self):
        # Do nothing to the read data here. Instead, let the base
        # class to collect the data to self.buffer.
        pass

    def handle_completed(self, err, errmsg):
        asyncurl.async_curl_dispatcher.handle_completed(self, err, errmsg)
        if self.donefunc is not None:
            err, errmsg = convert_curl_error(err, errmsg, self.aborted)
            self.donefunc(err, errmsg)


class MMSDownload(DownloaderBase, asyncore.file_dispatcher):
    def __init__(self, url, writefunc=None, headerfunc=None,
                 donefunc=None, headers_only=False):
        DownloaderBase.__init__(self, url)
        self.r, self.w = os.pipe()
        asyncore.file_dispatcher.__init__(self, self.r)

        self.writefunc = writefunc
        self.headerfunc = headerfunc
        self.donefunc = donefunc
        self.relaylen = -1
        self.expectedlen = -1
        self.headers_only = headers_only
        self.stream = None
        self.errmsg = None
        self.aborted = False

    def set_file(self, fd):
        self.socket = asyncore.file_wrapper(fd)
        self._fileno = self.socket.fileno()
        
    def recv(self, buffer_size):
        data = self.stream.read()
        if not data:
            self.handle_close()
            return ''
        else:
            return data

    def close(self):
        if self.stream is not None:
            self.stream.close()

        os.close(self.w)
        asyncore.file_dispatcher.close(self)

    def readable(self):
        return self.stream is not None

    def writable(self):
        return False

    def start(self):
        try:
            self.stream = libmms.Stream(self.url, 1000000)
        except libmms.Error, e:
            self.errmsg = e.message
            self.handle_close()
            return

        os.write(self.w, '0') # signal that this dispatcher has data available

        if self.headerfunc:
            # Output the length in a HTTP-like header field so that we
            # can use the same callbacks as with HTTP downloads.
            ext = utils.get_url_extension(self.url)
            if ext == 'wma':
                self.headerfunc('Content-Type: audio/x-ms-wma')
            else: # if ext == 'wmv':
                self.headerfunc('Content-Type: video/x-ms-wmv')
            self.headerfunc('Content-Length: %d' % self.stream.length())

        if self.headers_only:
            self.handle_close()
        else:
            self.add_channel()

    def abort(self):
        self.aborted = True

    def handle_read(self):
        if self.aborted:
            self.handle_close()
            return ''

        try:
            data = self.recv(4096)
            if data and (self.writefunc is not None):
                self.writefunc(data)
        except libmms.Error, e:
            self.errmsg = e.message
            self.handle_close()
            return

    def handle_close(self):
        self.close()
        self.stream = None

        if self.errmsg is not None:
            self.donefunc(500, self.errmsg)
        elif self.aborted:
            self.donefunc(402, 'Aborted')
        elif self.relaylen < self.expectedlen:
            # We got fewer bytes than expected. Maybe the connection
            # was lost?
            self.donefunc(504, 'Download may be incomplete (length %d < %d)' % 
                          (self.relaylen, self.expectedlen))
        else:
            self.donefunc(0, '')


class ExternalDownloader(DownloaderBase, asyncore.file_dispatcher):
    """Executes an external process and reads its result on standard
    output."""
    def __init__(self, executable, parameters, writefunc=None,
                 headerfunc=None, donefunc=None, headers_only=False):
        DownloaderBase.__init__(self, '')
        asyncore.dispatcher.__init__(self, None, None)
        self.executable = executable
        self.writefunc = writefunc
        self.headerfunc = headerfunc
        self.donefunc = donefunc
        self.headers_only = headers_only
        self.contenttype = ''
        self.aborted = False
        
        args = []
        for par in parameters:
            try:
                key, val = par.split('=', 1)
                if key == 'contenttype':
                    self.contenttype = val
                elif key == 'arg':
                    args.append(val)
            except ValueError:
                pass

        if args:
            self.url = args[0]
        else:
            self.url = executable
        self.cmd = [executable] + args

        self.process = None

    def start(self):
        self.headerfunc('Content-Type: ' + self.contenttype)

        if self.headers_only:
            self.donefunc(0, None)
            return

        self.process = subprocess.Popen(self.cmd, stdout=subprocess.PIPE,
                                        close_fds=True,
                                        preexec_fn=_new_process_group)
        asyncore.file_dispatcher.__init__(self, os.dup(self.process.stdout.fileno()))

    def abort(self):
        if self.process is not None:
            self.aborted = True
            pg = os.getpgid(self.process.pid)
            os.killpg(pg, signal.SIGTERM)

    def readable(self):
        # Return True if the subprocess is still alive
        return self.process is not None and self.process.returncode is None

    def writable(self):
        return False

    def handle_read(self):
        try:
            data = self.recv(4096)
            if data and self.writefunc is not None:
                self.writefunc(data)
        except socket.error:
            self.handle_error()
            return

    def handle_close(self):
        self.close()
        self.process.wait()

        if self.donefunc is not None:
            if self.process.returncode == 0:
                self.donefunc(0, '')
            elif self.aborted and self.process.returncode == -signal.SIGTERM:
                self.donefunc(402, 'Aborted')
            else:
                self.donefunc(500, 'Child process "%s" returned error %s' % \
                                  (' '.join(self.cmd), str(self.process.returncode)))

        self.process = None
