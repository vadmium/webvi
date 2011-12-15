PREFIX ?= /usr/local
SYSLIBDIR = $(PREFIX)/lib

LIBNAME=libwebvi.so
LIBSONAME=$(LIBNAME).1
LIBMINOR=$(LIBSONAME).0

VERSION:=$(shell grep VERSION webvi/version.py | cut -d \' -f 2)
PYLIB:=$(shell $(PYTHON) pythonlibname.py)
DEFINES:=-DPYTHONSHAREDLIB=\"$(PYLIB)\" -DLIBWEBVI_VERSION=\"$(VERSION)\"
# append -DDEBUG to DEFINES to get debug output

all: $(LIBMINOR)

libwebvi.o: libwebvi.c libwebvi.h
	$(CC) -fPIC -Wall -O2 -g $(CFLAGS) $(DEFINES) `$(PYTHON)-config --cflags` -c -o libwebvi.o libwebvi.c

$(LIBMINOR): libwebvi.o
	$(CC) -shared -Wl,-soname,$(LIBSONAME) -Wl,--as-needed libwebvi.o `$(PYTHON)-config --ldflags` -o $(LIBMINOR)
	ln -sf $(LIBMINOR) $(LIBSONAME)
	ln -sf $(LIBSONAME) $(LIBNAME)

libwebvi.so: $(LIBMINOR)

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
