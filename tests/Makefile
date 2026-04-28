#!/bin/make -f

.PHONY: all clean

CFLAGS=-I.. -Wall -Wfatal-errors

sources = $(wildcard *.c)
targets = $(sources:.c=)

all: $(targets)

%: %.c ../helpers.h
	@gcc $(CFLAGS) $< -o $@
	@echo -n running $@...
	-@./$@ > $@.log
	-@diff $@.log $@.cmp > /dev/null && echo ok

clean:
	$(RM) *.o *.log $(targets)
