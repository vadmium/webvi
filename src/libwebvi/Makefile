PREFIX ?= /usr/local

LIBNAME=libwebvi.so
LIBSONAME=$(LIBNAME).0
LIBMINOR=$(LIBSONAME).4

VERSION:=$(shell cat ../version)
PYLIB:=$(shell python pythonlibname.py)
DEFINES:=-DPYTHONSHAREDLIB=\"$(PYLIB)\" -DLIBWEBVI_VERSION=\"$(VERSION)\"
# append -DDEBUG to DEFINES to get debug output

all: $(LIBMINOR)

libwebvi.o: libwebvi.c libwebvi.h
	$(CC) -fPIC -Wall -O2 -g $(CFLAGS) $(DEFINES) `python-config --cflags` -c -o libwebvi.o libwebvi.c

$(LIBMINOR): libwebvi.o
	$(CC) -shared -Wl,-soname,$(LIBSONAME) -Wl,--as-needed libwebvi.o `python-config --ldflags` -o $(LIBMINOR)
	ln -sf $(LIBMINOR) $(LIBSONAME)
	ln -sf $(LIBSONAME) $(LIBNAME)

libwebvi.a: libwebvi.o
	ar rsc libwebvi.a libwebvi.o

clean:
	rm -f *.o *~ libwebvi.so* libwebvi.a
	rm -f webvi/*.pyc webvi/*~

install: $(LIBMINOR)
	mkdir -p $(PREFIX)/lib
	cp --remove-destination -d $(LIBNAME)* $(PREFIX)/lib
	/sbin/ldconfig $(PREFIX)/lib

.PHONY: clean install
