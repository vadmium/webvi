import os
import os.path
import sys
from distutils.core import setup

def extract_version():
    sys.path.append('src/libwebvi/webvi')
    import version
    sys.path.pop()
    return version.VERSION

def install_service_files():
    sourcedir = 'templates'
    destdir = 'share/webvi/templates'

    res = []
    for service in os.listdir(sourcedir):
        sdir = os.path.join(sourcedir, service)
        sfiles = []
        for f in os.listdir(sdir):
            sfiles.append(os.path.join(sdir, f))
        res.append((os.path.join(destdir, service), sfiles))
    return res

setup(
  name='libwebvi',
  version=extract_version(),
  description='webvideo downloader library and command line client',
  author='Antti Ajanki',
  author_email='antti.ajanki@iki.fi',
  license='GPLv3',
  url='http://users.tkk.fi/~aajanki/vdr/webvideo',
  package_dir = {'webvi': 'src/libwebvi/webvi', 'webvicli': 'src/webvicli/webvicli'},
  packages=['webvi', 'webvicli'],
  scripts=['src/webvicli/webvi'],
  data_files=install_service_files()
  )
