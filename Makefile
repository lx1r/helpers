#!/bin/make -f

.PHONY: all test clean

CFLAGS=-I.. -Wall -Wfatal-errors
sources = $(wildcard *.c)
headers = $(wildcard *.h)
tests = $(sources:.c=)

test: $(tests)

%: %.c $(headers)

	@gcc $(CFLAGS) $< -o $@
	@echo -n running $@...
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > /dev/null && echo ok

clean:

	$(RM) *.o *.log $(tests)
