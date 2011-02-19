PREFIX ?= /usr/
SYSLIBDIR = $(PREFIX)/lib

LIBNAME=libwebvi.so
LIBSONAME=$(LIBNAME).0
LIBMINOR=$(LIBSONAME).4

VERSION:=$(shell grep VERSION webvi/version.py | cut -d \' -f 2)
PYLIB:=$(shell python2 pythonlibname.py)
DEFINES:=-DPYTHONSHAREDLIB=\"$(PYLIB)\" -DLIBWEBVI_VERSION=\"$(VERSION)\"
# append -DDEBUG to DEFINES to get debug output

all: $(LIBMINOR)

libwebvi.o: libwebvi.c libwebvi.h
	$(CC) -fPIC -Wall -O2 -g $(CFLAGS) $(DEFINES) `python2-config --cflags` -c -o libwebvi.o libwebvi.c

$(LIBMINOR): libwebvi.o
	$(CC) -shared -Wl,-soname,$(LIBSONAME) -Wl,--as-needed libwebvi.o `python2-config --ldflags` -o $(LIBMINOR)
	ln -sf $(LIBMINOR) $(LIBSONAME)
	ln -sf $(LIBSONAME) $(LIBNAME)

libwebvi.a: libwebvi.o
	ar rsc libwebvi.a libwebvi.o

clean:
	rm -f *.o *~ libwebvi.so* libwebvi.a
	rm -f webvi/*.pyc webvi/*~

install: $(LIBMINOR)
	mkdir -p $(DESTDIR)$(SYSLIBDIR)
	cp --remove-destination -d $(LIBNAME)* $(DESTDIR)$(SYSLIBDIR)
	/sbin/ldconfig $${DESTDIR:+-N} $(DESTDIR)$(SYSLIBDIR)

.PHONY: clean install
