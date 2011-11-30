# request.py - webvi request class
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

import urllib
import libxml2
import os.path
import cStringIO
import re
import download
import sys
import tempfile
import utils
import json2xml
import asyncurl
from constants import WebviRequestType

DEBUG = False

DEFAULT_TEMPLATE_PATH = '/usr/local/share/webvi/templates'
template_path = DEFAULT_TEMPLATE_PATH
timeout_data = None

def debug(msg):
    if DEBUG:
        if type(msg) == unicode:
            sys.stderr.write(msg.encode('ascii', 'replace'))
        else:
            sys.stderr.write(msg)
        sys.stderr.write('\n')

def set_template_path(path):
    global template_path
    
    if path is None:
        template_path = os.path.realpath(DEFAULT_TEMPLATE_PATH)
    else:
        template_path = os.path.realpath(path)
    
    debug("set_template_path " + template_path)

def set_timeout_callback(callback):
    c_api_callback = lambda mdisp, timeout: callback(timeout, timeout_data)
    asyncurl.multi_dispatcher.timeout_callback = c_api_callback

def parse_reference(reference):
    """Parses URLs of the following form:

    wvt:///youtube/video.xsl?srcurl=http%3A%2F%2Fwww.youtube.com%2F&param=name1,value1&param=name2,value2

    reference is assumed to be URL-encoded UTF-8 string.

    Returns (template, srcurl, params, processing_instructions) where
    template if the URL path name (the part before ?), srcurl is the
    parameter called srcurl, and params is a dictionary of (name,
    quoted-value) pairs extracted from param parameters. Parameter
    values are quoted so that the xslt parser handles them as string.
    processing_instructions is dictionary of options that affect the
    further processing of the data.
    """
    try:
        reference = str(reference)
    except UnicodeEncodeError:
        return (None, None, None, None)

    if not reference.startswith('wvt:///'):
        return (None, None, None, None)

    ref = reference[len('wvt:///'):]

    template = None
    srcurl = ''
    parameters = {}
    substitutions = {}
    refsettings = {'HTTP-headers': {}}

    fields = ref.split('?', 1)
    template = fields[0]
    if len(fields) == 1:
        return (template, srcurl, parameters, refsettings)
        
    for par in fields[1].split('&'):
        paramfields = par.split('=', 1)
        key = paramfields[0]

        if len(paramfields) == 2:
            value = urllib.unquote(paramfields[1])
        else:
            value = ''

        if key.lower() == 'srcurl':
            srcurl = value
            
        elif key.lower() == 'param':
            fields2 = value.split(',', 1)
            pname = fields2[0].lower()
            if len(fields2) == 2:
                pvalue = fields2[1]
            else:
                pvalue = ''
            parameters[pname] = utils.xpath_str(pvalue)
            
        elif key.lower() == 'subst':
            substfields = value.split(',', 1)
            if len(substfields) == 2:
                substitutions[substfields[0]] = substfields[1]

        elif key.lower() == 'minquality':
            try:
                refsettings['minquality'] = int(value)
            except ValueError:
                pass

        elif key.lower() == 'maxquality':
            try:
                refsettings['maxquality'] = int(value)
            except ValueError:
                pass

        elif key.lower() == 'postprocess':
            refsettings.setdefault('postprocess', []).append(value)

        elif key.lower() == 'contenttype':
            refsettings['overridecontenttype'] = value

        elif key.lower() == 'http-header':
            try:
                headername, headerdata = value.split(',', 1)
            except ValueError:
                continue
            refsettings['HTTP-headers'][headername] = headerdata

    if substitutions:
        srcurl = brace_substitution(srcurl, substitutions)
    
    return (template, srcurl, parameters, refsettings)

def brace_substitution(template, subs):
    """Substitute subs[x] for '{x}' in template. Unescape {{ to { and
    }} to }. Unescaping is not done in substitution keys, i.e. while
    scanning for a closing brace after a single opening brace."""
    strbuf = cStringIO.StringIO()

    last_pos = 0
    for match in re.finditer(r'{{?|}}', template):
        next_pos = match.start()
        if next_pos < last_pos:
            continue

        strbuf.write(template[last_pos:next_pos])
        if match.group(0) == '{{':
            strbuf.write('{')
            last_pos = next_pos+2

        elif match.group(0) == '}}':
            strbuf.write('}')
            last_pos = next_pos+2

        else: # match.group(0) == '{'
            key_end = template.find('}', next_pos+1)
            if key_end == -1:
                strbuf.write(template[next_pos:])
                last_pos = len(template)
                break

            try:
                strbuf.write(urllib.quote(subs[template[next_pos+1:key_end]]))
            except KeyError:
                strbuf.write(template[next_pos:key_end+1])
            last_pos = key_end+1

    strbuf.write(template[last_pos:])
    return strbuf.getvalue()

