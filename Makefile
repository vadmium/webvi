# prefix for non-VDR stuff
PREFIX ?= /usr/local
# VDR directory
VDRDIR ?= /usr/src/vdr-1.6.0
# VDR's library directory
VDRPLUGINDIR ?= $(VDRDIR)/PLUGINS/lib
# VDR's plugin conf directory
VDRPLUGINCONFDIR ?= /video/plugins
# VDR's locale directory
VDRLOCALEDIR ?= $(VDRDIR)/locale

VERSION := $(shell grep VERSION src/libwebvi/webvi/version.py | cut -d \' -f 2)

TMPDIR = /tmp
ARCHIVE = webvideo-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

APIVERSION := $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h 2> /dev/null)
LIBDIR = $(VDRPLUGINDIR)

# Default target compiles everything but does not install anything.
all-noinstall: libwebvi vdr-plugin

# This target is used by VDR's make plugins. It compiles everything
# and installs VDR plugin.
all: libwebvi vdr-plugin $(LIBDIR)/libvdr-webvideo.so.$(APIVERSION) webvi.conf

vdr-plugin: libwebvi
	$(MAKE) -C src/vdr-plugin LOCALEDIR=./locale LIBDIR=. VDRDIR=$(VDRDIR) CXXFLAGS="-fPIC -g -O2 -Wall -Woverloaded-virtual -Wno-parentheses $(CXXFLAGS)"

libwebvi: build-python
	$(MAKE) -C src/libwebvi all libwebvi.a

build-python: webvi.conf
	python setup.py build

webvi.conf webvi.plugin.conf: %.conf: examples/%.conf
	sed 's_templatepath = /usr/local/share/webvi/templates_templatepath = $(PREFIX)/share/webvi/templates_g' < $< > $@

$(DESTDIR)$(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION): vdr-plugin
ifeq ($(APIVERSION),)
	@echo "No APIVERSION in $(VDRDIR)/config.h"
	@exit 1
else
	mkdir -p $(DESTDIR)$(VDRPLUGINDIR)
	cp -f src/vdr-plugin/libvdr-webvideo.so.$(APIVERSION) $(DESTDIR)$(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION)
endif

install-vdr-plugin: $(DESTDIR)$(VDRPLUGINDIR)/libvdr-webvideo.so.$(APIVERSION)
	mkdir -p $(DESTDIR)$(VDRLOCALEDIR)
	cp -rf src/vdr-plugin/locale/* $(DESTDIR)$(VDRLOCALEDIR)
	mkdir -p $(DESTDIR)$(VDRPLUGINCONFDIR)/webvideo
	cp -f src/vdr-plugin/mime.types $(DESTDIR)$(VDRPLUGINCONFDIR)/webvideo

install-libwebvi: libwebvi
	$(MAKE) -C src/libwebvi install

install-python: uninstall-deprecated-templates
	python setup.py install --skip-build --prefix $(PREFIX) $${DESTDIR:+--root $(DESTDIR)}

install-conf: webvi.conf webvi.plugin.conf
	mkdir -p $(DESTDIR)/etc
	cp -f webvi.conf $(DESTDIR)/etc
	mkdir -p $(DESTDIR)$(VDRPLUGINCONFDIR)/webvideo
	cp -f webvi.plugin.conf $(DESTDIR)$(VDRPLUGINCONFDIR)/webvideo

install-webvi: install-libwebvi install-python

install: install-vdr-plugin install-webvi install-conf

# Template directories were renamed in 0.3.3. Remove old templates.
uninstall-deprecated-templates:
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/youtube
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/svtplay
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/moontv
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/metacafe
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/vimeo
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/katsomo
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/subtv
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/ruutufi
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/google
	rm -rf $(DESTDIR)$(PREFIX)/share/webvi/templates/yleareena

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
