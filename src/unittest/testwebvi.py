#!/usr/bin/python
# -*- coding: utf-8 -*-

# This file is part of vdr-webvideo-plugin.
#
# Copyright 2009-2011 Antti Ajanki <antti.ajanki@iki.fi>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Blackbox tests for each of thee supported video sites and webvicli.

Mainly useful for checking if the web sites have changed so much that
the XSLT templates don't match to them anymore. Requires network
connection because the tests automatically connect and navigate
through links on the video sites.
"""

import unittest
import sys
import re
import urllib
from urlparse import urlparse

sys.path.append('../webvicli')
sys.path.append('../libwebvi')
from webvicli import client, menu
import webvi.api
from webvi.constants import WebviConfig

class TestServiceModules(unittest.TestCase):

    # ========== Helper functions ==========

    def setUp(self):
        webvi.api.set_config(WebviConfig.TEMPLATE_PATH, '../../templates')
        self.client = client.WVClient([], {}, {}, False)
    
    def getLinks(self, menuobj):
        links = []
        for i in xrange(len(menuobj)):
            if isinstance(menuobj[i], menu.MenuItemLink):
                links.append(menuobj[i])
        return links
    
    def downloadMenuPage(self, reference, menuname):
        (status, statusmsg, menuobj) = self.client.getmenu(reference)
        self.assertEqual(status, 0, 'Unexpected status code %s (%s) in %s menu\nFailed ref was %s' % (status, statusmsg, menuname, reference))
        self.assertNotEqual(menuobj, None, 'Failed to get %s menu' % menuname)
        return menuobj

    def downloadAndExtractLinks(self, reference, minlinks, menuname):
        menuobj = self.downloadMenuPage(reference, menuname)
        links = self.getLinks(menuobj)
        self.assertTrue(len(links) >= minlinks, 'Too few links in %s menu' % menuname)
        return links

    def checkMediaUrl(self, reference):
        streamurl = self.client.get_stream_url(reference)
        self.assertNotEqual(streamurl, None, 'get_stream_url returned None')
        self.assertNotEqual(streamurl, '', 'get_stream_url returned empty string')

    def getServiceReference(self, templatedir):
        service = open(templatedir + '/service.xml').read()
        m = re.search(r'<ref>(.*)</ref>', service)
        self.assertNotEqual(m, None, 'no <ref> in service.xml')
        return m.group(1)

    def urlToWvtref(self, url):
        domain = urlparse(url).netloc.lower()
        if domain == '':
            return None

        return 'wvt:///%s/videopage.xsl?srcurl=%s' % (domain, urllib.quote(url, ''))

    def extractQueryParams(self, ref):
        res = {}
        params = {}
        splitref = ref.split('?', 1)
        if len(splitref) == 1:
            return (res, params)

        for param in splitref[1].split('&'):
            keyval = param.split('=', 1)
            key = keyval[0].lower()
            if len(keyval) == 2:
                val = keyval[1]

            if key == 'param':
                psplit = val.split(',', 1)
                pname = psplit[0].lower()
                if len(psplit) == 2:
                    pvalue = psplit[1]
                
                params[pname] = pvalue

            res[key] = urllib.unquote(val)
        return (res, params)

    # ========== Tests for supported websites ==========

    def testMainMenu(self):
        self.downloadAndExtractLinks('wvt:///?srcurl=mainmenu', 4, 'main')

    def testYoutube(self):
        # Category page
        ref = self.getServiceReference('../../templates/www.youtube.com')
        links = self.downloadAndExtractLinks(ref, 3, 'category')

        # Navigation page
        # The third one is the first "proper" category. The first and second are "Search" and "All"
        navigationref = links[2].ref
        links = self.downloadAndExtractLinks(navigationref, 2, 'navigation')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')
        self.checkMediaUrl(videolink.stream)

    def testYoutubeSearch(self):
        menuobj = self.downloadMenuPage('wvt:///www.youtube.com/search.xsl', 'search')
        self.assertTrue(len(menuobj) >= 4, 'Too few items in search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemList))
        self.assertTrue(len(menuobj[1].items) >= 4)
        self.assertTrue(isinstance(menuobj[2], menu.MenuItemList))
        self.assertTrue(len(menuobj[2].items) >= 4)
        self.assertTrue(isinstance(menuobj[3], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'youtube'
        # Sort by: rating
        menuobj[1].current = 3
        # Uploaded: This month
        menuobj[2].current = 3

        resultref = menuobj[3].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testGoogleSearch(self):
        ref = self.getServiceReference('../../templates/video.google.com')
        menuobj = self.downloadMenuPage(ref, 'search')
        self.assertTrue(len(menuobj) == 4, 'Unexpected number of items in Google search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemList))
        self.assertTrue(len(menuobj[1].items) >= 2)
        self.assertTrue(isinstance(menuobj[2], menu.MenuItemList))
        self.assertTrue(len(menuobj[2].items) >= 4)
        self.assertTrue(isinstance(menuobj[3], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'google'
        # Sort by: date
        menuobj[1].current = 3
        # Duration: Short
        menuobj[2].current = 1

        resultref = menuobj[3].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testMetacafe(self):
        # Category page
        ref = self.getServiceReference('../../templates/www.metacafe.com')
        links = self.downloadAndExtractLinks(ref, 3, 'category')

        # The first is "Search", the second is "Channels" and the
        # third is the first "proper" navigation.
        channelsref = links[1].ref
        navigationref = links[2].ref

        # Navigation page
        links = self.downloadAndExtractLinks(navigationref, 2, 'navigation')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')
        self.checkMediaUrl(videolink.stream)

        # User channels
        links = self.downloadAndExtractLinks(channelsref, 3, 'channel list')

    def testMetacafeSearch(self):
        menuobj = self.downloadMenuPage('wvt:///www.metacafe.com/search.xsl', 'search')
        self.assertTrue(len(menuobj) >= 4, 'Too few items in search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemList))
        self.assertTrue(len(menuobj[1].items) == 3)
        self.assertTrue(isinstance(menuobj[2], menu.MenuItemList))
        self.assertTrue(len(menuobj[2].items) == 4)
        self.assertTrue(isinstance(menuobj[3], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'metacafe'
        # Sort by: most discussed
        menuobj[1].current = 2
        # Published: Anytime
        menuobj[2].current = 0

        resultref = menuobj[3].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testVimeo(self):
        # Category page
        ref = self.getServiceReference('../../templates/vimeo.com')
        links = self.downloadAndExtractLinks(ref, 3, 'Vimeo main page')

        # The first is "Search", the second is "Channels" and the
        # third is "Groups"
        channelsref = links[1].ref
        groupsref = links[2].ref

        # Channels page
        links = self.downloadAndExtractLinks(channelsref, 2, 'channels')

        # Navigation page
        links = self.downloadAndExtractLinks(links[0].ref, 2, 'channels navigation')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')
        queries, params = self.extractQueryParams(videolink.stream)
        self.assertTrue('srcurl' in queries, 'Required parameter missing in video link')
        self.checkMediaUrl(videolink.stream)

        # User groups
        links = self.downloadAndExtractLinks(groupsref, 2, 'channel list')

        # Navigation page
        links = self.downloadAndExtractLinks(links[0].ref, 2, 'groups navigation')

    def testVimeoSearch(self):
        menuobj = self.downloadMenuPage('wvt:///vimeo.com/search.xsl?srcurl=http://www.vimeo.com/', 'search')
        self.assertTrue(len(menuobj) >= 3, 'Too few items in search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemList))
        self.assertTrue(len(menuobj[1].items) >= 2)
        self.assertTrue(isinstance(menuobj[2], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'vimeo'
        # Sort by: newest
        menuobj[1].current = 1

        resultref = menuobj[2].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testYLEAreena(self):
        # Category page
        ref = self.getServiceReference('../../templates/areena.yle.fi')
        links = self.downloadAndExtractLinks(ref, 3, 'category')

        # The first is "Search", the second is "live", the third is
        # "all", the rest are navigation links.
        liveref = links[1].ref
        navigationref = links[3].ref

        # Navigation page
        links = self.downloadAndExtractLinks(navigationref, 2, 'navigation')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')

        # Direct video page link
        queries, params = self.extractQueryParams(videolink.stream)
        self.assertTrue('srcurl' in queries, 'Required parameter missing in video link')
        videopageurl = queries['srcurl']
        videopageref = self.urlToWvtref(videopageurl)
        self.checkMediaUrl(videopageref)

        # live broadcasts
        links = self.downloadAndExtractLinks(liveref, 2, 'live broadcasts')

    def testYLEAreenaSearch(self):
        menuobj = self.downloadMenuPage('wvt:///areena.yle.fi/search.xsl?srcurl=http://areena.yle.fi/haku', 'search')
        self.assertTrue(len(menuobj) >= 8, 'Too few items in search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemList))
        self.assertTrue(len(menuobj[1].items) >= 3)
        self.assertTrue(isinstance(menuobj[2], menu.MenuItemList))
        self.assertTrue(len(menuobj[2].items) >= 2)
        self.assertTrue(isinstance(menuobj[3], menu.MenuItemList))
        self.assertTrue(len(menuobj[3].items) >= 2)
        self.assertTrue(isinstance(menuobj[4], menu.MenuItemList))
        self.assertTrue(len(menuobj[4].items) >= 3)
        self.assertTrue(isinstance(menuobj[5], menu.MenuItemList))
        self.assertTrue(len(menuobj[5].items) >= 4)
        self.assertTrue(isinstance(menuobj[6], menu.MenuItemList))
        self.assertTrue(len(menuobj[6].items) >= 2)
        self.assertTrue(isinstance(menuobj[7], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'yle'
        # Media: video
        menuobj[1].current = 1
        # Category: all
        menuobj[2].current = 0
        # Channel: all
        menuobj[3].current = 0
        # Language: Finnish
        menuobj[4].current = 1
        # Uploaded: all
        menuobj[5].current = 0
        # Only outside Finland: no
        menuobj[6].current = 0

        resultref = menuobj[7].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testKatsomo(self):
        # Category page
        ref = self.getServiceReference('../../templates/katsomo.fi')
        links = self.downloadAndExtractLinks(ref, 2, 'category')

        # The first is "Search", the rest are navigation links.
        navigationref = links[1].ref

        # Navigation page
        links = self.downloadAndExtractLinks(navigationref, 1, 'navigation')

        # Program page
        links = self.downloadAndExtractLinks(links[0].ref, 1, 'program')

        # Video link
        # The first few links may be navigation links, but there
        # should be video links after them.
        foundVideo = False
        for link in links:
            if link.stream is not None:
                foundVideo = True

        self.assertTrue(foundVideo, 'No a video links in the program page')

    def testKatsomoSearch(self):
        menuobj = self.downloadMenuPage('wvt:///katsomo.fi/search.xsl', 'search')
        self.assertTrue(len(menuobj) >= 2, 'Too few items in search menu')

        self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
        self.assertTrue(isinstance(menuobj[1], menu.MenuItemSubmitButton))

        # Query term
        menuobj[0].value = 'mtv3'

        resultref = menuobj[1].activate()
        self.assertNotEqual(resultref, None)
        self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testRuutuFi(self):
        # Category page
        ref = self.getServiceReference('../../templates/www.ruutu.fi')
        links = self.downloadAndExtractLinks(ref, 3, 'category')

        seriesref = links[0].ref

        # Series page
        links = self.downloadAndExtractLinks(seriesref, 1, 'series')

        # Program page
        links = self.downloadAndExtractLinks(links[0].ref, 1, 'program')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')

        # Direct video page link
        queries, params = self.extractQueryParams(links[0].stream)
        self.assertTrue('srcurl' in queries, 'Required parameter missing in video link')
        queries, params = self.extractQueryParams(queries['srcurl'])
        self.assertTrue('q' in queries, 'Required parameter missing in video link')
        vt, vid = queries['q'].split('/')
        videopageurl = 'http://www.ruutu.fi/video?vt=%s&vid=%s' % (vt, vid)
        videopageref = self.urlToWvtref(videopageurl)
        self.checkMediaUrl(videopageref)
        
    # def testRuutuFiSearch(self):
    #     menuobj = self.downloadMenuPage('wvt:///www.ruutu.fi/search.xsl', 'search')
    #     self.assertTrue(len(menuobj) >= 2, 'Too few items in search menu')

    #     self.assertTrue(isinstance(menuobj[0], menu.MenuItemTextField))
    #     self.assertTrue(isinstance(menuobj[1], menu.MenuItemSubmitButton))

    #     # Query term
    #     menuobj[0].value = 'nelonen'

    #     resultref = menuobj[1].activate()
    #     self.assertNotEqual(resultref, None)
    #     self.downloadAndExtractLinks(resultref, 1, 'search result')

    def testSubtv(self):
        # Category page
        ref = self.getServiceReference('../../templates/www.sub.fi')
        links = self.downloadAndExtractLinks(ref, 4, 'series')

        # Program page
        links = self.downloadAndExtractLinks(links[0].ref, 1, 'program')

        # Video link
        videolink = links[0]
        self.assertNotEqual(videolink.stream, None, 'No media object in a video link')
        self.assertNotEqual(videolink.ref, None, 'No description page in a video link')

        # Direct video page link
        queries, params = self.extractQueryParams(links[0].stream)
        self.assertTrue('srcurl' in queries and 'pid' in params, 'Required parameter missing in video link')
        videopageurl = queries['srcurl'] + '?' + params['pid']
        videopageref = self.urlToWvtref(videopageurl)
        self.checkMediaUrl(videopageref)
        


if __name__ == '__main__':
    testnames = sys.argv[1:]

    if not testnames:
        # Run all tests
        unittest.main()
    else:
        # Run test listed on the command line
        for test in testnames:
            suite = unittest.TestSuite()
            suite.addTest(TestServiceModules(test))
            unittest.TextTestRunner(verbosity=2).run(suite)