def create_fifo():
    while True:
        fifoname = tempfile.mktemp()
        try:
            os.mkfifo(fifoname, 0600)
            return fifoname
        except IOError:
            pass

def safe_unlink(name):
    try:
        os.unlink(name)
    except OSError:
        pass


class Request:
    DEFAULT_URL_PRIORITY = 50
    
    def __init__(self, reference, reqtype):
        self.handle = None
        self.dl = None

        # state variables
        self.xsltfile, self.srcurl, self.xsltparameters, self.processing = \
            parse_reference(reference)
        self.type = reqtype
        self.status = -1
        self.errmsg = None
        self.mediaurls = []
        self.checking_url = None

        # stream information
        self.contenttype = 'text/xml'
        self.contentlength = -1
        self.streamtitle = ''

        # callbacks
        self.writefunc = None
        self.writedata = None
        self.readfunc = None
        self.readdata = None

    def handle_header(self, buf):
        namedata = buf.split(':', 1)
        if len(namedata) == 2:
            headername, headerdata = namedata
            if headername.lower() == 'content-type':
                # Strip parameters like charset="utf-8"
                self.contenttype = headerdata.split(';', 1)[0].strip()
            elif headername.lower() == 'content-length':
                try:
                    self.contentlength = int(headerdata.strip())
                except ValueError:
                    self.contentlength = -1

    def setup_downloader(self, url, writefunc, headerfunc, donefunc,
                         HTTPheaders=None, headers_only=False):
        try:
            self.dl = download.create_downloader(url,
                                                 template_path,
                                                 writefunc,
                                                 headerfunc,
                                                 donefunc,
                                                 HTTPheaders,
                                                 headers_only)
            self.dl.start()
        except download.DownloaderException, exc:
            self.dl = None
            if donefunc is not None:
                donefunc(exc.code, exc.msg)

    def start(self):
        debug('start %s\ntemplate = %s, type = %s\n'
              'parameters = %s, processing = %s' % 
              (self.srcurl, self.xsltfile, self.type, str(self.xsltparameters),
               str(self.processing)))
        
        if self.type == WebviRequestType.MENU and self.srcurl == 'mainmenu':
            self.send_mainmenu()
        else:
            self.setup_downloader(self.srcurl, None,
                                  self.handle_header,
                                  self.finished_apply_xslt,
                                  self.processing['HTTP-headers'])

    def stop(self):
        if self.dl is not None:
            debug("aborting")
            self.dl.abort()
    
    def start_download(self, url=None):
        """Initialize a download.

        If url is None, pop the first URL out of self.mediaurls. If
        URL is an ASX playlist, read the content URL from it and start
        to download the actual content.
        """
        while url is None or url == '':
            try:
                url = self.mediaurls.pop(0)
            except IndexError:
                self.request_done(406, 'No more URLs left')

        debug('Start_download ' + url)

        # reset stream status
        self.contenttype = 'text/xml'
        self.contentlength = -1
        
        if self.is_asx_playlist(url):
            self.setup_downloader(url, None,
                                  self.handle_header,
                                  self.finished_playlist_loaded,
                                  self.processing['HTTP-headers'])
                                                 
        else:
            self.setup_downloader(url, self.writewrapper,
                                  self.handle_header,
                                  self.finished_download,
                                  self.processing['HTTP-headers'])
    
    def check_and_send_url(self, url=None):
        """Check if the target exists (currently only for HTTP URLs)
        before relaying the URL to the client."""
        while url is None or url == '':
            try:
                url = self.mediaurls.pop(0)
            except IndexError:
                self.request_done(406, 'No more URLs left')
                return

        debug('check_and_send_url ' + str(url))

        if self.is_asx_playlist(url):
            self.setup_downloader(url, None, self.handle_header,
                                  self.finished_playlist_loaded,
                                  self.processing['HTTP-headers'])
        elif url.startswith('http://') or url.startswith('https://'):
            self.checking_url = url
            self.setup_downloader(url, None, None,
                                  self.finished_check_url,
                                  self.processing['HTTP-headers'], True)
        elif url.startswith('wvt://'):
            if not url.startswith('wvt:///bin/'):
                self.request_done(406,'Streaming not supported')

            fifo = create_fifo()
            fifourl = url + '&arg=' + fifo

            # Unlink fifo when downloader has finished. Note: If the
            # reader doesn't read the fifo for some reason, the writer
            # process will deadlock and the fifo is never unlinked.
            self.setup_downloader(fifourl, None, None,
                                  lambda x, y: safe_unlink(fifo))
            self.writewrapper('file://' + fifo)
            self.request_done(0, None)
        else:
            self.writewrapper(url)
            self.request_done(0, None)

    def send_mainmenu(self):
        """Build the XML main menu from the module description files
        in the hard drive.
        """
        if not os.path.isdir(template_path):
            self.request_done(404, "Can't access service directory %s" %
                              template_path)
            return

        debug('Reading XSLT templates from ' + template_path)

        # Find menu items in the service.xml files in the subdirectories
        menuitems = {}
        for f in os.listdir(template_path):
            if f == 'bin':
                continue

            filename = os.path.join(template_path, f, 'service.xml')
            if not os.path.isfile(filename):
                continue

            try:
                doc = libxml2.parseFile(filename)
            except libxml2.parserError:
                debug("Failed to parse " + filename)
                continue

            title = ''
            url = ''

            root = doc.getRootElement()
            if (root is None) or (root.name != 'service'):
                debug("Root node is not 'service' in " + filename)
                doc.freeDoc()
                continue
            node = root.children
            while node is not None:
                if node.name == 'title':
                    title = utils.get_content_unicode(node)
                elif node.name == 'ref':
                    url = utils.get_content_unicode(node)
                node = node.next
            doc.freeDoc()

            if (title == '') or (url == ''):
                debug("Empty <title> or <ref> in " + filename)
                continue
            
            menuitems[title.lower()] = ('<link>\n'
                                        '<label>%s</label>\n'
                                        '<ref>%s</ref>\n'
                                        '</link>\n' %
                                        (libxml2.newText(title),
                                         libxml2.newText(url)))
        # Sort the menu items
        titles = menuitems.keys()
        titles.sort()

        # Build the menu
        mainmenu = ('<?xml version="1.0"?>\n'
                    '<wvmenu>\n'
                    '<title>Select video source</title>\n')
        for t in titles:
            mainmenu += menuitems[t]
        mainmenu += '</wvmenu>'

        self.dl = download.DummyDownloader(mainmenu,
                                           writefunc=self.writewrapper,
                                           donefunc=self.request_done)
        self.dl.start()

    def writewrapper(self, inp):
        """Wraps pycurl write callback (with the data as the only
        parameter) into webvi write callback (with signature (data,
        length, usertag)). If self.writefunc is not set, write to
        stdout."""
        if self.writefunc is not None:
            inplen = len(inp)
            written = self.writefunc(inp, inplen, self.writedata)
            if written != inplen:
                self.dl.close()
                self.request_done(405, 'Write callback failed')
        else:
            sys.stdout.write(inp)

    def is_asx_playlist(self, url):
        if utils.get_url_extension(url).lower() == 'asx':
            return True
        else:
            return False

    def get_url_from_asx(self, asx, asxurl):
        """Simple ASX parser. Return the content of the first <ref>
        tag."""
        try:
            doc = libxml2.htmlReadDoc(asx, asxurl, None,
                                      libxml2.HTML_PARSE_NOERROR |
                                      libxml2.HTML_PARSE_NOWARNING |
                                      libxml2.HTML_PARSE_NONET)
        except libxml2.treeError:
            debug('Can\'t parse ASX:\n' + asx)
            return None
        root = doc.getRootElement()
        ret = self._get_ref_recursive(root).strip()
        doc.freeDoc()
        return ret

    def _get_ref_recursive(self, node):
        if node is None:
            return None
        if node.name.lower() == 'ref':
            href = node.prop('href')
            if href is not None:
                return href
        child = node.children
        while child:
            res = self._get_ref_recursive(child)
            if res is not None:
                return res
            child = child.next
        return None

    def parse_mediaurl(self, xml, minpriority, maxpriority):
        debug('parse_mediaurl\n' + xml)

        self.streamtitle = '???'
        mediaurls = []

        try:
            doc = libxml2.parseDoc(xml)
        except libxml2.parserError:
            debug('Invalid XML')
            return mediaurls

        root = doc.getRootElement()
        if root is None:
            debug('No root node')
            return mediaurls
        
        urls_and_priorities = []
        node = root.children
        while node:
            if node.name == 'title':
                self.streamtitle = utils.get_content_unicode(node)
            elif node.name == 'url':
                try:
                    priority = int(node.prop('priority'))
                except (ValueError, TypeError):
                    priority = self.DEFAULT_URL_PRIORITY

                content = node.getContent()
                if priority >= minpriority and priority <= maxpriority and content != '':
                    urls_and_priorities.append((priority, content))
            node = node.next
        doc.freeDoc()

        urls_and_priorities.sort()
        urls_and_priorities.reverse()
        mediaurls = [b[1] for b in urls_and_priorities]

        return mediaurls

    def finished_download(self, err, errmsg):
        if err == 0:
            self.request_done(0, None)
        elif err != 402 and self.mediaurls:
            debug('Download failed (%s %s).\nTrying the next one.' % (err, errmsg))
            self.dl = None
            self.start_download()
        else:
            self.request_done(err, errmsg)

    def finished_playlist_loaded(self, err, errmsg):
        if err == 0:
            url = self.get_url_from_asx(self.dl.get_body(),
                                        self.dl.get_url())
            if url is None:
                err = 404
                errmsg = 'No ref tag in ASX file'
            else:
                if not self.is_asx_playlist(url) and url.startswith('http:'):
                    # The protocol is really "Windows Media HTTP
                    # Streaming Protocol", not plain HTTP, even though
                    # the scheme in the ASX file says "http://". We
                    # can't do MS-WMSP but luckily most MS-WMSP
                    # servers support MMS, too.
                    url = 'mms:' + url[5:]

                if self.type == WebviRequestType.STREAMURL:
                    self.check_and_send_url(url)
                else:
                    self.start_download(url)

        if err != 0:
            if not self.mediaurls:
                self.request_done(err, errmsg)
            else:
                if self.type == WebviRequestType.STREAMURL:
                    self.check_and_send_url()
                else:
                    self.start_download()

    def finished_apply_xslt(self, err, errmsg):
        if err != 0:
            self.request_done(err, errmsg)
            return

        url = self.srcurl

        # Add input documentURL to the parameters
        params = self.xsltparameters.copy()
        params['docurl'] = utils.xpath_str(url)

        minpriority = self.processing.get('minquality', 0)
        maxpriority = self.processing.get('maxquality', 100)

        xsltpath = os.path.join(template_path, self.xsltfile)

        xml = self.dl.get_body()
        encoding = self.dl.get_encoding()

        if self.processing.has_key('postprocess') and \
                'json2xml' in self.processing['postprocess']:
            xmldoc = json2xml.json2xml(xml, encoding)
            if xmldoc is None:
                self.request_done(503, 'Invalid JSON content')
                return
            xml = xmldoc.serialize('utf-8')
            encoding = 'utf-8'

        #debug(xml)

        resulttree = utils.apply_xslt(xml, encoding, url,
                                      xsltpath, params)
        if resulttree is None:
            self.request_done(503, 'XSLT transformation failed')
            return
        
        if self.type == WebviRequestType.MENU:
            debug("result:")
            debug(resulttree)
            self.writewrapper(resulttree)
            self.request_done(0, None)
        elif self.type == WebviRequestType.STREAMURL:
            self.mediaurls = self.parse_mediaurl(resulttree, minpriority, maxpriority)
            if self.mediaurls:
                self.check_and_send_url()
            else:
                self.request_done(406, 'No valid URLs found')
        elif self.type == WebviRequestType.FILE:
            self.mediaurls = self.parse_mediaurl(resulttree, minpriority, maxpriority)
            if self.mediaurls:
                self.start_download()
            else:
                self.request_done(406, 'No valid URLs found')
        else:
            self.request_done(0, None)

    def finished_extract_playlist_url(self, err, errmsg):
        if err == 0:
            url = self.get_url_from_asx(self.dl.get_body(),
                                        self.dl.get_url())
            if url is not None:
                if self.is_asx_playlist(url):
                    self.setup_downloader(url, None, None,
                                          self.finished_extract_playlist_url,
                                          self.processing['HTTP-headers'])
                else:
                    if url.startswith('http:'):
                        url = 'mms:' + url[5:]
                    self.check_and_send_url(url)
            else:
                self.request_done(503, 'XSLT tranformation failed to produce URL')
        else:
            self.request_done(err, errmsg)


    def finished_check_url(self, err, errmsg):
        if err == 0:
            self.writewrapper(self.checking_url)
            self.request_done(0, None)
        else:
            self.check_and_send_url()

    def request_done(self, err, errmsg):
        debug('request_done: %d %s' % (err, errmsg))

        self.status = err
        self.errmsg = errmsg
        self.dl = None

    def is_finished(self):
        return self.status >= 0
        

class RequestList(dict):
    nextreqnum = 1

    def put(self, req):
        reqnum = RequestList.nextreqnum
        RequestList.nextreqnum += 1
        req.handle = reqnum
        self[reqnum] = req
        return reqnum
