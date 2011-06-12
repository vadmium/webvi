#!/usr/bin/env python

# webvicli.py - webvi command line client
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

import cStringIO
import sys
import cmd
import mimetypes
import select
import os.path
import subprocess
import time
import re
import urllib
import libxml2
import webvi.api
import webvi.utils
from optparse import OptionParser
from ConfigParser import RawConfigParser
from urlparse import urlparse
from webvi.constants import WebviRequestType, WebviOpt, WebviInfo, WebviSelectBitmask, WebviConfig
from . import menu

VERSION = '0.4.2'

# Default options
DEFAULT_PLAYERS = ['vlc --play-and-exit "%s"', 
                   'totem "%s"', 
                   'mplayer "%s"', 
                   'xine "%s"']

# These mimetypes are common but often missing
mimetypes.init()
mimetypes.add_type('video/flv', '.flv')
mimetypes.add_type('video/x-flv', '.flv')

def safe_filename(name, vfat):
    """Sanitize a filename. If vfat is False, replace '/' with '_', if
    vfat is True, replace also other characters that are illegal on
    VFAT. Remove dots from the beginning of the filename."""
    if vfat:
        excludechars = r'[\\"*/:<>?|]'
    else:
        excludechars = r'[/]'

    res = re.sub(excludechars, '_', name)
    res = res.lstrip('.')
    res = res.encode(sys.getfilesystemencoding(), 'ignore')

    return res

class DownloadData:
    def __init__(self, handle, progressstream):
        self.handle = handle
        self.destfile = None
        self.destfilename = ''
        self.contentlength = -1
        self.bytes_downloaded = 0
        self.progress = ProgressMeter(progressstream)

class ProgressMeter:
    def __init__(self, stream):
        self.last_update = None
        self.samples = []
        self.total_bytes = 0
        self.stream = stream
        self.progress_len = 0
        self.starttime = time.time()

    def pretty_bytes(self, bytes):
        """Pretty print bytes as kB or MB."""
        if bytes < 1100:
            return '%d B' % bytes
        elif bytes < 1024*1024:
            return '%.1f kB' % (float(bytes)/1024)
        elif bytes < 1024*1024*1024:
            return '%.1f MB' % (float(bytes)/1024/1024)
        else:
            return '%.1f GB' % (float(bytes)/1024/1024/1024)

    def pretty_time(self, seconds):
        """Pretty print seconds as hour and minutes."""
        seconds = int(round(seconds))
        if seconds < 60:
            return '%d s' % seconds
        elif seconds < 60*60:
            secs = seconds % 60
            mins = seconds/60
            return '%d min %d s' % (mins, secs)
        else:
            hours = seconds / (60*60)
            mins = (seconds-60*60*hours) / 60
            return '%d hours %d min' % (hours, mins)
        
    def update(self, bytes):
        """Update progress bar.

        Updates the estimates of download rate and remaining time.
        Prints progress bar, if at least one second has passed since
        the previous update.
        """
        now = time.time()

        if self.total_bytes > 0:
            percentage = float(bytes)/self.total_bytes * 100.0
        else:
            percentage = 0

        if self.total_bytes > 0 and bytes >= self.total_bytes:
            self.stream.write('\r')
            self.stream.write(' '*self.progress_len)
            self.stream.write('\r')
            self.stream.write('%3.f %% of %s downloaded in %s (%.1f kB/s)\n' % 
                              (percentage, self.pretty_bytes(self.total_bytes), 
                               self.pretty_time(now-self.starttime),
                               float(bytes)/(now-self.starttime)/1024.0))
            self.stream.flush()
            return

        force_refresh = False
        if self.last_update is None:
            # This is a new progress meter
            self.last_update = now
            force_refresh = True

        if (not force_refresh) and (now <= self.last_update + 1):
            # do not update too often
            return

        self.last_update = now

        # Estimate bytes per second rate from the last 10 samples
        self.samples.append((bytes, now))
        if len(self.samples) > 10:
            self.samples.pop(0)

        bytes_old, time_old = self.samples[0]
        if now > time_old:
            rate = float(bytes-bytes_old)/(now-time_old)
        else:
            rate = 0

        if self.total_bytes > 0:
            remaining = self.total_bytes - bytes
        
            if rate > 0:
                time_left = self.pretty_time(remaining/rate)
            else:
                time_left = '???'
        
            progress = '%3.f %% of %s (%.1f kB/s) %s remaining' % \
                       (percentage, self.pretty_bytes(self.total_bytes),
                        rate/1024.0, time_left)
        else:
            progress = '%s downloaded (%.1f kB/s)' % \
                       (self.pretty_bytes(bytes), rate/1024.0)

        new_progress_len = len(progress)
        if new_progress_len < self.progress_len:
            progress += ' '*(self.progress_len - new_progress_len)
        self.progress_len = new_progress_len

        self.stream.write('\r')
        self.stream.write(progress)
        self.stream.flush()


