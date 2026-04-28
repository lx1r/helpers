#!/bin/make -f

.PHONY: all clean test

all: README.md test

test:
	@cd tests && make

clean:
	@cd tests && make clean

README.md: helpers.h
	@echo gen $@
	@mdgen.sh -d $^ > $@
