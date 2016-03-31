EMACS_ROOT ?= ../..
EMACS ?= emacs

CC      = gcc
LD      = gcc
CPPFLAGS = -I$(EMACS_ROOT)/src
CFLAGS = -std=gnu99 -ggdb3 -Wall -fPIC $(CPPFLAGS)

.PHONY : test

all: barcode-core.so

barcode-core.so: barcode-core.o
	$(LD) -shared $(LDFLAGS) -o $@ $^ -lbarcode

barcode-core.o: barcode-core.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f barcode-core.so barcode-core.o

test:
	$(EMACS) -Q -batch -L . $(LOADPATH) \
		-l test/test.el \
		-f ert-run-tests-batch-and-exit
