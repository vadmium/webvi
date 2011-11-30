# utils.py - misc. utility functions
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

import urlparse
import re
import libxml2
import libxslt
import urllib

def get_url_extension(url):
    """Extracts and returns the file extension from a URL."""
    # The extension is located right before possible query
    # ("?query=foo") or fragment ("#bar").
    try:
        i = url.index('?')
        url = url[:i]
    except ValueError:
        pass
    # The extension is the part after the last '.' that does not
    # contain '/'.
    idot = url.rfind('.')
    islash = url.rfind('/')
    if idot > islash:
        return url[idot+1:]
    else:
        return ''

def urljoin_query_fix(base, url, allow_fragments=True):
    """urlparse.urljoin in Python 2.5 (2.6?) and older is broken in
    case url is a pure query. See http://bugs.python.org/issue1432.
    This handles correctly the case where base is a full (http) url
    and url is a query, and calls urljoin() for other cases."""
    if url.startswith('?'):
        bscheme, bnetloc, bpath, bparams, bquery, bfragment = \
            urlparse.urlparse(base, '', allow_fragments)
        bquery = url[1:]
        return urlparse.urlunparse((bscheme, bnetloc, bpath, 
                                    bparams, bquery, bfragment))
    else:
        return urlparse.urljoin(base, url, allow_fragments)

def get_content_unicode(node):
    """node.getContent() returns an UTF-8 encoded sequence of bytes (a
    string). Convert it to a unicode object."""
    return unicode(node.getContent(), 'UTF-8', 'replace')

def apply_xslt(buf, encoding, url, xsltfile, params=None):
    """Apply xslt transform from file xsltfile to the string buf
    with parameters params. url is the location of buf. Returns
    the transformed file as a string, or None if the
    transformation couldn't be completed."""
    stylesheet = libxslt.parseStylesheetFile(xsltfile)

    if stylesheet is None:
        #self.log_info('Can\'t open stylesheet %s' % xsltfile, 'warning')
        return None
    try:
        # htmlReadDoc fails if the buffer is empty but succeeds
        # (returning an empty tree) if the buffer is a single
        # space.
        if buf == '':
            buf = ' '

        # Guess whether this is an XML or HTML document.
        if buf.startswith('<?xml'):
            doc = libxml2.readDoc(buf, url, None,
                                  libxml2.XML_PARSE_NOERROR |
                                  libxml2.XML_PARSE_NOWARNING |
                                  libxml2.XML_PARSE_NONET)
        else:
            #self.log_info('Using HTML parser', 'debug')
            doc = libxml2.htmlReadDoc(buf, url, encoding,
                                      libxml2.HTML_PARSE_NOERROR |
                                      libxml2.HTML_PARSE_NOWARNING |
                                      libxml2.HTML_PARSE_NONET)
    except libxml2.treeError:
        stylesheet.freeStylesheet()
        #self.log_info('Can\'t parse XML document', 'warning')
        return None
    resultdoc = stylesheet.applyStylesheet(doc, params)
    stylesheet.freeStylesheet()
    doc.freeDoc()
    if resultdoc is None:
        #self.log_info('Can\'t apply stylesheet', 'warning')
        return None

    # Postprocess the document:
    # Resolve relative URLs in srcurl (TODO: this should be done in XSLT)
    root = resultdoc.getRootElement()
    if root is None:
        resultdoc.freeDoc()
        return None

    node2 = root.children
    while node2 is not None:
        if node2.name not in ['link', 'button']:
            node2 = node2.next
            continue

        node = node2.children
        while node is not None:
            if (node.name == 'ref') or (node.name == 'stream') or \
                    (node.name == 'submission'):
                refurl = node.getContent()

                match = re.search(r'\?.*srcurl=([^&]*)', refurl)
                if match is not None:
                    oldurl = urllib.unquote(match.group(1))
                    absurl = urljoin_query_fix(url, oldurl)
                    newurl = refurl[:match.start(1)] + \
                        urllib.quote(absurl) + \
                        refurl[match.end(1):]
                    node.setContent(resultdoc.encodeSpecialChars(newurl))

            node = node.next
        node2 = node2.next

    ret = resultdoc.serialize('UTF-8')
    resultdoc.freeDoc()
    return ret

def xpath_str(str):
    """Convert a string to an X Path expression, which can be tricky if the
    string contains both single (') and double (") quotes. See
    http://kushalm.com/the-perils-of-xpath-expressions-specifically-escaping-quotes
    """
    
    try:
        sq = str.index("'")
    except ValueError:
        return "'" + str + "'"
    
    try:
        dq = str.index('"')
    except ValueError:
        return '"' + str + '"'
    
    quotes = ("'", '"')
    if dq > sq:
        i = 1
    else:
        i = 0
    
    parts = list()
    pos = 0
    while pos < len(str):
        try:
            partend = str.index(quotes[i], pos)
        except:
            partend = len(str)
        parts.append(quotes[i] + str[pos:partend] + quotes[i])
        pos = partend
        i ^= 1
    return 'concat(' + ', '.join(parts) + ')'
