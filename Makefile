#!/bin/make -f

.PHONY: all clean test

all: libgh.a README.md test

libgh.a: libgh.a(helpers.o)
helpers.o: helpers.c
helpers.c: helpers.h

README.md: helpers.h
	./mdgen.sh -d $^ > $@

test:
	$(MAKE) -C tests

clean:
	$(RM) helpers.o libgh.a
	$(MAKE) -C tests clean