class WVClient:
    def __init__(self, streamplayers, downloadlimits, streamlimits, vfatfilenames):
        self.streamplayers = streamplayers
        self.history = []
        self.history_pointer = 0
        self.quality_limits = {'download': downloadlimits,
                                'stream': streamlimits}
        self.vfatfilenames = vfatfilenames

    def parse_page(self, page):
        if page is None:
            return None
        try:
            doc = libxml2.parseDoc(page)
        except libxml2.parserError:
            return None

        root = doc.getRootElement()
        if root.name != 'wvmenu':
            return None
        queryitems = []
        menupage = menu.Menu()
        node = root.children
        while node:
            if node.name == 'title':
                menupage.title = webvi.utils.get_content_unicode(node)
            elif node.name == 'link':
                menuitem = self.parse_link(node)
                menupage.add(menuitem)
            elif node.name == 'textfield':
                menuitem = self.parse_textfield(node)
                menupage.add(menuitem)
                queryitems.append(menuitem)
            elif node.name == 'itemlist':
                menuitem = self.parse_itemlist(node)
                menupage.add(menuitem)
                queryitems.append(menuitem)
            elif node.name == 'textarea':
                menuitem = self.parse_textarea(node)
                menupage.add(menuitem)
            elif node.name == 'button':
                menuitem = self.parse_button(node, queryitems)
                menupage.add(menuitem)
            node = node.next
        doc.freeDoc()
        return menupage
        
    def parse_link(self, node):
        label = ''
        ref = None
        stream = None
        child = node.children
        while child:
            if child.name == 'label':
                label = webvi.utils.get_content_unicode(child)
            elif child.name == 'ref':
                ref = webvi.utils.get_content_unicode(child)
            elif child.name == 'stream':
                stream = webvi.utils.get_content_unicode(child)
            child = child.next
        return menu.MenuItemLink(label, ref, stream)

    def parse_textfield(self, node):
        label = ''
        name = node.prop('name')
        child = node.children
        while child:
            if child.name == 'label':
                label = webvi.utils.get_content_unicode(child)
            child = child.next
        return menu.MenuItemTextField(label, name)

    def parse_textarea(self, node):
        label = ''
        child = node.children
        while child:
            if child.name == 'label':
                label = webvi.utils.get_content_unicode(child)
            child = child.next
        return menu.MenuItemTextArea(label)

    def parse_itemlist(self, node):
        label = ''
        name = node.prop('name')
        items = []
        values = []
        child = node.children
        while child:
            if child.name == 'label':
                label = webvi.utils.get_content_unicode(child)
            elif child.name == 'item':
                items.append(webvi.utils.get_content_unicode(child))
                values.append(child.prop('value'))
            child = child.next
        return menu.MenuItemList(label, name, items, values, sys.stdout)

    def parse_button(self, node, queryitems):
        label = ''
        submission = None
        encoding = 'utf-8'
        child = node.children
        while child:
            if child.name == 'label':
                label = webvi.utils.get_content_unicode(child)
            elif child.name == 'submission':
                submission = webvi.utils.get_content_unicode(child)
                enc = child.hasProp('encoding')
                if enc is not None:
                    encoding = webvi.utils.get_content_unicode(enc)
            child = child.next
        return menu.MenuItemSubmitButton(label, submission, queryitems, encoding)

    def guess_extension(self, mimetype, url):
        ext = mimetypes.guess_extension(mimetype)
        if (ext is None) or (mimetype == 'text/plain'):
            # This function is only called for video files. Try to
            # extract the extension from url because text/plain is
            # clearly wrong.
            lastcomponent = re.split(r'[?#]', url, 1)[0].split('/')[-1]
            i = lastcomponent.rfind('.')
            if i == -1:
                ext = ''
            else:
                ext = lastcomponent[i:]
            
        return ext

    def execute_webvi(self, handle):
        """Call webvi.api.perform until handle is finished."""
        while True:
            rescode, readfds, writefds, excfds, maxfd = webvi.api.fdset()
            if [] == readfds == writefds == excfds:
                finished, status, errmsg, remaining = webvi.api.pop_message()
                if finished == handle:
                    return (status, errmsg)
                elif finished != -1:
                    return (501, 'Unexpected handle (got %d, expected %d)' % (finished, handle))
                else:
                    return (501, 'No active sockets')
            
            readyread, readywrite, readyexc = select.select(readfds, writefds, excfds, 30.0)

            for fd in readyread:
                webvi.api.perform(fd, WebviSelectBitmask.READ)
            for fd in readywrite:
                webvi.api.perform(fd, WebviSelectBitmask.WRITE)

            remaining = -1
            while remaining != 0:
                finished, status, errmsg, remaining = webvi.api.pop_message()
                if finished == handle:
                    return (status, errmsg)

    def collect_data(self, inp, inplen, dlbuffer):
        """Callback that writes the downloaded data to dlbuffer.
        """
        dlbuffer.write(inp)
        return inplen

    def open_dest_file(self, inp, inplen, dldata):
        """Initial download callback. This opens the destination file,
        and reseats the callback to self.write_to_dest. The
        destination file can not be opened until now, because the
        stream title and final URL are not known before.
        """
        title = webvi.api.get_info(dldata.handle, WebviInfo.STREAM_TITLE)[1]
        contenttype = webvi.api.get_info(dldata.handle, WebviInfo.CONTENT_TYPE)[1]
        contentlength = webvi.api.get_info(dldata.handle, WebviInfo.CONTENT_LENGTH)[1]
        url = webvi.api.get_info(dldata.handle, WebviInfo.URL)[1]
        ext = self.guess_extension(contenttype, url)

        safename = safe_filename(title, self.vfatfilenames)
        destfilename = self.next_available_file_name(safename, ext)

        try:
            destfile = open(destfilename, 'w')
        except IOError, err:
            print 'Failed to open the destination file %s: %s' % (destfilename, err.args[1])
            return -1

        dldata.destfile = destfile
        dldata.destfilename = destfilename
        dldata.contentlength = contentlength
        dldata.progress.total_bytes = contentlength
        webvi.api.set_opt(dldata.handle, WebviOpt.WRITEFUNC, self.write_to_dest)
        
        return self.write_to_dest(inp, inplen, dldata)

    def write_to_dest(self, inp, inplen, dldata):
        """Callback that writes downloaded data to self.destfile."""
        try:
            dldata.destfile.write(inp)
        except IOError, err:
            print 'IOError while writing to %s: %s' % \
                  (dldata.destfilename, err.args[1])
            return -1

        dldata.bytes_downloaded += inplen

        dldata.progress.update(dldata.bytes_downloaded)
        
        return inplen

    def getmenu(self, ref):
        dlbuffer = cStringIO.StringIO()
        handle = webvi.api.new_request(ref, WebviRequestType.MENU)
        if handle == -1:
            print 'Failed to open handle'
            return (-1, '', None)
        
        webvi.api.set_opt(handle, WebviOpt.WRITEFUNC, self.collect_data)
        webvi.api.set_opt(handle, WebviOpt.WRITEDATA, dlbuffer)
        webvi.api.start_handle(handle)
        
        status, err = self.execute_webvi(handle)
        webvi.api.delete_handle(handle)

        if status != 0:
            print 'Download failed:', err
            return (status, err, None)

        return (status, err, self.parse_page(dlbuffer.getvalue()))

    def get_quality_params(self, videosite, streamtype):
        params = []
        lim = self.quality_limits[streamtype].get(videosite, {})
        if lim.has_key('min'):
            params.append('minquality=' + lim['min'])
        if lim.has_key('max'):
            params.append('maxquality=' + lim['max'])

        return '&'.join(params)
        
    def download(self, stream):
        m = re.match(r'wvt:///([^/]+)/', stream)
        if m is not None:
            stream += '&' + self.get_quality_params(m.group(1), 'download')

        handle = webvi.api.new_request(stream, WebviRequestType.FILE)
        if handle == -1:
            print 'Failed to open handle'
            return False

        dldata = DownloadData(handle, sys.stdout)
        
        webvi.api.set_opt(handle, WebviOpt.WRITEFUNC, self.open_dest_file)
        webvi.api.set_opt(handle, WebviOpt.WRITEDATA, dldata)
        webvi.api.start_handle(handle)

        status, err = self.execute_webvi(handle)
        if dldata.destfile is not None:
            dldata.destfile.close()

        webvi.api.delete_handle(handle)

        if status not in (0, 504):
            print 'Download failed:', err
            return

        if dldata.contentlength != -1 and \
               dldata.bytes_downloaded != dldata.contentlength:
            print 'Warning: the size of the file (%d) differs from expected (%d)' % \
                  (dldata.bytes_downloaded, dldata.contentlength)

        print 'Saved to %s' % dldata.destfilename

        return True

    def play_stream(self, ref):
        streamurl = self.get_stream_url(ref)
        if streamurl == '':
            print 'Did not find URL'
            return False

        if streamurl.startswith('wvt://'):
            print 'Streaming not supported, try downloading'
            return False

        # Found url, now find a working media player
        for player in self.streamplayers:
            if '%s' not in player:
                playcmd = player + ' ' + streamurl
            else:
                try:
                    playcmd = player % streamurl
                except TypeError:
                    print 'Can\'t substitute URL in', player
                    continue

            try:
                print 'Trying player: ' + playcmd
                retcode = subprocess.call(playcmd, shell=True)
                if retcode > 0:
                    print 'Player failed with returncode', retcode
                else:
                    return True
            except OSError, err:
                print 'Execution failed:', err

        return False

    def get_stream_url(self, ref):
        m = re.match(r'wvt:///([^/]+)/', ref)
        if m is not None:
            ref += '&' + self.get_quality_params(m.group(1), 'stream')

        handle = webvi.api.new_request(ref, WebviRequestType.STREAMURL)
        if handle == -1:
            print 'Failed to open handle'
            return ''

        dlbuffer = cStringIO.StringIO()
        webvi.api.set_opt(handle, WebviOpt.WRITEFUNC, self.collect_data)
        webvi.api.set_opt(handle, WebviOpt.WRITEDATA, dlbuffer)
        webvi.api.start_handle(handle)
        status, err = self.execute_webvi(handle)
        webvi.api.delete_handle(handle)
        
        if status != 0:
            print 'Download failed:', err
            return ''

        return dlbuffer.getvalue()

    def next_available_file_name(self, basename, ext):
        fullname = basename + ext
        if not os.path.exists(fullname):
            return fullname
        i = 1
        while os.path.exists('%s-%d%s' % (basename, i, ext)):
            i += 1
        return '%s-%d%s' % (basename, i, ext)

    def get_current_menu(self):
        if (self.history_pointer >= 0) and \
               (self.history_pointer < len(self.history)):
            return self.history[self.history_pointer]
        else:
            return None

    def history_add(self, menupage):
        if menupage is not None:
            self.history = self.history[:(self.history_pointer+1)]
            self.history.append(menupage)
            self.history_pointer = len(self.history)-1

    def history_back(self):
        if self.history_pointer > 0:
            self.history_pointer -= 1
        return self.get_current_menu()

    def history_forward(self):
        if self.history_pointer < len(self.history)-1:
            self.history_pointer += 1
        return self.get_current_menu()


