# prefix for non-VDR stuff
PREFIX ?= /usr/local
# VDR directory
VDRDIR ?= /usr/src/vdr-1.6.0
# VDR's library directory
VDRPLUGINDIR ?= $(VDRDIR)/PLUGINS/lib
# VDR's plugin conf directory
VDRPLUGINCONFDIR ?= /video
# VDR's locale directory
VDRLOCALEDIR ?= $(VDRDIR)/locale

VERSION := $(shell cat src/version)

TMPDIR = /tmp
ARCHIVE = webvideo-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

APIVERSION := $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
LIBDIR = $(VDRPLUGINDIR)

# Default target compiles everything but does not install anything.
all-noinstall: libwebvi vdr-plugin

# This target is used by VDR's make plugins. It compiles everything
# and installs VDR plugin.
all: libwebvi vdr-plugin $(LIBDIR)/libvdr-webvideo.so.$(APIVERSION) webvi.conf

vdr-plugin: libwebvi
	$(MAKE) -C src/vdr-plugin LOCALEDIR=./locale LIBDIR=. VDRDIR=$(VDRDIR) CXXFLAGS="-fPIC -g -O2 -Wall -Woverloaded-virtual -Wno-parentheses"

libwebvi: build-python
	$(MAKE) -C src/libwebvi all libwebvi.a

build-python: webvi.conf
	python setup.py build

webvi.conf:
	@echo "[webvi]\n\ntemplatepath = $(PREFIX)/share/webvi/templates" > webvi.conf

$(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION): vdr-plugin
	mkdir -p $(VDRPLUGINDIR)
	cp -f src/vdr-plugin/libvdr-webvideo.so.$(APIVERSION) $(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION)

install-vdr-plugin: vdr-plugin $(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION)
	mkdir -p $(VDRLOCALEDIR)
	cp -rf src/vdr-plugin/locale/* $(VDRLOCALEDIR)
	mkdir -p $(VDRPLUGINCONFDIR)/webvideo
	cp -f src/vdr-plugin/mime.types $(VDRPLUGINCONFDIR)/webvideo

install-libwebvi: libwebvi
	$(MAKE) -C src/libwebvi install

install-python:
	python setup.py install --prefix $(PREFIX)

install-conf: webvi.conf
	cp -f webvi.conf /etc/

install-webvi: install-libwebvi install-python

install: install-vdr-plugin install-webvi install-conf

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	$(MAKE) -C src/vdr-plugin clean
	$(MAKE) -C src/libwebvi clean
	rm -rf src/vdr-plugin/locale webvi.conf
	python setup.py clean -a
	find . -name "*~" -exec rm {} \;
	find . -name "*.pyc" -exec rm {} \;

.PHONY: vdr-plugin libwebvi build-python install install-vdr-plugin install-webvi dist clean
