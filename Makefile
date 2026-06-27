#!/bin/make -f

.PHONY: all clean test

all: libgh.a README.md test

libgh.a: helpers.o

	@ar r libgh.a helpers.o

helpers.c: helpers.h
helpers.o: helpers.c

	@gcc -c helpers.c

test:
	@cd tests && make

clean:
	rm -f helpers.o libgh.a
	@cd tests && make clean

README.md: helpers.h
	@echo gen $@
	@mdgen.sh -d $^ > $@