class WVShell(cmd.Cmd):
    def __init__(self, client, completekey='tab', stdin=None, stdout=None):
        cmd.Cmd.__init__(self, completekey, stdin, stdout)
        self.prompt = '> '
        self.client = client

    def preloop(self):
        self.stdout.write('webvicli %s starting\n' % VERSION)
        self.do_menu(None)

    def precmd(self, arg):
        try:
            int(arg)
            menuitem = self._get_numbered_item(int(arg))
            if getattr(menuitem, 'ref', None) is None and \
                    getattr(menuitem, 'stream', None) is not None:
                return 'download ' + arg
            else:
                return 'select ' + arg
        except ValueError:
            return arg

    def onecmd(self, c):
        try:
            return cmd.Cmd.onecmd(self, c)
        except Exception:
            import traceback
            print 'Exception occured while handling command "' + c + '"'
            print traceback.format_exc()
            return False

    def emptyline(self):
        pass

    def display_menu(self, menupage):
        if menupage is not None:
            self.stdout.write(unicode(menupage).encode(self.stdout.encoding, 'replace'))
    
    def _get_numbered_item(self, arg):
        menupage = self.client.get_current_menu()
        try:
            v = int(arg)-1
            if (menupage is None) or (v < 0) or (v >= len(menupage)):
                raise ValueError
        except ValueError:
            self.stdout.write('Invalid selection: %s\n' % arg)
            return None
        return menupage[v]

    def url_to_wvtref(self, url):
        domain = urlparse(url).netloc.lower()
        if domain == '':
            return None

        return 'wvt:///%s/videopage.xsl?srcurl=%s' % (domain, urllib.quote(url, ''))
        
    def do_select(self, arg):
        """select x
Select the link whose index is x.
        """
        menuitem = self._get_numbered_item(arg)
        if menuitem is None:
            return False
        ref = menuitem.activate()
        if ref is not None:
            status, statusmsg, menupage = self.client.getmenu(ref)
            if menupage is not None:
                self.client.history_add(menupage)
            else:
                self.stdout.write('Error: %d %s\n' % (status, statusmsg))
        else:
            menupage = self.client.get_current_menu()
        self.display_menu(menupage)
        return False

    def do_download(self, arg):
        """download x
Download a stream to a file. x can be an integer referring to a
downloadable item (item without brackets) in the current menu or an
URL of a video page.
        """
        stream = None
        try:
            menuitem = self._get_numbered_item(int(arg))
            if menuitem is not None:
                stream = menuitem.stream
        except (ValueError, AttributeError):
            pass

        if stream is None and arg.find('://') != -1:
            stream = self.url_to_wvtref(arg)

        if stream is not None:
            self.client.download(stream)
        else:
            self.stdout.write('Not a stream\n')
        return False

    def do_stream(self, arg):
        """stream x
Play a stream. x can be an integer referring to a downloadable item
(item without brackets) in the current menu or an URL of a video page.
        """
        stream = None
        try:
            menuitem = self._get_numbered_item(int(arg))
            if menuitem is not None:
                stream = menuitem.stream
        except (ValueError, AttributeError):
            pass

        if stream is None and arg.find('://') != -1:
            stream = self.url_to_wvtref(arg)

        if stream is not None:
            self.client.play_stream(stream)
        else:
            self.stdout.write('Not a stream\n')
        return False

    def do_display(self, arg):
        """Redisplay the current menu."""
        if not arg:
            self.display_menu(self.client.get_current_menu())
        else:
            self.stdout.write('Unknown parameter %s\n' % arg)
        return False

    def do_menu(self, arg):
        """Get back to the main menu."""
        status, statusmsg, menupage = self.client.getmenu('wvt:///?srcurl=mainmenu')
        if menupage is not None:
            self.client.history_add(menupage)
            self.display_menu(menupage)
        else:
            self.stdout.write('Error: %d %s\n' % (status, statusmsg))
            return True
        return False

    def do_back(self, arg):
        """Go to the previous menu in the history."""
        menupage = self.client.history_back()
        self.display_menu(menupage)
        return False

    def do_forward(self, arg):
        """Go to the next menu in the history."""
        menupage = self.client.history_forward()
        self.display_menu(menupage)
        return False

    def do_quit(self, arg):
        """Quit the program."""
        return True

    def do_EOF(self, arg):
        """Quit the program."""
        return True


