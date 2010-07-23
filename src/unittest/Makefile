CFLAGS=-O2 -g -Wall -I../libwebvi
LDFLAGS=-L../libwebvi -Wl,-rpath=../libwebvi -lwebvi

all: testlibwebvi testdownload

testlibwebvi: testlibwebvi.o ../libwebvi/libwebvi.so

testdownload: testdownload.o ../libwebvi/libwebvi.so

clean:
	rm -f testlibwebvi testlibwebvi.o testdownload testdownload.o
