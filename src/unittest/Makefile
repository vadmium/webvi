CFLAGS=-O2 -g -Wall -I../libwebvi
LDFLAGS=-L../libwebvi -Wl,-rpath=../libwebvi -lwebvi

.PHONY: all clean check test

all: testlibwebvi testdownload

testlibwebvi: testlibwebvi.o ../libwebvi/libwebvi.so
testdownload: testdownload.o ../libwebvi/libwebvi.so

../libwebvi/libwebvi.so:
	make -C ../libwebvi libwebvi.so

clean:
	rm -f testlibwebvi testlibwebvi.o testdownload testdownload.o

# testdownload and testwebvi.py require network access
check: testlibwebvi testdownload
	FAILED_TESTS=; \
	PYTHONPATH=../libwebvi ./testlibwebvi; \
	if [ $$? -ne 0 ]; then echo "$$?"; FAILED_TESTS="$$FAILED_TESTS testlibwebvi"; fi; \
	PYTHONPATH=../libwebvi ./testdownload; \
	if [ $$? -ne 0 ]; then FAILED_TESTS="$$FAILED_TESTS testdownload"; fi; \
	PYTHONPATH=../libwebvi python ./testwebvi.py; \
	if [ $$? -ne 0 ]; then FAILED_TESTS="$$FAILED_TESTS testwebvi.py"; fi; \
	echo "================================================"; \
	if [ -n $$FAILED_TEST ]; then \
	  echo "The following tests failed:$$FAILED_TESTS"; \
	else \
	  echo "Tests completed successfully!"; \
	fi

test: check