def load_config(options):
    """Load options from config files."""
    cfgprs = RawConfigParser()
    cfgprs.read(['/etc/webvi.conf', os.path.expanduser('~/.webvi')])
    for sec in cfgprs.sections():
        if sec == 'webvi':
            for opt, val in cfgprs.items('webvi'):
                if opt in ['vfat', 'verbose']:
                    try:
                        options[opt] = cfgprs.getboolean(sec, opt)
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

        else:
            sitename = urlparse(sec).netloc
            if sitename == '':
                sitename = sec

            if not options.has_key('download-limits'):
                options['download-limits'] = {}
            if not options.has_key('stream-limits'):
                options['stream-limits'] = {}
            options['download-limits'][sitename] = {}
            options['stream-limits'][sitename] = {}

            for opt, val in cfgprs.items(sec):
                if opt == 'download-min-quality':
                    options['download-limits'][sitename]['min'] = val
                elif opt == 'download-max-quality':
                    options['download-limits'][sitename]['max'] = val
                elif opt == 'stream-min-quality':
                    options['stream-limits'][sitename]['min'] = val
                elif opt == 'stream-max-quality':
                    options['stream-limits'][sitename]['max'] = val

    return options

def parse_command_line(cmdlineargs, options):
    parser = OptionParser()
    parser.add_option('-t', '--templatepath', type='string',
                      dest='templatepath',
                      help='read video site templates from DIR', metavar='DIR',
                      default=None)
    parser.add_option('-v', '--verbose', action='store_const', const=1,
                      dest='verbose', help='debug output', default=0)
    parser.add_option('--vfat', action='store_true',
                      dest='vfat', default=False,
                      help='generate Windows compatible filenames')
    cmdlineopt = parser.parse_args(cmdlineargs)[0]

    if cmdlineopt.templatepath is not None:
        options['templatepath'] = cmdlineopt.templatepath
    if cmdlineopt.verbose > 0:
        options['verbose'] = cmdlineopt.verbose
    if cmdlineopt.vfat:
        options['vfat'] = cmdlineopt.vfat

    return options

def player_list(options):
    """Return a sorted list of player commands extracted from options
    dictionary."""
    # Load streamplayer items from the config file and sort them
    # according to quality.
    players = []
    for opt, val in options.iteritems():
        m = re.match(r'streamplayer([1-9])$', opt)
        if m is not None:
            players.append((int(m.group(1)), val))

    players.sort()
    ret = []
    for quality, playcmd in players:
        ret.append(playcmd)

    # If the config file did not define any players use the default
    # players
    if not ret:
        ret = list(DEFAULT_PLAYERS)

    return ret

def main(argv):
    options = load_config({})
    options = parse_command_line(argv, options)

    if options.has_key('verbose'):
        webvi.api.set_config(WebviConfig.DEBUG, options['verbose'])
    if options.has_key('templatepath'):
        webvi.api.set_config(WebviConfig.TEMPLATE_PATH, options['templatepath'])

    shell = WVShell(WVClient(player_list(options),
                             options.get('download-limits', {}),
                             options.get('stream-limits', {}),
                             options.get('vfat', False)))
    shell.cmdloop()

if __name__ == '__main__':
    main([])
