#!/usr/bin/python

# Proxy for relaying commands to play a video on VDR from a web
# browser to VDR.
#
# Listens for HTTP GET /play?url=XXX requests where XXX is the address
# of the video page (not address of the video stream) and converts
# them to webvideo plugin SVDRP commands. The bookmarklet in
# webvi_bookmarklet.js generates such requests. See README for the
# list of supported video sites.
#
# Antti Ajanki <antti.ajanki@iki.fi>

import urllib
import socket
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from optparse import OptionParser

SVDRP_ADDRESS = ('', 2001)  # default port is 6419 starting from VDR 1.7.15

class SVDRPRequestHandler(BaseHTTPRequestHandler):
    def send(self, cmd):
        self.sock.sendall(cmd)
        self.sock.sendall('\r\n')

    def do_GET(self):
        if self.path.startswith('/play?url='):
            videopage = urllib.unquote(self.path[len('/play?url='):])

            # Strip everything after the first linefeed to prevent
            # SVDRP command injection.
            videopage = videopage.split('\r', 1)[0].split('\n', 1)[0]

            try:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.settimeout(10)
                self.sock.connect(SVDRP_ADDRESS)
                self.send('plug webvideo play %s' % videopage)
                self.send('quit')
                while len(self.sock.recv(4096)) > 0:
                    pass
                self.sock.close()

                self.send_response(204, 'OK')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
            except socket.error, exc:
                self.send_response(503, 'SVDRP connection error: %s' % exc)
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
            except socket.timeout:
                self.send_response(504, 'SVDRP timeout')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
        else:
            self.send_response(404, 'Not found')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
    
def main():
    parser = OptionParser()
    parser.add_option('-s', '--svdrpport', dest='svdrpport',
                      type='int', default=2001, help='set SVDRP port')
    parser.add_option('-d', '--svdrpaddress', dest='svdrpaddress',
                      default='', help='set SVDRP address')
    parser.add_option('-l', '--listen', dest='listenport',
                      type='int', default=43280, help='listen to connection on this port')
    (options, args) = parser.parse_args()

    global SVDRP_ADDRESS
    SVDRP_ADDRESS = (options.svdrpaddress, options.svdrpport)

    httpd = HTTPServer(('', options.listenport), SVDRPRequestHandler)
    httpd.serve_forever()

if __name__ == '__main__':
    main()